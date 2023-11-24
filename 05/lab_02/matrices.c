#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N_ROWS_MAX 10
#define M_COLS_MAX 10
#define ERR_VALUE 1
#define ERR_SIZE 2
#define ERR_EMPTY 3
#define CORRECT_INP_NUM 1

// Ввод размера массива
int input_size(size_t *const size, const size_t max_size)
{
    int exit_code = EXIT_SUCCESS;

    if (scanf("%lu", size) != CORRECT_INP_NUM)
    {
        perror("Ошибка! Размер должен быть целым числом.\n");
        exit_code = ERR_VALUE;
    }
    
    if (((*size > max_size) || (*size == 0)) && (exit_code == EXIT_SUCCESS))
    {
        perror("Ошибка! Размер матрицы должен быть больше нуля и меньше либо равен 10.\n");
        exit_code = ERR_SIZE;
    }

    return exit_code;
}

// Ввод элементов матрицы с проверкой на корректность
int input_elements(int m[N_ROWS_MAX][M_COLS_MAX], const size_t rows, const size_t cols)
{
    int exit_code = EXIT_SUCCESS;

    printf("Введите элементы матрицы (целые числа):\n");
    for (size_t i = 0; (i < rows) && (exit_code == EXIT_SUCCESS); i++)
        for (size_t j = 0; (j < cols) && (exit_code == EXIT_SUCCESS); j++)
            if (scanf("%d", &m[i][j]) != CORRECT_INP_NUM)
            {
                perror("Ошибка! Элементы должны быть целыми числами.\n");
                exit_code = ERR_VALUE;
            }

    return exit_code;
}

// Ввод размеров и элементов матрицы
int input_matrix(int m[N_ROWS_MAX][M_COLS_MAX], size_t *const rows, size_t *const cols)
{
    printf("Введите количество строк (0<N<=10):\n");
    int exit_code = input_size(rows, N_ROWS_MAX);

    if (exit_code == EXIT_SUCCESS)
    {
        printf("Введите количество столбцов (0<M<=10):\n");
        exit_code = input_size(cols, M_COLS_MAX);
        
        if (exit_code == EXIT_SUCCESS)
            exit_code = input_elements(m, *rows, *cols);
    }

    return exit_code;
}

// Вывод массива
void print_array(int *const a, const size_t size)
{
    for (size_t i = 0; i < size; i++)
        printf("%d%s", a[i], (i == size - 1) ? "\n" : " ");
}

// Вывод матрицы
void print_matrix(int m[N_ROWS_MAX][M_COLS_MAX], const size_t rows, const size_t cols)
{
    for (size_t i = 0; i < rows; i++)
        print_array(m[i], cols);
}

// Сумма цифр числа
int get_digit_sum(const int number)
{
    int sum = 0, n = abs(number);
    
    while (n > 0)
    {
        sum = sum + n % 10;
        n = n / 10;
    }

    return sum;
}

// Получение номеров строки и столбца, в которых элемент с мин. суммой цифр
void get_min_elem_index(int m[N_ROWS_MAX][M_COLS_MAX], const size_t rows, const size_t cols, size_t *const ind_row, size_t *const ind_col)
{
    int min_sum = 0, t_i = 0, t_j = 0;

    for (size_t i = 0; i < rows; i++)
        for (size_t j = 0; j < cols; j++)
            if ((get_digit_sum(m[i][j]) < min_sum) || ((i == 0) && (j == 0)))
            {
                t_i = i;
                t_j = j;
                min_sum = get_digit_sum(m[i][j]);
            }

    *ind_row = t_i;
    *ind_col = t_j;
}

// Удаление строки index из матрицы
void delete_row(int m[N_ROWS_MAX][M_COLS_MAX], size_t *const rows, const size_t cols, const size_t index)
{
    for (size_t j = 0; j < cols; j++)
        for (size_t i = index; i < *rows - 1; i++)
            m[i][j] = m[i + 1][j];
    
    *rows = *rows - 1;
}

// Удаление столбца index из матрицы
void delete_column(int m[N_ROWS_MAX][M_COLS_MAX], const size_t rows, size_t *const cols, const size_t index)
{
    for (size_t i = 0; i < rows; i++)
        for (size_t j = index; j < *cols - 1; j++)
            m[i][j] = m[i][j + 1];
    
    *cols = *cols - 1;
}

// Удаление строки и столбца, на пересек. кот. есть элемент с мин. суммой цифр
void delete_min_sum_elem(int m[N_ROWS_MAX][M_COLS_MAX], size_t *const rows, size_t *const cols)
{
    size_t del_row_index = 0, del_col_index = 0;
    get_min_elem_index(m, *rows, *cols, &del_row_index, &del_col_index);
    delete_row(m, rows, *cols, del_row_index);
    delete_column(m, *rows, cols, del_col_index);
}

int main(void)
{
    printf("Программа получает на вход матрицу (строк, столбцов - от 0 до 10) и удаляет строку и столбец, в которых содержится элемент с минимальной суммой цифр\n");
    int exit_code = EXIT_SUCCESS;
    
    int matrix[N_ROWS_MAX][M_COLS_MAX];
    size_t rows = 0, cols = 0;
    
    exit_code = input_matrix(matrix, &rows, &cols);
    
    if (exit_code == EXIT_SUCCESS)
    {
        delete_min_sum_elem(matrix, &rows, &cols);

        if (((rows == 0) || (cols == 0)) && (exit_code == EXIT_SUCCESS))
        {
            perror("Ошибка! В результате получена пустая матрица.\n");
            exit_code = ERR_EMPTY;
        }

        if (exit_code != ERR_EMPTY)
        {
            printf("\nРезультат:\n");
            print_matrix(matrix, rows, cols);
        }    
    }

    return exit_code;
}
