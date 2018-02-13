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
    const int M = commandLineOptions.kryl;

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
    cldenseVector *q;//vectors for Arnoldi
	cldenseVector w;
    clsparseScalar h;
	cldenseMatrix H;
    clsparseCsrMatrix H_csr;
    cl_buffer_region h_offset;
    h_offset.size = sizeof(real_t);

    clsparseInitVector(&w);
    w.values = clCreateBuffer(context, CL_MEM_READ_WRITE, d_mat.num_rows * sizeof(real_t),
                NULL, &cl_status);
    clsparseInitScalar(&h);

    cldenseInitMatrix(&H);
    H.values = clCreateBuffer(context, CL_MEM_READ_WRITE, (M+1) * M * sizeof(real_t),
                NULL, &cl_status);
    H.num_rows = M;
    H.num_cols = M;
    H.lead_dim = M;
    clEnqueueFillBuffer(queue, H.values, &zero_S, sizeof(real_t),
            0, (M+1) * M * sizeof(real_t), 0, NULL, NULL);

    clsparseInitCsrMatrix(&H_csr);
    H_csr.values = clCreateBuffer(context, CL_MEM_READ_WRITE, (M) * M * sizeof(real_t), NULL, &cl_status);
    H_csr.col_indices = clCreateBuffer(context, CL_MEM_READ_WRITE, (M) * M * sizeof(clsparseIdx_t), NULL, &cl_status);
    H_csr.row_pointer = clCreateBuffer(context, CL_MEM_READ_WRITE, (M + 1) * sizeof(clsparseIdx_t), NULL, &cl_status);

    x = malloc((commandLineOptions.num)*sizeof(cldenseVector));
    y = malloc((commandLineOptions.num)*sizeof(cldenseVector));
    q = malloc((commandLineOptions.kryl + 1)*sizeof(cldenseVector));
	real_t *init;
    srand(SEED+my_rank);
    init = malloc(sizeof(real_t)*d_mat.num_rows);

    for (int i = 0; i < M + 1; ++i)
    {
        clsparseInitVector(q+i);

        (q+i)->values = clCreateBuffer(context, CL_MEM_READ_WRITE, d_mat.num_rows * sizeof(real_t),
                NULL, &cl_status);
        (q+i)->num_values = d_mat.num_rows;
    }
    for(int j = 0; j<d_mat.num_rows; ++j)
    {
        init[j]=((real_t) rand())/RAND_MAX;
    }
    cl_status = clEnqueueWriteBuffer(queue, (q+0)->values, CL_TRUE, 0, d_mat.num_rows * sizeof(real_t),
            init, 0, NULL, NULL);
    for (int i = 0; i< commandLineOptions.num; ++i)
    {
        clsparseInitVector(x+i);

        (x+i)->values = clCreateBuffer(context, CL_MEM_READ_WRITE, d_mat.num_rows * sizeof(real_t),
                NULL, &cl_status);
        (x+i)->num_values = d_mat.num_rows;

        real_t zeroFloat = 0.0f;
        cl_status = clEnqueueFillBuffer(queue, (x+i)->values, &zeroFloat, sizeof(real_t),
                0, d_mat.num_rows * sizeof(real_t), 0, NULL, NULL);

        clsparseInitVector(y+i);

        (y+i)->values = clCreateBuffer(context, CL_MEM_READ_WRITE, M * sizeof(real_t),
                NULL, &cl_status);
        (y+i)->num_values = M;
        // Fill x buffer with random values
        for(int j = 0; j<M; ++j)
        {
            init[j]=((real_t) rand())/RAND_MAX;
        }
        cl_status = clEnqueueWriteBuffer(queue, (y+i)->values, CL_TRUE, 0, M * sizeof(real_t),
                init, 0, NULL, NULL);
    }
    free(init);
    gram_schmidt(y, commandLineOptions.num, &context, createResult.control);

/******* CORE ALGORITHM *******/
    unsigned nb_iter = NB_ITER;
    real_t tolerance = 1;

