/**
 * \author Daumen Anton and Nicolas Derumigny
 * \file matrix_reader.c
 * \brief Definition of functions that reads the matrix.
 *
 */

#include "matrix_reader.h"

//TODO: define matrix struct and get result in it
int read_Matrix(
        const char* filename)
{
    MM_typecode matcode;
    FILE *f;
    int nNz, nCol, nRow;

    // Opening File
    if ((f = fopen(filename, "r")) == NULL)
    {
        fprintf(stderr, "[ERROR]: matrix_reader.c: FILE NOT FOUND\n\n");
        return(EXIT_FAILURE);
    }

    // Processing Matrix Market banner
    if (mm_read_banner(f, &matcode) != 0)
    {
        fprintf(stderr, "[ERROR]: matrix_reader.c: Could not process Matrix Market banner.\n");
        return(EXIT_FAILURE);
    }

    // Getting dimension of matrix
    if (mm_read_mtx_crd_size(f, &nRow, &nCol, &nNz) != 0)
    {
        fprintf(stderr, "[ERROR]: matrix_reader.c: Could not process Matrix size\n");
        return(EXIT_FAILURE);
    }

    int *row, *col;
    double *val;

    row = malloc(sizeof(int) * nNz);
    col = malloc(sizeof(int) * nNz);

    if(mm_is_pattern(matcode))
    {
        val = NULL;
    }
    else if(mm_is_real(matcode) || mm_is_integer(matcode))
    {
        val = malloc(sizeof(double) * nNz);
    }
    else if(mm_is_complex(matcode))
    {
        fprintf(stderr, "[ERROR]: matrix_reader.c: Complex Matrix not supported\n");
        return(EXIT_FAILURE);
    }
    else
    {
        fprintf(stderr, "[ERROR]: matrix_reader.c: Matrix type not recognised\n");
        return(EXIT_FAILURE);
    }

    for(int i=0; i<nNz; ++i)
    {
        if(get_line(f, i, row, col, val) != EXIT_SUCCESS)
            return(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}


int get_line(
        FILE *f,
        int i,
        int* row,
        int* col,
        double* val)
{
    if(col == NULL && row == NULL) // Throwing line away
    {
        fscanf(f, "");
    }
    else if(col == NULL || row == NULL)
    {
        fprintf(stderr, "[ERROR]: matrix_reader.c: error in 'get_line()' arguments\n");
        return(EXIT_FAILURE);
    }
    else if(val == NULL) // Matrix is binary
    {
        if (fscanf(f, "%d %d", &row[i], &col[i]) != 2)
        {
            fprintf(stderr, "[ERROR]: matrix_reader.c: Problem while reading line, expected 2 element\n");
            return(EXIT_FAILURE);
        }
    }
    else // Reading integer as double
    {
        if (fscanf(f, "%d %d %lg\n", &row[i], &col[i], &val[i]) != 3)
        {
            fprintf(stderr, "[ERROR]: matrix_reader.c: Problem while reading line, expected 3 element\n");
            return(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}
