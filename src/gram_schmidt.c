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
	clsparseEnableAsync(control, CL_TRUE);


	clsparseScalar norm;
	clsparseInitScalar(&norm);
	norm.value = clCreateBuffer(*context, CL_MEM_READ_WRITE, sizeof(real_t),
            NULL, &cl_status);

	for (int i = 0; i<numberOfVectors; ++i)
	{
#ifdef DOUBLE_PRECISION
        cldenseDscale(&eigenVectors[i], &minusOne_S, &eigenVectors[i], control);
		for(int j = 0; j<i; ++j)
		{
			cldenseDdot(&norm, &eigenVectors[j], &eigenVectors[i], control);
			cldenseDaxpy(&eigenVectors[i], &norm, &eigenVectors[j], &eigenVectors[i], control);

		}
		cldenseDscale(&eigenVectors[i], &minusOne_S, &eigenVectors[i], control);
		cldenseDnrm2(&norm, &eigenVectors[i], control);
		clsparseScalarDinv(&norm, control);
		cldenseDscale(&eigenVectors[i], &norm, &eigenVectors[i], control);
#else
        cldenseSscale(&eigenVectors[i], &minusOne_S, &eigenVectors[i], control);
		for(int j = 0; j<i; ++j)
		{
			cldenseSdot(&norm, &eigenVectors[j], &eigenVectors[i], control);
			cldenseSaxpy(&eigenVectors[i], &norm, &eigenVectors[j], &eigenVectors[i], control);

		}
		cldenseSscale(&eigenVectors[i], &minusOne_S, &eigenVectors[i], control);
		cldenseSnrm2(&norm, &eigenVectors[i], control);
		clsparseScalarSinv(&norm, control);
		cldenseSscale(&eigenVectors[i], &norm, &eigenVectors[i], control);
#endif
    }

	clReleaseMemObject(norm.value);
}

clsparseStatus clsparseScalarSinv(
		clsparseScalar *scalar,
		clsparseControl control)
{
	cldenseVector  vector;
	vector.num_values = 1u;
	vector.values = scalar->value;
	vector.off_values = scalar->off_value;

	return cldenseSdiv(&vector, &one_V, &vector, control);
}


clsparseStatus clsparseScalarDinv(
		clsparseScalar *scalar,
		clsparseControl control)
{
	cldenseVector  vector;
	vector.num_values = 1u;
	vector.values = scalar->value;
	vector.off_values = scalar->off_value;

	return cldenseDdiv(&vector, &one_V, &vector, control);
}
