/* This serial code is based on and inspired by the following two articles:
https://subscription.packtpub.com/book/programming/9781784394004/1/ch01lvl1sec08/aristotle-s-number-puzzle
https://jtp.io/2017/01/12/aristotle-number-puzzle.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>

int find_starting_index(int n, int j){
    /* This function return the first index to access in the current row.
    The general equation holds, that the accessed indexes in the board representation have to add up to (n-1)*3.
    As one index is fixed, we have to find the starting index for the other two indexes such that the equation holds.
    */
    int index = 0;
    // This is the maximum value an index can have
    int max_index = (n-1)*2;
    // This is the sum the indexes have to have
    int target_sum = (n-1)*3;

    // Increase the index until it adds up to the sum given that the other free index takes the largest value possible
    while (j + index + max_index < target_sum){
        index++;
    }
    
    return index;
}

void get_coordinates_of_row(int (*coord_arr)[3], int diagonal, int row, int n){
    /* This function returns the coordinates of a row in a given diagonal.
    One of the three indexes indetifying an entry in the board is fixed as we have chosen a row.
    Hence, we have to caluclate the other two freely choosable indexes.
    */
    int r = n*2-1;
    int row_length = r-abs(n-1-row);
    int target_sum = (n-1)*3;

    // Get the startig value of the first freely choosable index
    int first_index = find_starting_index(n, row);
    // Calculate the same for the second freely choosable index, given that we know that they have to add up to the target_sum
    int second_index = target_sum - row - first_index;
    int i;
    // Loop over each element in the row and add the final coordinates to the array
    for (i = 0; i < row_length; i++){
        if (diagonal == 0){
            coord_arr[i][0] = row;
            coord_arr[i][1] = first_index;
            coord_arr[i][2] = second_index;
        }
        else if (diagonal == 1){
            coord_arr[i][0] = first_index;
            coord_arr[i][1] = row;
            coord_arr[i][2] = second_index;
        }
        else{
            coord_arr[i][0] = first_index;
            coord_arr[i][1] = second_index;
            coord_arr[i][2] = row;
        }

        // Increase one index and decrease the other so that we satisfy the index sum equation.
        first_index++;
        second_index--;
    }
}

bool validate_board(int r, int (*board)[r][r], int M, int n){
    /* This function checks for each row in each diagonal if they sum up to M.
    */
    int i, j, k;
    int total, row_length;
    
    // Loop through each diagonal of the hexagon
    for (i = 0; i < 3; i++){
        // Run over each row in the given diagonal
        for (j = 0; j < r; j++){
            total = 0;
            row_length = r-abs(n-1-j);

            // Get the coordinates of the row we have to access
            int coord_arr[row_length][3];
            get_coordinates_of_row(coord_arr, i, j, n);

            // Add up each element in the row
            for (k = 0; k < row_length; k++){
                total += board[coord_arr[k][0]][coord_arr[k][1]][coord_arr[k][2]];
            }
            
            // Check if sum is equal to M
            if (total != M){
                return false;
            }
        }
    }
    return true;
}

bool validate_tile(int r, int (*board)[r][r], int M, int n, int *tile_placed){
    /* This function checks whether a newly placed tile is keeping the board valid.
    This can be achieved by checking the validity of the three rows the tile is in in each diagonal.
    */
    int i, k;
    int total, board_val, row_length;
    bool all_set;
    
    // Loop through each diagonal of the hexagon
    for (i = 0; i < 3; i++){
        total = 0;
        // This variable saves whether all tiles in the row have a value
        all_set = true;
        row_length = r-abs(n-1-tile_placed[i]);

        // Select the row in which the tile is in in the given diagonal
        int coord_arr[row_length][3];
        get_coordinates_of_row(coord_arr, i, tile_placed[i], n);

        // Add up each element in the row
        for (k = 0; k < row_length; k++){
            board_val = board[coord_arr[k][0]][coord_arr[k][1]][coord_arr[k][2]];
            // Check if only partial solution in the row
            if (board_val == 0)
                all_set = false;
            else
                total += board_val;
        }
        
        // Check if sum is equal to M if all values were set, otherwise just check if not larger than M
        if ((all_set && total != M) || total > M){
            return false;
        }
    }
    return true;
}

