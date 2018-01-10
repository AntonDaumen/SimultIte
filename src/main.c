/**
 * \author Daumen Anton and Nicolas Derumigny
 * \file main.c
 * \brief Definition of the \a main() function for the SimultIte executable.
 *
 */
#include "executable_options.h"
#include "matrix_reader.h"
#include "cl_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

CommandLineOptions_t CommandLineOptions;

/**
 * \brief Main Function
 */

int main(
        int  argc,
        char *argv[],
        char *env[])
{
    cl_platform_id       *platforms;
    cl_device_id         *devices;
    cl_context           context;
    cl_command_queue     queue;
    clsparseCreateResult createResult;

    cl_init(&platforms, &devices, &context, &queue, &createResult);

    /** Allocate GPU buffers **/
    int N = 1024;
    cldenseVector x;
    clsparseInitVector(&x);
    clsparseScalar norm_x;
    clsparseInitScalar(&norm_x);

    cl_int         cl_status = CL_SUCCESS;
    clsparseStatus status;
    x.values = clCreateBuffer(context, CL_MEM_READ_WRITE, N * sizeof(float),
            NULL, &cl_status);
    x.num_values = N;

    // Fill x buffer with ones;
    float one = 1.0f;
    cl_status = clEnqueueFillBuffer(queue, x.values, &one, sizeof(float),
            0, N * sizeof(float), 0, NULL, NULL);

    // Allocate memory for result. No need for initializing with 0,
    // it is done internally in nrm1 function.
    norm_x.value = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float),
            NULL, &cl_status);


    status = cldenseSnrm1(&norm_x, &x, createResult.control);



    // Read  result
    float* host_norm_x =
        clEnqueueMapBuffer(queue, norm_x.value, CL_TRUE, CL_MAP_READ, 0, sizeof(float),
                0, NULL, NULL, &cl_status);

    printf("Result : %f\n", *host_norm_x);

    cl_status = clEnqueueUnmapMemObject(queue, norm_x.value, host_norm_x,
            0, NULL, NULL);

    

    // Free memory
    clReleaseMemObject(norm_x.value);
    clReleaseMemObject(x.values);

    cl_free(platforms, devices, context, queue, createResult);

    return EXIT_SUCCESS;
}
