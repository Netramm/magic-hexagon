/* This serial code is based on and inspired by the following two articles:
https://subscription.packtpub.com/book/programming/9781784394004/1/ch01lvl1sec08/aristotle-s-number-puzzle
https://jtp.io/2017/01/12/aristotle-number-puzzle.html
*/

#include <stdio.h>
#include <stdbool.h>

// bool validateBoard_alt(int board[], int N, int n){
//     int i, j, k;
//     int total;
    
//     // Loop thorugh each diagonal of the hexagon
//     for (k = 0; k < 3; k++){
//         // Run over each row in the given diagonal
//         for (j = 0; j < 2*n-1; j++){
//             total = 0;
//             // Add up each element in the row
//             // for (k = j*n + (j % n)){
//             break;
//             // }
//         }
//     }
//     return true;
// }

// int** generate_equations(){
//     int* equations[] = {
//         {0,1,2},
//         {3,4,5,6},
//         {7,8,9,10,11},
//         {12,13,14,15},
//         {16,17,18}
//     };

//     return equations;
// }

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

int main(int argc, char** argv) {
    // Side length of the hexagon
    // int n = 3;
    // Number of tiles to place, numbered from 1 to 19
    int N = 19;
    // Sum which has to be obtained in each row
    int M = 38;

    // The array storing the current board, initialized with 0's
    int board[N];
    int i;
    for (i = 0; i < N; i++){
        board[i] = 0;
    }

    // testing

    bool ret = validateBoard(board, M);
    printf("%d\n", ret);

    int val_board[] = {15,13,10,14,8,4,12,9,6,5,2,16,11,1,7,19,18,17,3};

    ret = validateBoard(val_board, M);
    printf("%d\n", ret);

    print_board(val_board);

    return 0;
}