void fill_board(int *values, int r, int n, int (*board)[r][r]){
    /* This function takes and array of values and fills them into the correct positions of the board.
    */
    int index_counter = 0;
    int i, j, row_length;
    // Loop over each row
    for (i = 0; i < r; i++){
        row_length = r-abs(n-1-i);
        int a[r][3];
        // Get the coordinates of the rows of the first diagonal
        get_coordinates_of_row(a, 0, i, n);

        // Loop over each element in the row and set the repective value
        for (j = 0; j < row_length; j++){
            board[a[j][0]][a[j][1]][a[j][2]] = values[index_counter];
            index_counter++;
        }
    }
}

void print_board(int r, int n, int (*b)[r][r]){
    /* This function prints out the given board to the console.
    */
    int i, j, k, diff, row_length;
    printf("\n");
    // Loop over each row
    for (i = 0; i < r; i++){
        row_length = r-abs(n-1-i);
        int a[r][3];
        // Get the coordinates of the rows of the first diagonal
        get_coordinates_of_row(a, 0, i, n);

        diff = r - row_length;
        // Print leading spaces
        for (k = 0; k < diff; k++){
            printf("  ");
        }
        // Loop over each element in the row and print the respective value
        for (j = 0; j < row_length; j++){
            printf("%02d  ", b[a[j][0]][a[j][1]][a[j][2]]);
        }
        // Print trailing spaces
        for (k = 0; k < diff - 1; k++){
            printf("  ");
        }
        printf("\n");
    }
    printf("\n");
}

