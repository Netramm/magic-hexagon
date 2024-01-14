/*Here you can find the serial and parallel implementation of the solver.

The implementation is based on and inspired by the following two articles:
https://subscription.packtpub.com/book/programming/9781784394004/1/ch01lvl1sec08/aristotle-s-number-puzzle
https://jtp.io/2017/01/12/aristotle-number-puzzle.html

The main data structure is based on this article:
https://www.redblobgames.com/grids/hexagons/
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include <mpi.h>
#ifdef _OPENMP
    #include <omp.h>
#endif

#include "helpers.h"

bool solver_depth_first(int r, int n, int N, int N_s, int M, int (*board)[r][r], bool *value_used, bool check_partial, bool find_all, bool print_solutions, int *sol_cnt){
    /* This function implements the depth first search algorithm to solve the magic hexagon problem.
    The algorithm is used for both the serial and the parallel implementation. It recursively tries to set a value at an unset position and then checks if the board is still valid.
    */
    int i, j, k, row_length;
    // Loop over each row
    for (i = 0; i < r; i++){
        row_length = r-abs(n-1-i);
        int a[r][3];
        // Get the coordinates of the rows of the first diagonal
        get_coordinates_of_row(a, 0, i, n);

        // Loop over each element in the row and check if it is already set
        for (j = 0; j < row_length; j++){
            if (board[a[j][0]][a[j][1]][a[j][2]] > 0){
                continue;
            }
            // It is not set, hence we can select a new value to set at that position and recurse
            for (k = 0; k < N; k++){
                // find a value which hasn't been set yet
                if (value_used[k])
                    continue;
                // set it and recurse
                board[a[j][0]][a[j][1]][a[j][2]] = k + N_s;
                value_used[k] = true;
                // If we selected to check partial solutions, we are now checking if the tile placement keeps the board valid
                // If so we recurse
                if (check_partial && validate_tile(r, board, M, n, a[j]) && solver_depth_first(r, n, N, N_s, M, board, value_used, check_partial, find_all, print_solutions, sol_cnt)){
                    return true;
                }
                // If no partial check selected, we just recurse and return true if it works out
                else if (!check_partial && solver_depth_first(r, n, N, N_s, M, board, value_used, check_partial, find_all, print_solutions, sol_cnt)){
                    return true;
                }
                // else reset the tile and try the next available value
                else{
                    board[a[j][0]][a[j][1]][a[j][2]] = 0;
                    value_used[k] = false;
                }
            }
            // we couldnt find any value to set, so we have to try a different branch
            return false;
        }
    }

    // To this point we only get if all tiles have a value assigned
    // Evaluate the board and return the result or print the board if we try to find all solutions
    bool ret = validate_board(r, board, M, n);
    if (find_all && ret){
        if (print_solutions)
            print_board(r, n, board);
        (*sol_cnt)++;
        return false;
    }
    else{
        return ret;
    }
}

bool generate_starting_row(int row_length, int N, int N_s, int M, int nr_s, int (*starting_row_list), int *prev_nrs, int ind, int *cnt){
    /* This function generates all possible starting rows for the solver. It is used to parallelize the solver. The geneartion is done by looking at all possible combinations of distinct numbers for the selected row that add up to M.
    */
    int i, j;
    bool duplicated = false;
    int start_index = 0;

    // Run over each position in the starting row
    if (ind < row_length){
        // Loop over all possible numbers
        for (i = start_index; i < N; i++){
            // Check if the number has already been used
            duplicated = false;
            for (j = 0; j < ind; j++){
                if (prev_nrs[j] == i){
                    duplicated = true;
                    break;
                }
            }
            if (duplicated)
                continue;
            // Set this number as new one and recurse until all numbers in the row are set
            prev_nrs[ind] = i;
            if (ind < row_length-1){
                if (!generate_starting_row(row_length, N, N_s, M, nr_s, starting_row_list, prev_nrs, ind + 1, cnt)){
                    return false;
                }
            }
            // If all numbers are set, check if the row is valid, meaning if it sums up to M
            else{
                int sum = 0;
                int k;
                for (k = 0; k < row_length; k++){
                    sum += prev_nrs[k] + N_s;
                }
                // If not valid, try the next number
                if (sum != M){
                    continue;
                }
                // If valid, add it to the list of starting rows
                for (k = 0; k < row_length; k++){
                    starting_row_list[(*cnt) * row_length + k] = prev_nrs[k] + N_s;
                }
                (*cnt)++;
                // Only generate a limited number of starting rows
                if (*cnt == nr_s)
                    return false;
            }
        }
    }

    return true;
}

