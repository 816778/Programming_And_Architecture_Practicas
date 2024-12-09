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
#include <ctime>
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

    // Leer archivo fuente
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

    // Crear y compilar programa
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


void createBuffers(cl_context context, cl_mem* in_buffer, cl_mem* out_buffer, cl_mem* sobel_x_buffer, cl_mem* sobel_y_buffer, 
                   size_t vector_size, const float* sobel_x, const float* sobel_y, cl_command_queue command_queue) {
    cl_int err;
    *in_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, vector_size * sizeof(float), NULL, &err);
    cl_error(err, "Failed to create input buffer");

    *out_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, vector_size * sizeof(float), NULL, &err);
    cl_error(err, "Failed to create output buffer");

    *sobel_x_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, 3 * 3 * sizeof(float), NULL, &err);
    cl_error(err, "Failed to create Sobel X buffer");

    *sobel_y_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, 3 * 3 * sizeof(float), NULL, &err);
    cl_error(err, "Failed to create Sobel Y buffer");

    // Transferir datos de Sobel
    clEnqueueWriteBuffer(command_queue, *sobel_x_buffer, CL_TRUE, 0, 3 * 3 * sizeof(float), sobel_x, 0, NULL, NULL);
    clEnqueueWriteBuffer(command_queue, *sobel_y_buffer, CL_TRUE, 0, 3 * 3 * sizeof(float), sobel_y, 0, NULL, NULL);
}


void executeKernel(cl_command_queue queue, cl_kernel kernel, size_t* global_size, size_t* local_size, cl_mem in_buffer, cl_mem out_buffer, 
                   cl_mem sobel_x_buffer, cl_mem sobel_y_buffer, unsigned int width, unsigned int height) {
    cl_int err;
    

    // Configurar argumentos del kernel
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &in_buffer);
    cl_error(err, "Failed to set argument 0");
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &sobel_x_buffer);
    cl_error(err, "Failed to set argument 1");
    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &sobel_y_buffer);
    cl_error(err, "Failed to set argument 2");
    err = clSetKernelArg(kernel, 3, sizeof(cl_mem), &out_buffer);
    cl_error(err, "Failed to set argument 3");
    err = clSetKernelArg(kernel, 4, sizeof(unsigned int), &width);
    cl_error(err, "Failed to set argument 4");
    err = clSetKernelArg(kernel, 5, sizeof(unsigned int), &height);
    cl_error(err, "Failed to set argument 5");

    // Ejecutar kernel
    cl_event kernel_event;
    err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size, local_size, 0, NULL, &kernel_event);
    cl_error(err, "Failed to enqueue kernel");

    clWaitForEvents(1, &kernel_event);
    clReleaseEvent(kernel_event);
}




