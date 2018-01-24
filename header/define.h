/**
 * \author Daumen Anton and Nicolas Derumigny
 * \file matrix_reader.h
 * \brief Definition of functions that reads the matrix.
 *
 */

#ifndef _DEFINE_H_
#define _DEFINE_H_

//#define DOUBLE_PRECISION
#ifdef DOUBLE_PRECISION
typedef double real_t;
#else
typedef float real_t;
#endif

typedef struct csrMatrix{
    int*    rows;
    int*    cols;
    real_t* vals;
    int     nNz;
    int     nRow;
    int     nCol;
}csrMatrix;


#endif
