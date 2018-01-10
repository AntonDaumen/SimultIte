/** 
 * \author Daumen Anton and Nicolas Derumigny
 * \file gram_schmidt.c
 * \brief Gram-Schmidt process on GPU using clSPARSE library
 *
 */

#include "gram_schmidt.h"

void gram_schmidt(
		cldenseVector   *eigenVectors,
		unsigned        numberOfVectors,
		cl_context      *context,
		clsparseControl control)
{
	cl_int         cl_status;

	clsparseScalar norm;
	clsparseInitScalar(&norm);
	norm.value = clCreateBuffer(*context, CL_MEM_WRITE_ONLY, sizeof(double),
            NULL, &cl_status);

	for (int i = 0; i<numberOfVectors; ++i)
	{
		cldenseDscale(&eigenVectors[i], &minusOneD_S, &eigenVectors[i], control);
		for(int j = 0; j<i; ++j)
		{
			cldenseDdot(&norm, &eigenVectors[j], &eigenVectors[i], control);
			cldenseDaxpy(&eigenVectors[i], &norm, &eigenVectors[j], &eigenVectors[i], control);

		}
		cldenseDscale(&eigenVectors[i], &minusOneD_S, &eigenVectors[i], control);
		cldenseDnrm2(&norm, &eigenVectors[i], control);
		clsparseScalarDinv(&norm, control);
		cldenseDscale(&eigenVectors[i], &norm, &eigenVectors[i], control);
	}

	clReleaseMemObject(norm.value);
}

clsparseStatus clsparseScalarDinv(
		clsparseScalar *scalar,
		clsparseControl control)
{
	cldenseVector  vector;
	vector.num_values = 1u;
	vector.values = scalar->value;
	vector.off_values = scalar->off_value;

	return cldenseDdiv(&vector, &oneD_V, &vector, control);
}