/**
 * \author Daumen Anton and Nicolas Derumigny
 * \file main.c
 * \brief Definition of the \a main() function for the SimultIte executable.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
    parse_argument(argc, argv, env);

    csrMatrix mat;
    read_Matrix(commandLineOptions.infilePath, &mat);


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

    print_mat(&mat);
    cl_print_matrix(&d_mat, queue);

    cldenseVector *x;
    x = malloc(commandLineOptions.num*sizeof(cldenseVector));
    real_t * init;
    srand(SEED);
    init = malloc(sizeof(real_t)*d_mat.num_rows);
    
    for (int i = 0; i< commandLineOptions.num; ++i) {
        clsparseInitVector(x+i);

        (x+i)->values = clCreateBuffer(context, CL_MEM_READ_WRITE, d_mat.num_rows * sizeof(real_t),
                NULL, &cl_status);
        (x+i)->num_values = d_mat.num_rows;

        // Fill x buffer with ones;
        for(int j = 0; j<d_mat.num_rows; ++j) { 
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
    while(nb_iter-- && tolerance > MAX_TOL) {
#ifdef DOUBLE_PRECISION

#else

#endif
    gram_schmidt(x, commandLineOptions.num, &context, createResult.control);
#ifdef DOUBLE_PRECISION

#else

#endif
    }
/******* GET THE DATA *******/
    for (int i = 0 ; i< commandLineOptions.num; ++i) {
        cldenseSnrm2(&norm_x, x+i, createResult.control);
        // Read  result
        real_t* host_norm_x =
            clEnqueueMapBuffer(queue, norm_x.value, CL_TRUE, CL_MAP_READ, 0, sizeof(real_t),
                    0, NULL, NULL, &cl_status);

        printf("Result : %.16f\n", *host_norm_x);
        cl_status = clEnqueueUnmapMemObject(queue, norm_x.value, host_norm_x,
                0, NULL, NULL);
        clReleaseMemObject((x+i)->values);
    }

    // Free memory
    clReleaseMemObject(norm_x.value);
    cl_free_matrix(&d_mat, createResult.control);

    cl_free(platforms, devices, context, queue, createResult);

    return EXIT_SUCCESS;
}
