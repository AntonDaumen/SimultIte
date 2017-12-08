/**
 * \author Daumen Anton and Nicolas Derumigny
 * \file matrix_reader.h
 * \brief Definition of functions that reads the matrix.
 *
 */

#ifndef _MATRIX_READER_H_
#define _MATRIX_READER_H_

#include <stdio.h>
#include <stdlib.h>
#include "../lib/header/mmio.h"

/**
 * \brief Open the Matrix Market file and read it.
 */
int read_Matrix(
    /// Name of the file to open
    const char* filename);

/**
 * \brief Read the values of a Matrix Market file line.
 *
 * If val is passed as 'NULL' then the function assumes that each line containsonly 2 values.
 *
 * If both row and col are passed as 'NULL' then the line is read but ignored.
 */
int get_line(
    /// File the line will be read from
    FILE *f,
    /// index where the data read will be written in 'row', 'col' and 'val'
    int i,
    /// Array where the row indices will be stored
    int* row,
    /// Array where the column indices will be stored
    int* col,
    /// Array where the value will be stored
    double* val);


#endif
