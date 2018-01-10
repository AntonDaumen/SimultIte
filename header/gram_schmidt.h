/** 
 * \author Daumen Anton and Nicolas Derumigny
 * \file gram_schmidt.h
 * \brief Gram-Schmidt process on GPU using clSPARSE library
 *
 */

#ifndef _GRAM_SCHMIDT_H
#define _GRAM_SCHMIDT_H

#include <stdio.h>
#include <stdlib.h>

#include "clSPARSE.h"
#include "clSPARSE-error.h"

#include "cl_utils.h"

void gram_schmidt(
	cldenseVector   *eigenVectors,
	unsigned        numberOfVector,
	cl_context      *context,
	clsparseControl control);

clsparseStatus clsparseScalarDinv(
	clsparseScalar  *scalar,
	clsparseControl control);

#endif