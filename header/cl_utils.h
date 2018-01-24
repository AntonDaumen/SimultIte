/**
 * \author Daumen Anton and Nicolas Derumigny
 * \file cl_utils.h
 * \brief Set of tools to simplify clSPARSE usage
 *
 */

#ifndef _CL_UTILS_H
#define _CL_UTILS_H

#include <stdio.h>
#include <stdlib.h>

#include "clSPARSE.h"
#include "clSPARSE-error.h"
#include "define.h"

/// \brief Vector with one value constant to one
extern cldenseVector  one_V;
/// \brief Scalar with constant to minus one
extern clsparseScalar minusOne_S;

/** \brief Initialize the OpenCL structures to use with the first available GPU
 */
void cl_init(
	cl_platform_id       **platforms,
	cl_device_id         **devices,
	cl_context           *context,
	cl_command_queue     *queue,
    clsparseCreateResult *createResult);

/** \brief Free the OpenCL structures created with \a cl_init()
 */
void cl_free(
	cl_platform_id       *platforms,
	cl_device_id         *devices,
	cl_context           context,
	cl_command_queue     queue,
    clsparseCreateResult createResult);

/** \brief Initialize a clsparse CSR Matrix from a CSR matrix on host
 */
void cl_init_matrix(
    csrMatrix*          host_mat,
    clsparseCsrMatrix*  d_mat,
    cl_context          context,
    cl_command_queue    queue,
    clsparseControl     control);

/** \brief Free the clsparse CSR Matrix
 */
void cl_free_matrix(
        clsparseCsrMatrix*  d_mat,
        clsparseControl     control);

/** \brief Print the clsparseCsrMatrix
 */
void cl_print_matrix(
    clsparseCsrMatrix*  d_mat,
    cl_command_queue    queue);
#endif
