/**
 * \author Daumen Anton and Nicolas Derumigny
 * \file define.h
 * \brief Defines the constant values used by the program and whether to use floats or doubles.
 *
 */

#ifndef _DEFINE_H_
#define _DEFINE_H_

#ifndef NB_ITER
#define NB_ITER 500
#endif
#ifndef MAX_TOL
#define MAX_TOL 1e-8
#endif

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#ifdef DOUBLE_PRECISION
typedef double real_t;
#else
typedef float real_t;
#endif

#define SEED 1

typedef struct csrMatrix{
    int*    rows;
    int*    cols;
    real_t* vals;
    int     nNz;
    int     nRow;
    int     nCol;
}csrMatrix;


#endif