/**** Arnoli Projection *****/
#ifdef DOUBLE_PRECISION
        cldenseDnrm2(&norm_x, q+0, createResult.control);
        clsparseScalarDinv(&norm_x, createResult.control);

        cldenseDscale(q+0, &norm_x, q+0, createResult.control);

        for(int k=1; k<M; ++k)
        {
            clsparseDcsrmv(&one_S, &d_mat, q+k-1, &zero_S, q+k, createResult.control);
            cldenseDscale(q+k, &minusOne_S, q+k, createResult.control);
            for (int j=0; j<k; ++j)
            {
                h_offset.origin = sizeof(real_t) * (j * M + k - 1);
                h.value = clCreateSubBuffer(H.values, CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &h_offset, NULL);
                cldenseDdot(&h, q+j, q+k, createResult.control);
                cldenseDaxpy(q+k, &h, q+j, q+k, createResult.control);
            }
            cldenseDscale(q+k, &minusOne_S, q+k, createResult.control);

            h_offset.origin = sizeof(real_t) * (k * M + k - 1);
            h.value = clCreateSubBuffer(H.values, CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &h_offset, NULL);
            cldenseDnrm2(&h, q+k, createResult.control);
            clsparseScalarDinv(&h, createResult.control);
            cldenseDscale(q+k, &h, q+k, createResult.control);
            clsparseScalarDinv(&h, createResult.control); //Because we keep h
        }

        clsparseDdense2csr(&H, &H_csr, createResult.control);
        clsparseCsrMetaCreate(&H_csr, createResult.control);
#else
        cldenseSnrm2(&norm_x, q+0, createResult.control);
        clsparseScalarSinv(&norm_x, createResult.control);

        cldenseSscale(q+0, &norm_x, q+0, createResult.control);

        for(int k=1; k<=M; ++k)
        {
            clsparseScsrmv(&one_S, &d_mat, q+k-1, &zero_S, q+k, createResult.control);
            for (int j=0; j<k; ++j)
            {
                h_offset.origin = sizeof(real_t) * (j * M + (k-1));
                h.value = clCreateSubBuffer(H.values, CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &h_offset, NULL);
                cldenseSdot(&h, q+j, q+k, createResult.control);
                clsparseScalarSopos(&h, createResult.control);
                cldenseSaxpy(q+k, &h, q+j, q+k, createResult.control);
                clsparseScalarSopos(&h, createResult.control); //Because we keep h
            }

            h_offset.origin = sizeof(real_t) * (k * M + k - 1);
            h.value = clCreateSubBuffer(H.values, CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &h_offset, NULL);
            cldenseSnrm2(&h, q+k, createResult.control);
            clsparseScalarSinv(&h, createResult.control);
            cldenseSscale(q+k, &h, q+k, createResult.control);
            clsparseScalarSinv(&h, createResult.control); //Because we keep h
        }
        clsparseSdense2csr(&H, &H_csr, createResult.control);
        clsparseCsrMetaCreate(&H_csr, createResult.control);
#endif
        int* rwptr = malloc( (H_csr.num_rows + 1) * sizeof(int));
        for(int i=0; i < H_csr.num_rows + 1; ++i)
        {
            rwptr[i] = i*H_csr.num_cols;

        }
        clEnqueueWriteBuffer(queue, H_csr.row_pointer, CL_TRUE, 0, sizeof(real_t) * (H_csr.num_rows + 1), rwptr, 0, NULL, NULL);

        real_t *pred_nrm, *cur_nrm, shift = 0.0;
        pred_nrm = malloc(commandLineOptions.num * sizeof(real_t));
        cur_nrm = malloc(commandLineOptions.num * sizeof(real_t));
        clsparseScalar y_nrm;
        clsparseInitScalar(&y_nrm);
        y_nrm.value = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(real_t), NULL, &cl_status);
        while(nb_iter-- && tolerance > MAX_TOL)
        {
            /***** Simultaneous Iteration Method on the matrix H computed with the Arnoldi factorization*****/
            for(int k=0; k<commandLineOptions.num; ++k)
            {
#ifdef DOUBLE_PRECISION
                clsparseDcsrmv(&one_S, &H_csr, y+k, &zero_S, y+k, createResult.control);
#else
                clsparseScsrmv(&one_S, &H_csr, y+k, &zero_S, y+k, createResult.control);
#endif
            }

            gram_schmidt(y, commandLineOptions.num, &context, createResult.control);

            // Tolerance check
            for(int k=0; k<commandLineOptions.num; ++k)
            {
#ifdef DOUBLE_PRECISION
            cldenseDnrm2(&y_nrm, y+k, createResult.control);
#else
            cldenseSnrm2(&y_nrm, y+k, createResult.control);
#endif
            real_t *nrm = clEnqueueMapBuffer(queue, y_nrm.value, CL_TRUE, CL_MAP_READ, 0, sizeof(real_t), 0, NULL, NULL, &cl_status);
            cur_nrm[k] = (*nrm);
            }

            if(nb_iter < NB_ITER - 1)
            {
                shift = 0.0;
                for(int k=0; k<commandLineOptions.num; ++k)
                {
                   shift += ((cur_nrm[k] < pred_nrm[k]) ? (pred_nrm[k] - cur_nrm[k]) : (cur_nrm[k] - pred_nrm[k]));
                }

                printf("Partial Error : %g\n", shift);
            }

            real_t *tmp;
            tmp = pred_nrm; pred_nrm = cur_nrm; cur_nrm = tmp;
        }

