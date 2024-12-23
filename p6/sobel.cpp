////////////////////////////////////////////////////////////////////
//File: basic_environ.c
//
//Description: base file for environment exercises with openCL
//g++ sobel_cl.cpp -o sobel -lOpenCL -I -lm -lpthread -lX11 -ljpeg
//  
////////////////////////////////////////////////////////////////////
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __APPLE__
  #include <OpenCL/opencl.h>
#else
  #include <CL/cl.h>
#endif

#define cimg_use_jpeg
#include <iostream>
#include "CImg.h"
using namespace cimg_library;

// ABOUT ERRORS
// Remmember to check error codes from every OpenCL API call
// Info about error codes can be found at the reference manual of OpenCL
// At the following url, you can find name for each error code
//  https://gist.github.com/bmount/4a7144ce801e5569a0b6
//  https://streamhpc.com/blog/2013-04-28/opencl-error-codes/
// Following function checks errors, and in such a case, it prints the code, the string and it exits

void cl_error(cl_int code, const char *string){
    if (code != CL_SUCCESS){
        printf("%d - %s\n", code, string);
        exit(-1);
    }
}

void scanPlatformsAndDevices(cl_platform_id* platforms_ids, cl_device_id devices_ids[][10], cl_uint* n_platforms, cl_uint* n_devices, int verbose) {
    size_t t_buf = 50;
    char str_buffer[t_buf];
    size_t e_buf;

    cl_int err = clGetPlatformIDs(10, platforms_ids, n_platforms);
    cl_error(err, "Error: Failed to scan for platform IDs");
    if (verbose) printf("Number of available platforms: %d\n\n", *n_platforms);

    for (int i = 0; i < *n_platforms; i++) {
        err = clGetPlatformInfo(platforms_ids[i], CL_PLATFORM_NAME, t_buf, str_buffer, &e_buf);
        cl_error(err, "Error: Failed to get info of the platform\n");
        if (verbose) printf("\t[%d]-Platform Name: %s\n", i, str_buffer);

        // Scan devices
        err = clGetDeviceIDs(platforms_ids[i], CL_DEVICE_TYPE_ALL, 10, devices_ids[i], &(n_devices[i]));
        cl_error(err, "Error: Failed to scan for device IDs");
        if (verbose) printf("\t[%d]-Platform. Number of available devices: %d\n", i, n_devices[i]);
    }
}

cl_context createContext(cl_platform_id platform_id, cl_device_id device_id) {
    cl_int err;
    cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id, 0};
    cl_context context = clCreateContext(properties, 1, &device_id, NULL, NULL, &err);
    cl_error(err, "Failed to create a compute context");
    return context;
}

cl_command_queue createCommandQueue(cl_context context, cl_device_id device_id) {
    cl_int err;
    cl_command_queue_properties properties[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device_id, properties, &err);
    cl_error(err, "Failed to create a command queue");
    return queue;
}


cl_program loadAndBuildProgram(cl_context context, cl_device_id device_id, const char* filename) {
    cl_int err;

    // Load the kernel source code
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Failed to open kernel file %s\n", filename);
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* sourceCode = (char*)malloc(fileSize + 1);
    fread(sourceCode, sizeof(char), fileSize, file);
    sourceCode[fileSize] = '\0';
    fclose(file);

    // Create and build the program
    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&sourceCode, &fileSize, &err);
    cl_error(err, "Failed to create program with source");
    free(sourceCode);

    err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        size_t len;
        char buffer[2048];
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        fprintf(stderr, "Build log: %s\n", buffer);
        exit(EXIT_FAILURE);
    }
    return program;
}



