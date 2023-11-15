/* This serial code is based on and inspired by the following two articles:
https://subscription.packtpub.com/book/programming/9781784394004/1/ch01lvl1sec08/aristotle-s-number-puzzle
https://jtp.io/2017/01/12/aristotle-number-puzzle.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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

bool validateBoard(int r, int (*board)[r][r], int M, int n){
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


    // testing

    bool ret = validateBoard(r, board, M, n);
    printf("Board full of 0s returns %d\n", ret);

    int correct_values[] = {15,13,10,14,8,4,12,9,6,5,2,16,11,1,7,19,18,17,3};
    // int correct_values[] = {1,2,3,4,5,6,7};
    // int correct_values[] = {15,13,10,14,8,4,12,9,6,5,2,16,11,1,7,19,18,17,3,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
    int correct_board[r][r][r];
    fill_board(correct_values, r, n, correct_board);

    ret = validateBoard(r, correct_board, M, n);
    printf("Correct board returns %d\n", ret);

    print_board(r, n, correct_board);

    return 0;
}