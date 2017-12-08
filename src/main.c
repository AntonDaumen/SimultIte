/**
 * \author Daumen Anton and Nicolas Derumigny
 * \file main.c
 * \brief Definition of the \a main() function for the SimultIte executable.
 *
 */
#include "executable_options.h"
#include "matrix_reader.h"

#include <stdio.h>
#include <stdlib.h>

#include "clSPARSE.h"
#include "clSPARSE-error.h"

CommandLineOptions_t CommandLineOptions;

/**
 * \brief Main Function
 */

int main(
        int  argc,
        char *argv[],
        char *env[])
{
    printf("Executing sample clSPARSE Norm1 C\n");

    /**  Step 1. Setup OpenCL environment; **/

    cl_int cl_status = CL_SUCCESS;

    cl_platform_id* platforms = NULL;
    cl_device_id* devices = NULL;
    cl_uint num_platforms = 0;
    cl_uint num_devices = 0;


    // Get number of compatible OpenCL platforms
    cl_status = clGetPlatformIDs(0, NULL, &num_platforms);

    if (num_platforms == 0)
    {
        printf ("No OpenCL platforms found. Exiting.\n");
        return 0;
    }

    // Allocate memory for platforms
    platforms = (cl_platform_id*) malloc (num_platforms * sizeof(cl_platform_id));

    // Get platforms
    cl_status = clGetPlatformIDs(num_platforms, platforms, NULL);

    if (cl_status != CL_SUCCESS)
    {
        printf("Problem with getting platform IDs. Err: %d\n", cl_status);
        free(platforms);
        return -1;
    }


    // Get devices count from first available platform;
    cl_status = clGetDeviceIDs(platforms[ 0 ], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);

    if (num_devices == 0)
    {
        printf("No OpenCL GPU devices found on platform 0. Exiting\n");
        free(platforms);
        return -2;
    }

    // Allocate space for devices
    devices = (cl_device_id*) malloc( num_devices * sizeof(cl_device_id));

    // Get devices from platform 0;
    cl_status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, num_devices, devices, NULL);

    if (cl_status != CL_SUCCESS)
    {
        printf("Problem with getting device id from platform. Exiting\n");
        free(devices);
        free(platforms);
        return -3;
    }

    // Get context and queue
    cl_context context = clCreateContext( NULL, 1, devices, NULL, NULL, NULL );
    cl_command_queue queue = clCreateCommandQueue( context, devices[ 0 ], 0, NULL );

    /** Allocate GPU buffers **/

    int N = 1024;
    cldenseVector x;
    clsparseInitVector(&x);
    clsparseScalar norm_x;
    clsparseInitScalar(&norm_x);

    x.values = clCreateBuffer(context, CL_MEM_READ_WRITE, N * sizeof (float),
            NULL, &cl_status);
    x.num_values = N;

    // Fill x buffer with ones;
    float one = 2.0f;
    cl_status = clEnqueueFillBuffer(queue, x.values, &one, sizeof(float),
            0, N * sizeof(float), 0, NULL, NULL);

    // Allocate memory for result. No need for initializing with 0,
    // it is done internally in nrm1 function.
    norm_x.value = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float),
            NULL, &cl_status);


    /** Step 3. Init clSPARSE library **/
    clsparseStatus status = clsparseSetup();

    if (status != clsparseSuccess)
    {
        printf ("Problem with executing clsparseSetup()");
        return -1;
    }

    // Create clSPARSE control object it require queue for kernel execution
    clsparseCreateResult createResult = clsparseCreateControl( queue );
    CLSPARSE_V( createResult.status, "Failed to create clsparse control" );

    status = cldenseSnrm1(&norm_x, &x, createResult.control );

    // Read  result
    float* host_norm_x =
        clEnqueueMapBuffer(queue, norm_x.value, CL_TRUE, CL_MAP_READ, 0, sizeof(float),
                0, NULL, NULL, &cl_status);

    printf ("\tResult : %f\n", *host_norm_x);

    cl_status = clEnqueueUnmapMemObject(queue, norm_x.value, host_norm_x,
            0, NULL, NULL);

    status = clsparseReleaseControl( createResult.control );

    status = clsparseTeardown();
    if (status != clsparseSuccess)
    {
        printf ("Problem with executing clsparseTeardown()");
        return -1;
    }

    // Free memory
    clReleaseMemObject(norm_x.value);
    clReleaseMemObject(x.values);

    // Free OpenCL resources
    clReleaseCommandQueue (queue);
    clReleaseContext (context);

    free (devices);
    free (platforms);
    printf ("Program completed\n");

    return 0;
}