int solver(int n, int r, int N_s, int N, int M, bool find_all, int precomputed_row, int nr_s, bool parallel_exec, bool check_partial, bool print_solutions, int verbosity, int benchmark){
    /* This function is the main function of the solver. It initializes the board to solve and calls the solver_depth_first function. The mutidimensional-array storing the current board is initialized with 0's and dependent on the solver varaint chosen either the first tile or a full row will have a precomputed value assigned before solving the rest of the tiles.
    The representation of the board is inspired by: https://www.redblobgames.com/grids/hexagons/
    */

    // This is the board to solve
    int board[r][r][r];
    // This array holds the values which we will initially set on the board
    int vals_to_solve[N];
    int i;
    for (i = 0; i < N; i++){
        vals_to_solve[i] = 0;
    }

    // A list which determines whether a value has already been set
    bool value_used[N];

    // A counter which counts the number of found solutions
    int sol_cnt = 0;

    // If we want to use precomputed combinations for a specific row, choose this branch
    if (precomputed_row >= 0){
        // If executed in parallel split the tasks
        if (parallel_exec){
            // Get the number of processes
            int comm_sz;
            MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

            // Get the rank of the process
            int my_rank;
            MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

            // Calculate the length of the row we are precomputing
            int row_length = n + precomputed_row;
            // This array holds the precomputed combinations
            int starting_row[nr_s * row_length];
            // The number of precomputed combinations
            int cnt;
            // Calculate possible starting row combinations on the first process
            if (my_rank == 0){
                int prev_nrs[row_length];
                cnt = 0;

                // Calculate possible starting row combinations
                bool ret_generator = generate_starting_row(row_length, N, N_s, M, nr_s, starting_row, prev_nrs, 0, &cnt);

                if (!ret_generator){
                    printf("The number of possible starting rows exceeds the number selected!\nChoose a larger number to generate all starting rows.\n");
                }
                
                printf("Number of possible starting rows: %d\n", cnt);

            }
            // Broadcast the number of precomputed combinations to all processes
            MPI_Bcast(&cnt, 1, MPI_INT, 0, MPI_COMM_WORLD);

            // Calculate how many positions per process have to be calculated. Only works if the number of processes is a divisor of the number of possible combinations
            if (benchmark != 2 && cnt % comm_sz != 0){
                if (my_rank == 0){
                    printf("The number of processes (%d) has to be a divisor of the number of possible values (%d)!\n", comm_sz, cnt);
                }
                return 0;
            }
            int share = (int)(cnt / comm_sz);

            // Distribute the respective shares of precomputed combinations to all processes
            int local_starting_row[share * row_length];
            MPI_Scatter(starting_row, share * row_length, MPI_INT, local_starting_row, share * row_length, MPI_INT, 0, MPI_COMM_WORLD);

            int j, k;
            int visited = 0;
            bool ret_solver;
            // Calculate the index of the first position of the precomputed combinations
            int start_index = 0;
            for (k = 0; k < precomputed_row; k++){
                start_index += n;
                start_index += k;
            }

            // Only use OpenMP if we can access the API. If so, parallelize the loop over the precomputed combinations
            #ifdef _OPENMP
                #pragma omp parallel for default(none) private(j, value_used, board, ret_solver) firstprivate(vals_to_solve, visited) shared(N, share, r, n, row_length, N_s, M, print_solutions, local_starting_row, my_rank, verbosity, find_all, start_index, check_partial) reduction(+:sol_cnt)
            #endif
            // Loop over the assigned precomputed combinations of this process
            for (i = 0; i < share; i++){
                // Print out the process and the CPU it is running on as well as the thread if OpenMP is used
                if (verbosity > 1 && visited == 0){
                    #ifdef _OPENMP
                        #pragma omp critical
                        {
                            printf("Thread %d of process %d on CPU %d\n", omp_get_thread_num(), my_rank, sched_getcpu());
                            if (verbosity > 2)
                                print_board(r,n,board);
                        }
                    #else
                        printf("Process %d on CPU %d\n", my_rank, sched_getcpu());
                    #endif
                    visited = 1;
                }

                // Fill the board with the values of the precomputed combination
                fill_value_list(N, value_used);
                for (j = 0; j < row_length; j++){
                    vals_to_solve[start_index + j] = local_starting_row[i * row_length + j];
                    value_used[local_starting_row[i * row_length + j] - N_s] = true;
                }
                fill_board(vals_to_solve, r, n, board);

                // Call the solver
                ret_solver = solver_depth_first(r, n, N, N_s, M, board, value_used, check_partial, find_all, print_solutions, &sol_cnt);

                // If we only want to find the first solution, we can abort the program if we found one
                if (!find_all){
                    if (ret_solver){
                        printf("Solver found a solution!\nThis is the solution he found:\n");
                        print_board(r, n, board);
                        MPI_Abort(MPI_COMM_WORLD, 1);
                    }
                }
            }
        }
        // Sequential execution
        else{
            // Calculate the length of the row we are precomputing
            int row_length = n + precomputed_row;
            // This array holds the precomputed combinations
            int starting_row[nr_s * row_length];
            int prev_nrs[row_length];
            // The number of precomputed combinations
            int cnt = 0;

            // Calculate possible starting row combinations
            bool ret_generator = generate_starting_row(row_length, N, N_s, M, nr_s, starting_row, prev_nrs, 0, &cnt);

            if (!ret_generator){
                printf("The number of possible starting rows exceeds the number selected!\nChoose a larger number to generate all starting rows.\n");
            }
            
            printf("Number of possible starting rows: %d\n", cnt);

            int j, k;
            bool ret_solver;
            // Calculate the index of the first position of the precomputed combinations
            int start_index = 0;
            for (k = 0; k < precomputed_row; k++){
                start_index += n;
                start_index += k;
            }
            // Loop over all precomputed combinations
            for (i = 0; i < cnt; i++){
                // Fill the board with the values of the precomputed combination
                fill_value_list(N, value_used);
                for (j = 0; j < row_length; j++){
                    vals_to_solve[start_index + j] = starting_row[i * row_length + j];
                    value_used[starting_row[i * row_length + j] - N_s] = true;
                }
                fill_board(vals_to_solve, r, n, board);

                // Call the solver
                ret_solver = solver_depth_first(r, n, N, N_s, M, board, value_used, check_partial, find_all, print_solutions, &sol_cnt);
                
                // If we only want to find the first solution, we can abort the program if we found one
                if (!find_all && ret_solver){
                    printf("Solver found a solution!\nThis is the solution he found:\n");
                    print_board(r, n, board);
                    return 1;
                }
            }
            // We didn't find any solution
            if (!find_all){
                printf("Solver was not able to find a solution for this board!\n");
                return 0;
            }
        }
        // Return the number of found solutions
        return sol_cnt;
    }
    // If we don't want to use precomputed combinations, choose this branch
    else{
        // If executed in parallel split the tasks
        if (parallel_exec){
            // Get the number of processes
            int comm_sz;
            MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

            // Get the rank of the process
            int my_rank;
            MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

            // Calculate how many values per process have to be calculated. Only works if the number of processes is a divisor of the number of values
            if (benchmark != 2 && N % comm_sz != 0){
                if (my_rank == 0){
                    printf("The number of processes (%d) has to be a divisor of the number of possible values (%d)!\n", comm_sz, N);
                }
                return 0;
            }
            int share = (int)(N / comm_sz);
            bool ret_solver;

            // Loop over the assigned values of this process
            for (i = my_rank * share; i < (my_rank + 1) * share; i++){
                // Fill the first tile with the respective value
                fill_value_list(N, value_used);
                vals_to_solve[0] = i + N_s;
                value_used[i] = true;
                fill_board(vals_to_solve, r, n, board);

                // Call the solver
                ret_solver = solver_depth_first(r, n, N, N_s, M, board, value_used, check_partial, find_all, print_solutions, &sol_cnt);

                // If we only want to find the first solution, we can abort the program if we found one
                if (!find_all){
                    if (ret_solver){
                        printf("Solver found a solution!\nThis is the solution he found:\n");
                        print_board(r, n, board);
                        MPI_Abort(MPI_COMM_WORLD, 1);
                        return 1;
                    }
                }
            }
        }
        // Sequential execution
        else{
            // Prepare the board by setting all tiles to 0
            fill_board(vals_to_solve, r, n, board);
            fill_value_list(N, value_used);

            // Call the solver
            bool ret_solver = solver_depth_first(r, n, N, N_s, M, board, value_used, check_partial, find_all, print_solutions, &sol_cnt);

            // If we only want to find the first solution, we can abort the program if we found one
            if (!find_all){
                if (ret_solver){
                    printf("Solver found a solution!\nThis is the solution he found:\n");
                    print_board(r, n, board);
                    return 1;
                }
                else{
                    printf("Solver was not able to find a solution for this board!\n");
                    return 0;
                }
            }
        }
        // Return the number of found solutions
        return sol_cnt;
    }
}

