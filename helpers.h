int find_starting_index(int n, int j);

void get_coordinates_of_row(int (*coord_arr)[3], int diagonal, int row, int n);

void fill_board(int *values, int r, int n, int (*board)[r][r]);

void fill_value_list(int N, bool *array);

void print_board(int r, int n, int (*b)[r][r]);

bool validate_board(int r, int (*board)[r][r], int M, int n);

bool validate_tile(int r, int (*board)[r][r], int M, int n, int *tile_placed);

double get_time_diff(struct timespec start, struct timespec end);