int main(int argc, char** argv) {
    clock_t start_time_programm = clock();
    // Inicialización
    int err;   
    cl_platform_id platforms_ids[10];
    cl_device_id devices_ids[10][10];
    cl_uint n_platforms, n_devices[10];
    int verbose = 0;
    std::string img_file = "images/image.jpg";

    // Escanear plataformas y dispositivos
    scanPlatformsAndDevices(platforms_ids, devices_ids, &n_platforms, n_devices, verbose);

    cl_context context = createContext(platforms_ids[0], devices_ids[0][0]);
    cl_command_queue command_queue = createCommandQueue(context, devices_ids[0][0]);

    // Cargar y compilar kernel
    cl_program program = loadAndBuildProgram(context, devices_ids[0][0], "kernel.cl");
    // Create a compute kernel with the program we want to run
    cl_kernel kernel = clCreateKernel(program, "sobel_filter", &err);
    cl_error(err, "Failed to create kernel from the program\n");

    CImg<unsigned char> img(img_file.c_str());
    int img_w = img.width(),
        img_h = img.height(),
        img_d = img.depth(),
        img_s = img.spectrum();

    CImg <unsigned char> img_gray = img.get_RGBtoYCbCr().get_channel(0);
    img_w = img_gray.width(); img_h = img_gray.height(); img_s = img_gray.spectrum();
    // img_gray.display("Original Image");
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

    // Build sobel kernels
    float sobel_x[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    float sobel_y[3][3] = {
        {1, 2, 1},
        {0, 0, 0},
        {-1, -2, -1}
    };

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


    cl_event write_event;
    err = clEnqueueWriteBuffer(command_queue, in_device_object, CL_TRUE, 0, sizeof(float) * VECTOR_SIZE, input, 0, NULL, &write_event);
    cl_ulong write_start, write_end;
    clGetEventProfilingInfo(write_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &write_start, NULL);
    clGetEventProfilingInfo(write_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &write_end, NULL);
    double write_time = (write_end - write_start) * 1e-9;
    double host_to_device_bandwidth = (VECTOR_SIZE * sizeof(float)) / write_time / 1e9; // GB/s
    std::cout << "\n\nHost to Device Bandwidth: " << host_to_device_bandwidth << " GB/s";

    clReleaseEvent(write_event);

    cl_error(err, "Failed to enqueue a write command to the device memory\n");
    err = clEnqueueWriteBuffer(command_queue, sobel_x_device_object, CL_TRUE, 0, 3 * 3 * sizeof(float), sobel_x, 0, NULL, NULL);
    cl_error(err, "Failed to enqueue a write command to the device memory\n");
    err = clEnqueueWriteBuffer(command_queue, sobel_y_device_object, CL_TRUE, 0, 3 * 3 * sizeof(float), sobel_y, 0, NULL, NULL);
    cl_error(err, "Failed to enqueue a write command to the device memory\n");

    // Configurar los argumentos del kernel
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
    size_t global_size[2] = {img_w, img_h};  // Dimensiones globales para X e Y
    size_t local_size[2] = {16, 16};      

    cl_event kernel_event;
    err = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global_size, local_size, 0, NULL, &kernel_event);
    cl_error(err, "Failed to launch kernel to the device\n");

    clWaitForEvents(1, &kernel_event);

    cl_ulong start_time, end_time;
    clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start_time, NULL);
    clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end_time, NULL);

    double kernel_execution_time = (end_time - start_time) * 1e-9; // Convertir de nanosegundos a segundos
    std::cout << "\nKernel execution time: " << kernel_execution_time << " seconds";
    clReleaseEvent(kernel_event);


    // Read data form device memory back to host memory
    cl_event read_event;
    err = clEnqueueReadBuffer(command_queue, out_device_object, CL_TRUE, 0, VECTOR_SIZE * sizeof(float), output, 0, NULL, &read_event);
    cl_error(err, "Failed to enqueue a read command\n");
    cl_ulong read_start, read_end;
    clGetEventProfilingInfo(read_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &read_start, NULL);
    clGetEventProfilingInfo(read_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &read_end, NULL);
    double read_time = (read_end - read_start) * 1e-9;
    double device_to_host_bandwidth = (VECTOR_SIZE * sizeof(float)) / read_time / 1e9; // GB/s
    std::cout << "\nDevice to Host Bandwidth: " << device_to_host_bandwidth << " GB/s" << std::endl;

    clReleaseEvent(read_event);

    // Volcar los valores de salida es una imagen cimg
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


    clock_t end_time_programm = clock();
    double elapsed_time = double(end_time_programm - start_time_programm) / CLOCKS_PER_SEC;

    size_t total_pixels = img_w * img_h;
    double throughput = total_pixels / kernel_execution_time; // Píxeles procesados por segundo
    std::cout << "Kernel throughput: " << throughput << " pixels/second\n";


    size_t memory_footprint = sizeof(float) * VECTOR_SIZE     // Buffer de entrada
                            + sizeof(float) * VECTOR_SIZE     // Buffer de salida
                            + 3 * 3 * sizeof(float) * 2;      // Buffers Sobel X y Y

    std::cout << "Memory footprint: " << memory_footprint / (1024.0 * 1024.0) << " MB\n";
    std::cout << "Elapsed time: " << elapsed_time << " seconds\n\n";
    // output_img.display("Sobel filter");

    return 0;
}