/* This serial code is based on and inspired by the following two articles:
https://subscription.packtpub.com/book/programming/9781784394004/1/ch01lvl1sec08/aristotle-s-number-puzzle
https://jtp.io/2017/01/12/aristotle-number-puzzle.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>

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

bool solver_depth_first(int r, int n, int N, int M, int (*board)[r][r], bool *value_used, bool check_partial){
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
                board[a[j][0]][a[j][1]][a[j][2]] = k + 1;
                value_used[k] = true;
                // If we selected to check partial solutions, we are now checking if the tile placement keeps the board valid
                // If so we recurse
                if (check_partial && validate_tile(r, board, M, n, a[j]) && solver_depth_first(r, n, N, M, board, value_used, check_partial)){
                    return true;
                }
                // If no partial check selected, we just recurse and return true if it works out
                else if (!check_partial && solver_depth_first(r, n, N, M, board, value_used, check_partial)){
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
    // Evaluate the board and return the result
    return validate_board(r, board, M, n);
}

void fill_value_list(int N, bool *array){
    /* This function is used to initialize an array containing whether a value has been used.
    */
    int i;
    for (i = 0; i < N; i++){
        array[i] = false;
    }
}

int main(int argc, char** argv) {
    // Side length of the hexagon
    int n = 3; // 3, 4, 2
    // Numbe of rows of the hexagon
    int r = n*2-1;
    // Number of tiles to place, numbered from 1 to 19
    int N = 19; // 19, 37, 7
    // Sum which has to be obtained in each row
    int M = 38;

    // The mutidimensional-array storing the current board, initialized with 0's
    // This representation is inspired by: https://www.redblobgames.com/grids/hexagons/
    int board[r][r][r];
    int zeros[N];
    int i;
    for (i = 0; i < N; i++){
        zeros[i] = 0;
    }
    fill_board(zeros, r, n, board);

    // A list which determines whether a value has already been set
    bool value_used[N];
    fill_value_list(N, value_used);


    // testing

    // bool ret = validate_board(r, board, M, n);
    // printf("Board full of 0s returns %d\n", ret);

    int correct_values[] = {15,13,10,14,8,4,12,9,6,5,2,16,11,1,7,19,18,17,3};
    // int correct_values[] = {1,2,3,4,5,6,7};
    // int correct_values[] = {15,13,10,14,8,4,12,9,6,5,2,16,11,1,7,19,18,17,3,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
    int correct_board[r][r][r];
    fill_board(correct_values, r, n, correct_board);

    // ret = validate_board(r, correct_board, M, n);
    // printf("Correct board returns %d\n", ret);

    // print_board(r, n, correct_board);

    // printf("Start the depth first solver\n");

    int vals_to_solve[] = {15,13,10,14,8,4,12,9,6,5,2,16,0,0,0,0,0,0,0};
    int vals_to_solve_2[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int board_to_solve[r][r][r];
    fill_board(vals_to_solve_2, r, n, board_to_solve);
    print_board(r, n, board_to_solve);

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    bool ret = solver_depth_first(r, n, N, M, board_to_solve, value_used, true);

    gettimeofday(&end_time, NULL);

    printf("Could solve the board or not? %d\n", ret);
    printf("This took %ld seconds. Used partial checks.", end_time.tv_sec - start_time.tv_sec);
    print_board(r, n, board_to_solve);

    fill_board(vals_to_solve, r, n, board_to_solve);
    fill_value_list(N, value_used);

    gettimeofday(&start_time, NULL);

    ret = solver_depth_first(r, n, N, M, board_to_solve, value_used, false);

    gettimeofday(&end_time, NULL);

    printf("Could solve the board or not? %d\n", ret);
    printf("This took %ld seconds.", end_time.tv_sec - start_time.tv_sec);
    print_board(r, n, board_to_solve);

    print_board(r, n, correct_board);

    return 0;
}