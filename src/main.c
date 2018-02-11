/**
 * \author Daumen Anton and Nicolas Derumigny
 * \file main.c
 * \brief Definition of the \a main() function for the SimultIte executable.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>

#include "executable_options.h"
#include "matrix_reader.h"
#include "cl_utils.h"
#include "gram_schmidt.h"

/**
 * \brief Main Function
 */
int main(
        int  argc,
        char *argv[],
        char *env[])
{
    MPI_Init(&argc, &argv);
    int num_proc; MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    int my_rank;  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    parse_argument(argc, argv, env);
    csrMatrix mat;
    int err;

    err = read_Matrix(commandLineOptions.infilePath, &mat);
    if(err == EXIT_FAILURE)
    {
        if (my_rank == 0) fprintf(stderr,"[ERROR]: Error while reading matrix\n");
        MPI_Finalize();
        return(EXIT_FAILURE);
    }


    cl_platform_id       *platforms;
    cl_device_id         *devices;
    cl_context           context;
    cl_command_queue     queue;
    clsparseCreateResult createResult;

    cl_init(&platforms, &devices, &context, &queue, &createResult);

    clsparseCsrMatrix d_mat;
    cl_init_matrix(&mat, &d_mat, context, queue, createResult.control);

    /** Allocate GPU buffers **/
    cl_int         cl_status = CL_SUCCESS;
    clsparseScalar norm_x;
    clsparseInitScalar(&norm_x);
    norm_x.value = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(real_t),
                NULL, &cl_status);

    if(my_rank == 0)
    {
        //print_mat(&mat);
        cl_print_matrix(&d_mat, queue);
    }

    cldenseVector *x;//eigenvalues
    cldenseVector *y;//eigenvalues of the reduced problem
	cldenseVector w;
    cldenseVector randVect;//used for Arnoldi's projection
    clsparseScalar h;
	cldenseMatrix H;

    clsparseInitVector(&w);
    w.values = clCreateBuffer(context, CL_MEM_READ_WRITE, d_mat.num_rows * sizeof(real_t),
                NULL, &cl_status);
    clsparseInitVector(&randVect);
    randVect.values = clCreateBuffer(context, CL_MEM_READ_WRITE, d_mat.num_rows * sizeof(real_t),
                NULL, &cl_status);
    clsparseInitScalar(&h);
    h.value = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(real_t),
                NULL, &cl_status);

    x = malloc((commandLineOptions.num+1)*sizeof(cldenseVector));
    y = malloc((commandLineOptions.num+1)*sizeof(cldenseVector));
	real_t * init;
    srand(SEED);
    init = malloc(sizeof(real_t)*d_mat.num_rows);

    for(int j = 0; j<d_mat.num_rows; ++j)
    {
        init[j]=((real_t) rand())/RAND_MAX;
    }
    cl_status = clEnqueueWriteBuffer(queue, randVect.values, CL_TRUE, 0, d_mat.num_rows * sizeof(real_t),
            init, 0, NULL, NULL);

    for (int i = 0; i< commandLineOptions.num+1; ++i)
    {
        clsparseInitVector(x+i);

        (x+i)->values = clCreateBuffer(context, CL_MEM_READ_WRITE, d_mat.num_rows * sizeof(real_t),
                NULL, &cl_status);
        (x+i)->num_values = d_mat.num_rows;

        // Fill x buffer with random values
        for(int j = 0; j<d_mat.num_rows; ++j)
        {
            init[j]=((real_t) rand())/RAND_MAX;
        }
        cl_status = clEnqueueWriteBuffer(queue, (x+i)->values, CL_TRUE, 0, d_mat.num_rows * sizeof(real_t),
                init, 0, NULL, NULL);
    }
    free(init);
    gram_schmidt(x, commandLineOptions.num, &context, createResult.control);

/******* CORE ALGORITHM *******/
    unsigned nb_iter = NB_ITER;
    real_t tolerance = 1;

/**** Arnoli Projection *****/
    //TODO : keep H as a matrix
