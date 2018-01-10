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

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include "clSPARSE.h"
#include "clSPARSE-error.h"

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

#endif