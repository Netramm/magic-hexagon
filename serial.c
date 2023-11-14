/* This serial code is based on and inspired by the following two articles:
https://subscription.packtpub.com/book/programming/9781784394004/1/ch01lvl1sec08/aristotle-s-number-puzzle
https://jtp.io/2017/01/12/aristotle-number-puzzle.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool validateBoard_alt(int board[], int M, int n){
    int i, j, k;
    int total, end;
    int pos = 0;
    
    // Loop through each diagonal of the hexagon
    for (i = 0; i < 1; i++){
        // Run over each row in the given diagonal
        for (j = 0; j < 2*n-1; j++){
            total = 0;
            // Add up each element in the row
            end = 2*n-1-abs(n-1-j);
            for (k = 0; k < end; k++){
                total += board[pos + k];
            }
            // printf("Sum: %d with j %d\n", total, j);
            // Check if sum is equal to M
            if (total != M){
                return false;
            }
            // Increase the counter which translates to the array positions
            pos += end;
        }
    }
    return true;
}

bool validateBoard(int b[], int M){
    
    if (b[0] + b[1] + b[2] == M &&
        b[3] + b[4] + b[5] + b[6] == M &&
        b[7] + b[8] + b[9] + b[10] + b[11] == M &&
        b[12] + b[13] + b[14] + b[15] == M &&
        b[16] + b[17] + b[18] == M &&
        b[0] + b[3] + b[7] == M &&
        b[1] + b[4] + b[8] + b[12] == M &&
        b[2] + b[5] + b[9] + b[13] + b[16] == M &&
        b[6] + b[10] + b[14] + b[17] == M &&
        b[11] + b[15] + b[18] == M &&
        b[2] + b[6] + b[11] == M &&
        b[1] + b[5] + b[10] + b[15] == M &&
        b[0] + b[4] + b[9] + b[14] + b[18] == M &&
        b[3] + b[8] + b[13] + b[17] == M &&
        b[7] + b[12] + b[16] == M){
        return true;
    }
    return false;
}

void print_board(int b[]){
    printf("    %2d  %2d  %2d    \n  %2d  %2d  %2d  %2d  \n%2d  %2d  %2d  %2d  %2d\n  %2d  %2d  %2d  %2d  \n    %2d  %2d  %2d    \n", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15], b[16], b[17], b[18]);
}

bool validateBoard_alt2(int board[][][], int M, int r, int n){
    int i, j, k;
    int total, end;
    int pos = 0;
    
    // Loop through each diagonal of the hexagon
    for (i = 0; i < 3; i++){
        // Run over each row in the given diagonal
        for (j = 0; j < r; j++){
            total = 0;
            end = r-abs(n-1-j);
            // Define which diagonal is fixed
            if (i == 0){
                // Add up each element in the row
                for (k = 0; k < end; k++){
                    total += board[j][*y][*z];
                }
            }
            else if(i == 1){
                // Add up each element in the row
                for (k = 0; k < end; k++){
                    total += board[*x][j][*z];
                }
            }
            else{
                // Add up each element in the row
                for (k = 0; k < end; k++){
                    total += board[*x][*y][j];
                }
            }
            
            // Check if sum is equal to M
            if (total != M){
                return false;
            }
        }
    }
    return true;
}

int main(int argc, char** argv) {
    // Side length of the hexagon
    int n = 3;
    // Numbe of rows of the hexagon
    int r = n*2-1;
    // Number of tiles to place, numbered from 1 to 19
    int N = 19;
    // Sum which has to be obtained in each row
    int M = 38;

    // The mutidimensional-array storing the current board, initialized with 0's
    // This representation is inspired by: https://www.redblobgames.com/grids/hexagons/
    int board[r][r][r];
    int i,j,k;
    for (i = 0; i < r; i++){
        for (j = 0; j < r; j++){
            for (k = 0; k < r; k++){
                board[i][j][k] = 0;
            }
        }
    }

    // testing

    bool ret = validateBoard(board, M);
    printf("%d\n", ret);
    ret = validateBoard_alt(board, M, n);
    printf("%d\n", ret);

    int val_board[] = {15,13,10,14,8,4,12,9,6,5,2,16,11,1,7,19,18,17,3};

    ret = validateBoard(val_board, M);
    printf("%d\n", ret);

    ret = validateBoard_alt(val_board, M, n);
    printf("%d\n", ret);

    print_board(val_board);

    return 0;
}