bool solver_depth_first(int r, int n, int N, int N_s, int M, int (*board)[r][r], bool *value_used, bool check_partial, bool find_all, bool print_solutions, int *sol_cnt){
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

int generate_starting_row_old(int n, int N, int N_s, int M, int nr_s, int (*starting_row_list)[n]){
    int i, j, k;
    int cnt = 0;
    for (i = 0; i < N; i++){
        for (j = 0; j < N; j++){
            if (i == j)
                continue;
            for (k = i + 1; k < N; k++){
                if (k == j || i+j+k+3*N_s != M)
                    continue;
                starting_row_list[cnt][0] = i+N_s;
                starting_row_list[cnt][1] = j+N_s;
                starting_row_list[cnt][2] = k+N_s;
                cnt++;
                if (cnt == nr_s)
                    return cnt;
            }
        }
    }
    return cnt;
}

bool generate_starting_row(int n, int N, int N_s, int M, int nr_s, int (*starting_row_list), int *prev_nrs, int ind, int *cnt){
    int i, j;
    bool duplicated = false;
    int start_index = 0;

    if (ind < n){
        // TODO, has to be checked more general!!!!!!!!!!!!!!!!!!!!!!!
        // if (ind > (int)(n-1) / 2){
        //     start_index = prev_nrs[n - ind - 1] + 1;
        // }
        for (i = start_index; i < N; i++){
            duplicated = false;
            for (j = 0; j < ind; j++){
                if (prev_nrs[j] == i){
                    duplicated = true;
                    break;
                }
            }
            if (duplicated)
                continue;
            prev_nrs[ind] = i;
            if (ind < n-1){
                if (!generate_starting_row(n, N, N_s, M, nr_s, starting_row_list, prev_nrs, ind + 1, cnt)){
                    return false;
                }
            }
            else{
                int sum = 0;
                int k;
                for (k = 0; k < n; k++){
                    sum += prev_nrs[k] + N_s;
                }
                if (sum != M){
                    continue;
                }
                for (k = 0; k < n; k++){
                    starting_row_list[(*cnt) * n + k] = prev_nrs[k] + N_s;
                }
                (*cnt)++;
                if (*cnt == nr_s)
                    return false;
            }
        }
    }

    return true;
}

void fill_value_list(int N, bool *array){
    /* This function is used to initialize an array containing whether a value has been used.
    */
    int i;
    for (i = 0; i < N; i++){
        array[i] = false;
    }
}

double get_time_diff(struct timespec start, struct timespec end){
    return (end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) / (double)1000000000L;
}

int solver(int n, int r, int N_s, int N, int M, bool find_all, bool use_precomputed_row, int nr_s, bool parallel_exec, bool print_solutions){
    // The mutidimensional-array storing the current board, initialized with 0's
    // This representation is inspired by: https://www.redblobgames.com/grids/hexagons/
    int board[r][r][r];
    int vals_to_solve[N];
    int i;
    for (i = 0; i < N; i++){
        vals_to_solve[i] = 0;
    }

    // A list which determines whether a value has already been set
    bool value_used[N];

    // A counter which counts the number of found solutions
    int sol_cnt = 0;

    if (use_precomputed_row){
        // If executed in parallel split the tasks
        if (parallel_exec){
            // Get the number of processes
            int comm_sz;
            MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

            // Get the rank of the process
            int my_rank;
            MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

            // Calculate possible starting rows on the first process
            int starting_row[nr_s * n];
            int cnt;
            if (my_rank == 0){
                int prev_nrs[n];
                cnt = 0;

                bool ret_generator = generate_starting_row(n, N, N_s, M, nr_s, starting_row, prev_nrs, 0, &cnt);

                if (!ret_generator){
                    printf("The number of possible starting rows exceeds the number selected!\nChoose a larger number to generate all starting rows.\n");
                }
                
                printf("Number of possible starting rows: %d\n", cnt);

            }
            MPI_Bcast(&cnt, 1, MPI_INT, 0, MPI_COMM_WORLD);

            // Calculate how many positions per process have to be calculated
            int share = (int)(cnt / comm_sz);

            // scatterv
            int local_starting_row[share * n];
            MPI_Scatter(starting_row, share * n, MPI_INT, local_starting_row, share * n, MPI_INT, 0, MPI_COMM_WORLD);

            int j;
            for (i = 0; i < share; i++){
                fill_value_list(N, value_used);
                for (j = 0; j < n; j++){
                    vals_to_solve[j] = local_starting_row[i * n + j];
                    value_used[local_starting_row[i * n + j] - N_s] = true;
                }
                fill_board(vals_to_solve, r, n, board);

                solver_depth_first(r, n, N, N_s, M, board, value_used, true, true, print_solutions, &sol_cnt);
            }
        }
        else{
            int starting_row[nr_s * n];
            int prev_nrs[n];
            int cnt = 0;

            bool ret_generator = generate_starting_row(n, N, N_s, M, nr_s, starting_row, prev_nrs, 0, &cnt);

            if (!ret_generator){
                printf("The number of possible starting rows exceeds the number selected!\nChoose a larger number to generate all starting rows.\n");
            }
            
            printf("Number of possible starting rows: %d\n", cnt);

            int j;
            bool ret_solver;
            for (i = 0; i < cnt; i++){
                fill_value_list(N, value_used);
                for (j = 0; j < n; j++){
                    vals_to_solve[j] = starting_row[i * n + j];
                    value_used[starting_row[i * n + j] - N_s] = true;
                }
                fill_board(vals_to_solve, r, n, board);

                ret_solver = solver_depth_first(r, n, N, N_s, M, board, value_used, true, find_all, print_solutions, &sol_cnt);
                
                if (!find_all && ret_solver){
                    printf("Solver found a solution!\nThis is the solution he found:\n");
                    print_board(r, n, board);
                    return 1;
                }
            }
            if (!find_all){
                printf("Solver was not able to find a solution for this board!\n");
                return 0;
            }
        }

        return sol_cnt;
    }
    else{
        // If executed in parallel split the tasks
        if (parallel_exec){
            // Get the number of processes
            int comm_sz;
            MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

            // Get the rank of the process
            int my_rank;
            MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

            // Calculate how many positions per process have to be calculated
            int share = (int)(N / comm_sz);

            for (i = my_rank * share; i < (my_rank + 1) * share; i++){
                fill_value_list(N, value_used);
                vals_to_solve[0] = i + N_s;
                value_used[i] = true;
                fill_board(vals_to_solve, r, n, board);

                solver_depth_first(r, n, N, N_s, M, board, value_used, true, true, print_solutions, &sol_cnt);
            }
        }
        else{
            fill_board(vals_to_solve, r, n, board);
            fill_value_list(N, value_used);

            bool ret_solver = solver_depth_first(r, n, N, N_s, M, board, value_used, true, find_all, print_solutions, &sol_cnt);

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
        return sol_cnt;
    }
}

int main(int argc, char** argv) {
    // Side length of the hexagon
    int n = 3; // 3, 4, 2
    // Numbe of rows of the hexagon
    int r = n*2-1;
    // Number range of tiles to place
    int N_s = 1;
    // int N_e = 3*n*n-3*n+1;
    int N = 3*n*n-3*n+1;
    // Sum which has to be obtained in each row
    int M = 38;
    // Whether we want to find all solutions or only the first one
    bool find_all = false;
    // Max number of starting positions of the first row we are looking
    int nr_s = 1000;
    // Whether to run the code in parallel
    bool parallel_execution = false;
    // Do we use the precomputed starting rows?
    bool use_starting_rows = true;
    // Whether to print out the found solutions
    bool print_solutions = false;

    // Read out command line arguments if supplied
    int opt;
    while ((opt = getopt(argc, argv, "n:s::M:a:l::p::r::o::")) != -1){
        switch (opt){
            case 'n':
                n = atoi(optarg);
                r = n*2-1;
                N = 3*n*n-3*n+1;
                // N_e = 3*n*n-3*n+1;
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
                use_starting_rows = atoi(optarg);
                break;
            case 'o':
                print_solutions = atoi(optarg);
                break;
            
            default:
                printf("Command line argument could not be understood!\n");
                break;
        }
    }

    if (parallel_execution){
        // Initialize the MPI environment
        MPI_Init(NULL, NULL);

        // Get the number of processes
        int comm_sz;
        MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

        // Get the rank of the process
        int my_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

        double diff, max_diff, min_diff, sum_diff, start_time, end_time;
        int local_sol_cnt, sol_cnt;

        if (my_rank == 0){
            if (use_starting_rows)
                printf("\nStart parallel solver with precomputed rows.\n\n");
            else
                printf("\nStart parallel solver without precomputed rows.\n\n");
        }

        start_time = MPI_Wtime();

        local_sol_cnt = solver(n, r, N_s, N, M, true, use_starting_rows, nr_s, parallel_execution, print_solutions);
        
        // Add up number of found solutions
        MPI_Reduce(&local_sol_cnt, &sol_cnt, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        end_time = MPI_Wtime();

        diff = end_time - start_time;

        MPI_Reduce(&diff, &max_diff, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        MPI_Reduce(&diff, &min_diff, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
        MPI_Reduce(&diff, &sum_diff, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

        if (my_rank == 0){
            printf("The solver found %d solutions.\n", sol_cnt);
            printf("This took %lf seconds.\n", max_diff);
            printf("The shortest running process was %lf seconds long and on average a process took %lf seconds (total = %lf).", min_diff, sum_diff / comm_sz, sum_diff);
        }


        // Finalize the MPI environment
        MPI_Finalize();
    }
    else{
        struct timespec start_time, end_time;
        double diff;
        int sol_cnt;
        
        if (use_starting_rows)
            printf("\nStart sequential solver with precomputed rows.\n\n");
        else
            printf("\nStart sequential solver without precomputed rows.\n\n");

        clock_gettime(CLOCK_MONOTONIC, &start_time);

        sol_cnt = solver(n, r, N_s, N, M, find_all, true, nr_s, parallel_execution, print_solutions);

        clock_gettime(CLOCK_MONOTONIC, &end_time);

        printf("The solver found %d solutions.\n", sol_cnt);

        diff = get_time_diff(start_time, end_time);
        printf("This took %lf seconds.\n", diff);
    }

    return 0;
}