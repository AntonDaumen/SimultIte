/** 
 * \author Daumen Anton and Nicolas Derumigny
 * \file cl_utils.c
 * \brief Set of tools to simplify clSPARSE usage
 *
 */

#include "cl_utils.h"

cldenseVector  oneD_V;
clsparseScalar minusOneD_S;

void cl_init(
		cl_platform_id       **platforms,
		cl_device_id         **devices,
		cl_context           *context,
		cl_command_queue     *queue,
        clsparseCreateResult *createResult)
{
	cl_int cl_status = CL_SUCCESS;
    cl_uint num_platforms = 0;
    cl_uint num_devices = 0;

    // Get number of compatible OpenCL platforms
    cl_status = clGetPlatformIDs(0, NULL, &num_platforms);

    if (num_platforms == 0)
    {
        fprintf(stderr, "[CRITICAL ERROR] No OpenCL platforms found.\n");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for platforms
    *platforms = (cl_platform_id*) malloc(num_platforms * sizeof(cl_platform_id));

    // Get platforms
    cl_status = clGetPlatformIDs(num_platforms, *platforms, NULL);

    if (cl_status != CL_SUCCESS)
    {
        fprintf(stderr, "[CRITICAL ERROR] Problem with getting platform IDs (status %d)\n", cl_status);
        free(*platforms);
        exit(EXIT_FAILURE);
    }

    // Get devices count from first available platform;
    cl_status = clGetDeviceIDs(*platforms[0], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);

    if (num_devices == 0)
    {
        fprintf(stderr, "[CRITICAL ERROR] No OpenCL GPU devices found on platform 0.\n");
        free(*platforms);
        exit(EXIT_FAILURE);
    }

    // Allocate space for devices
    *devices = ((cl_device_id*) malloc(num_devices * sizeof(cl_device_id)));

    // Get devices from platform 0;
    cl_status = clGetDeviceIDs(*platforms[0], CL_DEVICE_TYPE_GPU, num_devices, *devices, NULL);

    if (cl_status != CL_SUCCESS)
    {
        printf("[CRITICAL ERROR] Problem with getting device id from platform.\n");
        free(*devices);
        free(*platforms);
        exit(EXIT_FAILURE);
    }

    // Get context and queue
    *context = clCreateContext(NULL, 1, *devices, NULL, NULL, NULL);
    *queue = clCreateCommandQueueWithProperties(*context, *devices[0], NULL, NULL);

    // Initialize oneD_V and minusOneD_S constant
    clsparseInitVector(&oneD_V);
    oneD_V.values = clCreateBuffer(*context, CL_MEM_WRITE_ONLY, sizeof(double),
            NULL, &cl_status);
    oneD_V.num_values = 1;
    double oneDouble = 1.0f;
    cl_status = clEnqueueFillBuffer(*queue, oneD_V.values, &oneDouble, sizeof(double),
            0, sizeof(double), 0, NULL, NULL);
    clsparseInitScalar(&minusOneD_S);
    minusOneD_S.value = clCreateBuffer(*context, CL_MEM_WRITE_ONLY, sizeof(double),
            NULL, &cl_status);
    oneDouble = -1.0f;
    cl_status = clEnqueueFillBuffer(*queue, minusOneD_S.value, &oneDouble, sizeof(double),
            0, sizeof(double), 0, NULL, NULL);

    clsparseStatus status = clsparseSetup();
    if (status != clsparseSuccess)
    {
        fprintf(stderr, "[CRITICAL ERROR] Problem with executing clsparseSetup()");
        exit(EXIT_FAILURE);
    }

    // Create clSPARSE control object it requires queue for kernel execution
    *createResult = clsparseCreateControl(*queue);
    CLSPARSE_V(createResult->status, "Failed to create clsparse control");
}

void cl_free(
		cl_platform_id       *platforms,
		cl_device_id         *devices,
		cl_context           context,
		cl_command_queue     queue,
        clsparseCreateResult createResult)
{
    cl_int         cl_status = CL_SUCCESS;
    clsparseStatus status;

    status = clsparseReleaseControl(createResult.control);

    status = clsparseTeardown();
    if (status != clsparseSuccess)
    {
        fprintf(stderr, "[CRITICAL ERROR] Problem with executing clsparseTeardown()");
        exit(EXIT_FAILURE);
    }

    // Free OpenCL resources
    clReleaseMemObject(oneD_V.values);
    clReleaseMemObject(minusOneD_S.value);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    free(devices);
    free(platforms);
}