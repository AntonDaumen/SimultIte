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
    printf("%d %d %d\n", sizeof(float), sizeof(double), sizeof(real_t));
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
    cl_init_matrix(&mat, &d_mat, context, queue);

    /** Allocate GPU buffers **/
    cl_int         cl_status = CL_SUCCESS;
    clsparseScalar norm_x;
    clsparseInitScalar(&norm_x);
    norm_x.value = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(real_t),
                NULL, &cl_status);

    real_t* values =
        clEnqueueMapBuffer(queue, d_mat.values, CL_TRUE, CL_MAP_READ, 0, sizeof(real_t) * d_mat.num_nonzeros,
                0, NULL, NULL, &cl_status);
    int* column =
        clEnqueueMapBuffer(queue, d_mat.col_indices, CL_TRUE, CL_MAP_READ, 0, sizeof(int) * d_mat.num_nonzeros,
                0, NULL, NULL, &cl_status);
    int* row_pointer =
        clEnqueueMapBuffer(queue, d_mat.row_pointer, CL_TRUE, CL_MAP_READ, 0, sizeof(int) * (d_mat.num_rows + 1),
                0, NULL, NULL, &cl_status);
    printf("Values: ");
    for(int i=0; i < mat.nNz; ++i)
    {
        printf("%f ", values[i]);
    }
    printf("\n");
    printf("Columns: ");
    for(int i=0; i < mat.nNz; ++i)
    {
        printf("%d ", column[i]);
    }
    printf("\n");
    printf("Rows: ");
    for(int i=0; i < mat.nRow + 1; ++i)
    {
        printf("%d ", row_pointer[i]);
    }
    printf("\n");

    printf("%d\n", sizeof(real_t));

    int N = 1024;
    cldenseVector *x;
    x = malloc(commandLineOptions.num*sizeof(cldenseVector));

    for (int i = 0 ; i< commandLineOptions.num; ++i) {
        clsparseInitVector(x+i);

        (x+i)->values = clCreateBuffer(context, CL_MEM_READ_WRITE, N * sizeof(real_t),
                NULL, &cl_status);
        (x+i)->num_values = N;

        // Fill x buffer with ones;
        real_t one = 1.0f;
        cl_status = clEnqueueFillBuffer(queue, (x+i)->values, &one, sizeof(real_t),
                0, N * sizeof(real_t), 0, NULL, NULL);
    }

    gram_schmidt(x, commandLineOptions.num, &context, createResult.control);

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

    cl_free(platforms, devices, context, queue, createResult);

    return EXIT_SUCCESS;
}