// Recover the eigenvectors in the big space by computing x_i = Q_m y_i with y_i the eigenvectors of the Simultaneous Iteration Method, belonging to the Krylov subspace
        clsparseScalar y_scal;
        clsparseInitScalar(&y_scal);
        y_scal.value = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(real_t), NULL, &cl_status);
        for(int k=0; k<commandLineOptions.num; ++k)
        {
            for(int i=0; i<M; ++i)
            {
                h_offset.origin = sizeof(real_t) * i;
                y_scal.value = clCreateSubBuffer((y+k)->values, CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &h_offset, NULL);
#ifdef DOUBLE_PRECISION
                cldenseDaxpy(x+k, &y_scal, q+i, x+k, createResult.control);
#else
                cldenseSaxpy(x+k, &y_scal, q+i, x+k, createResult.control);
#endif
            }

        }
        /******* GET THE DATA *******/
        real_t error = 0.0f;
        cldenseVector ax_vect, lx_vect, err_vect;
        clsparseInitVector(&ax_vect);
        clsparseInitVector(&lx_vect);
        clsparseInitVector(&err_vect);

        ax_vect.values = clCreateBuffer(context, CL_MEM_READ_WRITE, d_mat.num_rows * sizeof(real_t),
                NULL, &cl_status);
        ax_vect.num_values = d_mat.num_rows;
        lx_vect.values = clCreateBuffer(context, CL_MEM_READ_WRITE, d_mat.num_rows * sizeof(real_t),
                NULL, &cl_status);
        lx_vect.num_values = d_mat.num_rows;
        err_vect.values = clCreateBuffer(context, CL_MEM_READ_WRITE, d_mat.num_rows * sizeof(real_t),
                NULL, &cl_status);
        err_vect.num_values = d_mat.num_rows;

        for (int i = 0 ; i< commandLineOptions.num; ++i)
        {
#ifdef DOUBLE_PRECISION
            clsparseDcsrmv(&one_S, &d_mat, x+i, &zero_S, &ax_vect, createResult.control);
            cldenseDnrm2(&norm_x, x+i, createResult.control);
            cldenseDscale(&lx_vect, &norm_x, x+i, createResult.control);
            cldenseDsub(&err_vect, &ax_vect, &lx_vect, createResult.control);
            cldenseDnrm2(&norm_x, &err_vect, createResult.control);
#else
            clsparseScsrmv(&one_S, &d_mat, x+i, &zero_S, &ax_vect, createResult.control);
            cldenseSnrm2(&norm_x, x+i, createResult.control);
            cldenseSscale(&lx_vect, &norm_x, x+i, createResult.control);
            cldenseSsub(&err_vect, &ax_vect, &lx_vect, createResult.control);
            cldenseSnrm2(&norm_x, &err_vect, createResult.control);
#endif
            real_t *err = clEnqueueMapBuffer(queue, norm_x.value, CL_TRUE, CL_MAP_READ, 0, sizeof(real_t), 0, NULL, NULL, &cl_status);
            error += (*err);
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
        printf("FINAL ERROR : %g\n", error);
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
