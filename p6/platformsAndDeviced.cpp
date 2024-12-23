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

// g++ platformsAndDeviced.cpp -o platformsAndDeviced -lOpenCL -I -lm -lpthread -lX11 -ljpeg


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
    const cl_uint num_devices_ids = 10, num_platforms_ids = 10;    

    cl_int err = clGetPlatformIDs(num_platforms_ids, platforms_ids, n_platforms);
    
    cl_error(err, "Error: Failed to scan for platform IDs");
    if (verbose) printf("Number of available platforms: %d\n\n", *n_platforms);

    for (int i = 0; i < *n_platforms; i++) {
        err = clGetPlatformInfo(platforms_ids[i], CL_PLATFORM_NAME, t_buf, str_buffer, &e_buf);
        cl_error(err, "Error: Failed to get info of the platform\n");
        if (verbose) printf("\t[%d]-Platform Name: %s\n", i, str_buffer);
    }

    for (int i = 0; i < *n_platforms; i++) {
        err = clGetDeviceIDs(platforms_ids[i], CL_DEVICE_TYPE_ALL, num_devices_ids, devices_ids[i], &(n_devices[i]));
        cl_error(err, "Error: Failed to Scan for Devices IDs");
        if (verbose) printf("\t[%d]-Platform. Number of available devices: %d\n", i, n_devices[i]);


        for(int j = 0; j < n_devices[i]; j++){
            err = clGetDeviceInfo(devices_ids[i][j], CL_DEVICE_NAME, sizeof(str_buffer), &str_buffer, NULL);
            cl_error(err, "clGetDeviceInfo: Getting device name");
            if (verbose) printf("\t\t [%d]-Platform [%d]-Device CL_DEVICE_NAME: %s\n", i, j,str_buffer);
            
            cl_device_type device_type;
            err = clGetDeviceInfo(devices_ids[i][j], CL_DEVICE_TYPE, sizeof(device_type), &device_type, NULL);
            cl_error(err, "Error: Failed to get device type");

            if (device_type == CL_DEVICE_TYPE_GPU) {
                printf("\t\t The detected device is a GPU.\n");
            } else if (device_type == CL_DEVICE_TYPE_CPU) {
                printf("\t\t The detected device is a CPU.\n");
            } else {
                printf("\t\t The detected device is of another type.\n");
            }
        }
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

int main(int argc, char** argv) {
    // Initialization
    int err;   
    cl_platform_id platforms_ids[10];
    cl_device_id devices_ids[10][10];
    cl_uint n_platforms, n_devices[10];

    bool verbose = true, watch = false;

    // Scan platforms and devices
    scanPlatformsAndDevices(platforms_ids, devices_ids, &n_platforms, n_devices, verbose);
    cl_context context = createContext(platforms_ids[0], devices_ids[0][0]);
    cl_command_queue command_queue = createCommandQueue(context, devices_ids[0][0]);


    return 0;
}