int main(int argc, char** argv) {
    /* This is the main function of the programm. It reads out the command line arguments and calls the solver function.
    */

    // Side length of the hexagon
    int n = 3;
    // Number of rows of the hexagon
    int r = n*2-1;
    // Starting of number range of values to place
    int N_s = 1;
    // Number of tiles to place
    int N = 3*n*n-3*n+1;
    // Sum which has to be obtained in each row
    int M = 38;
    // Whether we want to find all solutions or only the first one
    bool find_all = false;
    // Max number of precomputed combinations of the first row we are looking at
    int nr_s = 1000;
    // Whether to run the code in parallel
    bool parallel_execution = false;
    // Whether to check for partial validity
    bool check_partial = true;
    // Which row do we precalculate (only choose from the first half), if none then set to -1
    int starting_rows_calc = 0;
    // Whether to print out the found solutions
    bool print_solutions = false;
    // Verbosity level
    int verbosity = 0;
    // Select the benchmark we are running
    int benchmark = 1;

    // Read out command line arguments if supplied
    int opt;
    while ((opt = getopt(argc, argv, "n:s::M:a:l::p::r::o::v::c::b::")) != -1){
        switch (opt){
            case 'n':
                n = atoi(optarg);
                r = n*2-1;
                N = 3*n*n-3*n+1;
                break;
            case 's':
                N_s = atoi(optarg);
                break;
            case 'M':
                M = atoi(optarg);
                break;
            case 'a':
                find_all = atoi(optarg);
                break;
            case 'l':
                nr_s = atoi(optarg);
                break;
            case 'p':
                parallel_execution = atoi(optarg);
                break;
            case 'r':
                starting_rows_calc = atoi(optarg);
                break;
            case 'o':
                print_solutions = atoi(optarg);
                break;
            case 'v':
                verbosity = atoi(optarg);
                break;
            case 'c':
                check_partial = atoi(optarg);
                break;
            case 'b':
                benchmark = atoi(optarg);
                break;
            
            default:
                printf("Command line argument could not be understood!\n");
                break;
        }
    }

    // If we want to execute in parallel, choose this branch
    if (parallel_execution){
        // Get the number of openMP threads
        #ifdef _OPENMP
            int threads = omp_get_max_threads();
        #else
            int threads = 1;
        #endif

        // Initialize the MPI environment
        if (threads > 1){
            int provided;
            MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
            if (provided != MPI_THREAD_FUNNELED)
            {
                printf("Failed to initialize MPI_THREAD_FUNNELED\n");
                exit(-1);
            }
        }
        else
            MPI_Init(&argc, &argv);

        // Get the number of processes
        int comm_sz;
        MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

        // Get the rank of the process
        int my_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

        // Print out the node name, process id and the CPU it is running on
        if (verbosity > 0){
            int cpu_num = sched_getcpu();
            char proc_name[100];
            int name_len = 0;
            MPI_Get_processor_name(proc_name, &name_len);
            printf("Name: %s, Process: %d, CPU: %d\n",proc_name, my_rank, cpu_num);

            MPI_Barrier(MPI_COMM_WORLD);
        }

        // variables holding runtime statistics
        double diff, max_diff, min_diff, sum_diff, start_time, end_time;
        // variables holding the number of found solutions
        int local_sol_cnt, sol_cnt, i;

        // Print out the parameters of the solver
        if (my_rank == 0){
            if (starting_rows_calc >= 0)
                printf("\nStart parallel solver with precomputed rows. We are using %d processes on %d threads.\n", comm_sz, threads);
            else
                printf("\nStart parallel solver without precomputed rows. We are using %d processes on %d threads.\n", comm_sz, threads);
            printf("n = %d, s = %d, M = %d, a = %d, l = %d\n\n", n, N_s, M, find_all, nr_s);
        }

        // Wait for all processes to reach this point and start the timer
        MPI_Barrier(MPI_COMM_WORLD);
        start_time = MPI_Wtime();

        // Call the solver dependent on which benchmark we are running
        if (benchmark == 1 || benchmark == 3){
            local_sol_cnt = solver(n, r, N_s, N, M, find_all, starting_rows_calc, nr_s, parallel_execution, check_partial, print_solutions, verbosity, benchmark);
     
            // Add up number of found solutions
            MPI_Reduce(&local_sol_cnt, &sol_cnt, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        }
        else if (benchmark == 2){
            // Loop over a range of possible values of M and call the solver for each of them
            for (i = 19; i <= 54; i++){
                local_sol_cnt = solver(n, r, N_s, N, i, find_all, starting_rows_calc, nr_s, parallel_execution, check_partial, print_solutions, verbosity, benchmark);

                // Add up number of found solutions
                MPI_Reduce(&local_sol_cnt, &sol_cnt, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
                if (my_rank == 0)
                    printf("M = %d, sol_cnt = %d\n", i, sol_cnt);
            }
        }
        else{
            printf("Please select a valid benchmark!\n");
            exit(0);
        }

        // Stop the timer
        end_time = MPI_Wtime();

        // Calculate the runtime statistics
        diff = end_time - start_time;
        MPI_Reduce(&diff, &max_diff, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        MPI_Reduce(&diff, &min_diff, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
        MPI_Reduce(&diff, &sum_diff, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

        // Print out the runtime statistics and number of found solutions
        if (my_rank == 0){
            if (benchmark != 2){
                printf("The solver found %d solutions.\n", sol_cnt);
            }
            printf("This took %lf seconds on %d processes.\n", max_diff, comm_sz);
            printf("The shortest running process was %lf seconds long and on average a process took %lf seconds (total = %lf).", min_diff, sum_diff / comm_sz, sum_diff);
        }

        // Finalize the MPI environment
        MPI_Finalize();
    }
    // Sequential execution
    else{
        // variables holding runtime statistics
        struct timespec start_time, end_time;
        double diff;
        // variables holding the number of found solutions
        int sol_cnt, i;
        
        // Print out the parameters of the solver
        if (starting_rows_calc >= 0)
            printf("\nStart sequential solver with precomputed rows.\n");
        else
            printf("\nStart sequential solver without precomputed rows.\n");
        printf("n = %d, s = %d, M = %d, a = %d, l = %d\n\n", n, N_s, M, find_all, nr_s);

        // Start the timer
        clock_gettime(CLOCK_MONOTONIC, &start_time);

        // Call the solver dependent on which benchmark we are running
        if (benchmark == 1 || benchmark == 3){
            sol_cnt = solver(n, r, N_s, N, M, find_all, starting_rows_calc, nr_s, parallel_execution, check_partial, print_solutions, verbosity, benchmark);
        }
        else if (benchmark == 2){
            // Loop over a range of possible values of M and call the solver for each of them
            for (i = 19; i <= 54; i++){
                sol_cnt = solver(n, r, N_s, N, i, find_all, starting_rows_calc, nr_s, parallel_execution, check_partial, print_solutions, verbosity, benchmark);

                printf("M = %d, sol_cnt = %d\n", i, sol_cnt);
            }
        }
        else{
            printf("Please select a valid benchmark!\n");
            exit(0);
        }

        // Stop the timer
        clock_gettime(CLOCK_MONOTONIC, &end_time);

        // Print out the number of found solutions and the runtime
        printf("The solver found %d solutions.\n", sol_cnt);
        diff = get_time_diff(start_time, end_time);
        printf("This took %lf seconds.\n", diff);
    }

    return 0;
}