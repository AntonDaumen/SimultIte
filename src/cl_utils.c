/**
 * \author Daumen Anton and Nicolas Derumigny
 * \file cl_utils.c
 * \brief Set of tools to simplify clSPARSE usage
 *
 */

#include "cl_utils.h"

cldenseVector  oneS_V;
clsparseScalar minusOneS_S;

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
    *queue = clCreateCommandQueue(*context, *devices[0], 0, NULL);

    // Initialize oneD_V and minusOneD_S constant
    clsparseInitVector(&oneS_V);
    oneS_V.values = clCreateBuffer(*context, CL_MEM_READ_ONLY, sizeof(float),
            NULL, &cl_status);
    oneS_V.num_values = 1;
    float oneFloat = 1.0f;
    cl_status = clEnqueueFillBuffer(*queue, oneS_V.values, &oneFloat, sizeof(float),
            0, sizeof(float), 0, NULL, NULL);
    clsparseInitScalar(&minusOneS_S);
    minusOneS_S.value = clCreateBuffer(*context, CL_MEM_READ_ONLY, sizeof(float),
            NULL, &cl_status);
    oneFloat = -1.0f;
    cl_status = clEnqueueFillBuffer(*queue, minusOneS_S.value, &oneFloat, sizeof(float),
            0, sizeof(float), 0, NULL, NULL);

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
    clReleaseMemObject(oneS_V.values);
    clReleaseMemObject(minusOneS_S.value);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    free(devices);
    free(platforms);
}

void cl_init_matrix(
        csrMatrix*          host_mat,
        clsparseCsrMatrix*  d_mat,
		cl_context          context,
		cl_command_queue    queue)
{
	cl_int cl_status;

    clsparseInitCsrMatrix(d_mat);

    d_mat->num_nonzeros = host_mat->nNz;
    d_mat->num_rows = host_mat->nRow;
    d_mat->num_cols = host_mat->nCol;

    d_mat->values = clCreateBuffer(context, CL_MEM_READ_ONLY, d_mat->num_nonzeros * sizeof(real_t), NULL, &cl_status);
    d_mat->col_indices = clCreateBuffer(context, CL_MEM_READ_ONLY, d_mat->num_nonzeros * sizeof(clsparseIdx_t), NULL, &cl_status);
    d_mat->row_pointer = clCreateBuffer(context, CL_MEM_READ_ONLY, (d_mat->num_rows + 1) * sizeof(clsparseIdx_t), NULL, &cl_status);

    clEnqueueWriteBuffer(queue, d_mat->values, CL_TRUE, 0, sizeof(real_t) * host_mat->nNz, host_mat->vals, 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, d_mat->col_indices, CL_TRUE, 0, sizeof(int) * host_mat->nNz, host_mat->cols, 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, d_mat->row_pointer, CL_TRUE, 0, sizeof(int) * (host_mat->nRow + 1), host_mat->rows, 0, NULL, NULL);
}
