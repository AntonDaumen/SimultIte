/**
 * \author Daumen Anton and Nicolas Derumigny
 * \file matrix_reader.c
 * \brief Definition of functions that reads the matrix.
 *
 */

#include "matrix_reader.h"

//TODO: define matrix struct and get result in it
int read_Matrix(
        const char* filename,
        csrMatrix* mat)
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
    if (mm_read_mtx_crd_size(f, &(mat->nRow), &(mat->nCol), &(mat->nNz)) != 0)
    {
        fprintf(stderr, "[ERROR]: matrix_reader.c: Could not process Matrix size\n");
        return(EXIT_FAILURE);
    }

    mat->rows = malloc(sizeof(int) * (mat->nRow+1));
    mat->cols = malloc(sizeof(int) * mat->nNz);

    memset(mat->rows, 0, sizeof(int) * (mat->nRow+1));

    if(mm_is_pattern(matcode))
    {
        mat->vals = NULL;
    }
    else if(mm_is_real(matcode) || mm_is_integer(matcode))
    {
        mat->vals = malloc(sizeof(real_t) * mat->nNz);
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

    for(int i=0; i<mat->nNz; ++i)
    {
        if(get_line(f, i, mat->rows, mat->cols, mat->vals) != EXIT_SUCCESS)
            return(EXIT_FAILURE);
    }

    for(int i=1; i < mat->nRow + 1; ++i)
    {
        mat->rows[i] += mat->rows[i-1];
    }

    printf("Values: ");
    for(int i=0; i < mat->nNz; ++i)
    {
        printf("%f ", mat->vals[i]);
    }
    printf("\n");
    printf("Columns: ");
    for(int i=0; i < mat->nNz; ++i)
    {
        printf("%d ", mat->cols[i]);
    }
    printf("\n");
    printf("Rows: ");
    for(int i=0; i < mat->nRow + 1; ++i)
    {
        printf("%d ", mat->rows[i]);
    }
    printf("\n");

    return EXIT_SUCCESS;
}


int get_line(
        FILE *f,
        int i,
        int* rows,
        int* cols,
        real_t* vals)
{
    int row;
    if(cols == NULL && rows == NULL) // Throwing line away
    {
        fscanf(f, "");
        return EXIT_SUCCESS;
    }
    else if(cols == NULL || rows == NULL)
    {
        fprintf(stderr, "[ERROR]: matrix_reader.c: error in 'get_line()' arguments\n");
        return(EXIT_FAILURE);
    }
    else if(vals == NULL) // Matrix is binary
    {
        if (fscanf(f, "%d %d", &row, &cols[i]) != 2)
        {
            fprintf(stderr, "[ERROR]: matrix_reader.c: Problem while reading line, expected 2 element\n");
            return(EXIT_FAILURE);
        }
    }
    else // Reading integer as double
    {
#ifdef DOUBLE_PRECISION
        if (fscanf(f, "%d %d %lg\n", &row, &cols[i], &vals[i]) != 3)
#else
        if (fscanf(f, "%d %d %f\n", &row, &cols[i], &vals[i]) != 3)
#endif
        {
            fprintf(stderr, "[ERROR]: matrix_reader.c: Problem while reading line, expected 3 element\n");
            return(EXIT_FAILURE);
        }
    }
    rows[row]++;

    return EXIT_SUCCESS;
}