int main(int argc, char** argv) {
    // Initialization
    int err;   
    cl_platform_id platforms_ids[10];
    cl_device_id devices_ids[10][10];
    cl_uint n_platforms, n_devices[10];
    bool verbose = false, watch = false;
    std::string img_file = "images/image.jpg";

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) verbose = true;
        if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--watch") == 0) watch = true;
        if ((strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--image") == 0) && i + 1 < argc) img_file = argv[++i];
    }

    // Scan platforms and devices
    scanPlatformsAndDevices(platforms_ids, devices_ids, &n_platforms, n_devices, verbose);

    cl_context context = createContext(platforms_ids[0], devices_ids[0][0]);
    cl_command_queue command_queue = createCommandQueue(context, devices_ids[0][0]);

    // Load and build the program
    cl_program program = loadAndBuildProgram(context, devices_ids[0][0], "kernel.cl");
    // Create the kernel
    cl_kernel kernel = clCreateKernel(program, "sobel_filter", &err);
    cl_error(err, "Failed to create kernel from the program\n");

    CImg<unsigned char> img(img_file.c_str());
    int img_w = img.width(),
        img_h = img.height(),
        img_d = img.depth(),
        img_s = img.spectrum();

    CImg <unsigned char> img_gray = img.get_RGBtoYCbCr().get_channel(0);
    img_w = img_gray.width(); img_h = img_gray.height(); img_s = img_gray.spectrum();
    if (watch) img_gray.display("Original Image");
    fprintf(stderr, "Image size: %d x %d x %d\n", img_w, img_h, img_s);

    // Create and initialize the input and output arrays at the host memory. Taking
    // into account the kernel definition, the data type of these arrays should be floating point.
    size_t VECTOR_SIZE = img_w * img_h * img_s;
    float *input = (float*) malloc(sizeof(float) * VECTOR_SIZE);
    float *output = (float*) malloc(sizeof(float) * VECTOR_SIZE);


    unsigned iter = 0;
    for (int channel = 0; channel < img_s; channel++) {
        for (int j = 0; j < img_h; j++){
            for (int i = 0; i < img_w; i++){
                input[iter++] = (float)img_gray(i, j, 0, channel);
            }
        }
    }

    float sobel_x[3][3] = {{-1, 0, 1},{-2, 0, 2},{-1, 0, 1}};
    float sobel_y[3][3] = {{1, 2, 1},{0, 0, 0},{-1, -2, -1}};

    // Create the input and output arrays at the device memory. Computing device will
    // refer to these objects as the source and destination of the power of two calculations,
    // and they will be referred as arguments of the kernel function at the file kernel.cl.
    cl_mem in_device_object = clCreateBuffer(context, CL_MEM_READ_ONLY, VECTOR_SIZE * sizeof(float), NULL, &err);
    cl_error(err, "Failed to create memory buffer at device\n");
    cl_mem sobel_x_device_object = clCreateBuffer(context, CL_MEM_READ_ONLY, 3 * 3 * sizeof(float), NULL, &err);
    cl_error(err, "Failed to create memory buffer at device\n");
    cl_mem sobel_y_device_object = clCreateBuffer(context, CL_MEM_READ_ONLY, 3 * 3 * sizeof(float), NULL, &err);
    cl_error(err, "Failed to create memory buffer at device\n");
    cl_mem out_device_object = clCreateBuffer(context, CL_MEM_WRITE_ONLY, VECTOR_SIZE * sizeof(float), NULL, &err);
    cl_error(err, "Failed to create memory buffer at device\n");


    err = clEnqueueWriteBuffer(command_queue, in_device_object, CL_TRUE, 0, sizeof(float) * VECTOR_SIZE, input, 0, NULL, NULL);
    cl_ulong write_start, write_end;
    cl_error(err, "Failed to enqueue a write command to the device memory\n");
    err = clEnqueueWriteBuffer(command_queue, sobel_x_device_object, CL_TRUE, 0, 3 * 3 * sizeof(float), sobel_x, 0, NULL, NULL);
    cl_error(err, "Failed to enqueue a write command to the device memory\n");
    err = clEnqueueWriteBuffer(command_queue, sobel_y_device_object, CL_TRUE, 0, 3 * 3 * sizeof(float), sobel_y, 0, NULL, NULL);
    cl_error(err, "Failed to enqueue a write command to the device memory\n");

    // Set the arguments to our compute kernel
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &in_device_object);
    cl_error(err, "Failed to set argument 0\n");
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &sobel_x_device_object);
    cl_error(err, "Failed to set argument 1\n");
    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &sobel_y_device_object);
    cl_error(err, "Failed to set argument 2\n");
    err = clSetKernelArg(kernel, 3, sizeof(cl_mem), &out_device_object);
    cl_error(err, "Failed to set argument 3\n");
    err = clSetKernelArg(kernel, 4, sizeof(unsigned int), &img_w);
    cl_error(err, "Failed to set argument 4\n");
    err = clSetKernelArg(kernel, 5, sizeof(unsigned int), &img_h);
    cl_error(err, "Failed to set argument 5\n");

    // Launch Kernel
    size_t global_size[2] = {img_w, img_h}; // 2D global work size
    size_t local_size[2] = {16, 16}; // 2D local work size


    err = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global_size, local_size, 0, NULL, NULL);
    cl_error(err, "Failed to launch kernel to the device\n");
    // Read data form device memory back to host memory
    err = clEnqueueReadBuffer(command_queue, out_device_object, CL_TRUE, 0, VECTOR_SIZE * sizeof(float), output, 0, NULL, NULL);
    cl_error(err, "Failed to enqueue a read command\n");

    // Create a new image with the output data
    CImg<unsigned char> output_img(img_w, img_h);
    iter = 0;
    for (int channel = 0; channel < img_s; channel++) {
        for (int j = 0; j < img_h; j++){
        for (int i = 0; i < img_w; i++){
            output_img(i, j) = (unsigned char)output[iter++];
        }
        }
    }

    // Release all the OpenCL memory objects allocated along this program.
    clReleaseMemObject(in_device_object);
    clReleaseMemObject(out_device_object);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);

    if (watch) output_img.display("Sobel filter");

    return 0;
}