#ifdef DOUBLE_PRECISION
        cldenseDnrm1(&norm_x, &randVect, createResult.control);
        clsparseScalarDinv(&norm_x, createResult.control);

        cldenseDscale(x+0, &norm_x, &randVect, createResult.control);

        for(int k=1; k<commandLineOptions.num; ++k)
        {
            clsparseDcsrmv(&one_S, &d_mat, x+k-1, &zero_S, &w, createResult.control);
            cldenseDscale(&w, &minusOne_S, &w, createResult.control);
            for (int j=1; j<k; ++j)
            {
                cldenseDdot(&h, &w, &randVect, createResult.control);
                cldenseDaxpy(&w, &h, x+j, &w, createResult.control);
            }
            cldenseDscale(&w, &minusOne_S, &w, createResult.control);

            cldenseDnrm1(&h, &w, createResult.control);
            clsparseScalarDinv(&h, createResult.control);
            cldenseDscale(x+k+1, &h, &w, createResult.control);
        }
#else
        cldenseSnrm1(&norm_x, &randVect, createResult.control);
        clsparseScalarSinv(&norm_x, createResult.control);

        cldenseSscale(x+0, &norm_x, &randVect, createResult.control);

        for(int k=1; k<commandLineOptions.num; ++k)
        {
            clsparseScsrmv(&one_S, &d_mat, x+k, &zero_S, &w, createResult.control);
            cldenseSscale(&w, &minusOne_S, &w, createResult.control);
            for (int j=1; j<k; ++j)
            {
                cldenseSdot(&h, &w, &randVect, createResult.control);
                cldenseSaxpy(&w, &h, x+j, &w, createResult.control);
            }
            cldenseSscale(&w, &minusOne_S, &w, createResult.control);

            cldenseSnrm1(&h, &w, createResult.control);
            clsparseScalarSinv(&h, createResult.control);
            cldenseSscale(x+k+1, &h, &w, createResult.control);
        }
#endif

    while(nb_iter-- && tolerance > MAX_TOL)
    {
        /***** Simultaneous Iteration Method on the matrix H computed with the Arnoldi factorization*****/
		for(int k=0; k<commandLineOptions.num; ++k)
			{
#ifdef DOUBLE_PRECISION
    		  	//cldenseDmul(y+k, H, y+k, createResult.control);
#else
				//cldenseSmul(y+k, H, y+k, createResult.control);
#endif
			}
		gram_schmidt(x, commandLineOptions.num, &context, createResult.control);
    }

// Recover the eigenvectors in the big space by computing x_i = Q_m y_i with y_i the eigenvectors of the Simultaneous Iteration Method, belonging to the Krylov subspace
	for(int k=0; k<commandLineOptions.num; ++k)
	{
#ifdef DOUBLE_PRECISION
		//cldenseDmul(x+k, Q, y+k, createResult.control);
#else
		//cldenseSmul(x+k, Q, y+k, createResult.control);
#endif
	}
/******* GET THE DATA *******/
    real_t error;
    for (int i = 0 ; i< commandLineOptions.num+1; ++i)
    {
        //TODO : error = sum norm( (A - lambda_i . Id) u_i) )
    }

/****** Sharing the results *****/
    // Assuming the error is in error
    real_t *errors;
    int    *array_min;
    if(my_rank == 0)
    {
        errors=malloc(num_proc*sizeof(real_t));
        array_min=malloc(num_proc*sizeof(int));
    }
#ifdef DOUBLE_PRECISION
    MPI_Gather(&error, 1, MPI_DOUBLE, errors, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#else
    MPI_Gather(&error, 1, MPI_FLOAT, errors, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
#endif

    if (my_rank == 0) {
        // compute the min
        real_t min = *errors;
        int min_index=0;
        array_min[0]=1;
        for (int i=1; i<num_proc; ++i)
        {
            array_min[i]=0;
            if (min<errors[i])
            {
                min = errors[i];
                array_min[min_index]=0;
                array_min[i]=1;
                min_index = i;
            }
        }
    }
    int is_min;
    MPI_Scatter(array_min, 1, MPI_INT, &is_min, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if(is_min) {
        //TODO print eigenvalues here
    }

    // Free memory
    if(my_rank == 0) free(errors);
    clReleaseMemObject(norm_x.value);
    clReleaseMemObject(h.value);
    clReleaseMemObject(w.values);
    cl_free_matrix(&d_mat);

    cl_free(platforms, devices, context, queue, createResult);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
