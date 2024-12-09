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
////////////////////////////////////////////////////////////////////////////////


/* ATTENTION: While prgramming in OpenCL it is a good idea to keep the reference manuals handy:
 * https://bashbaug.github.io/OpenCL-Docs/pdf/OpenCL_API.pdf
 * https://www.khronos.org/files/opencl-1-2-quick-reference-card.pdf (summary of OpenCL API)
 * https://www.khronos.org/assets/uploads/developers/presentations/opencl20-quick-reference-card.pdf
 */


int main(int argc, char** argv)
{
  int err;                              // error code returned from api calls
  size_t t_buf = 50;            // size of str_buffer
  char str_buffer[t_buf];       // auxiliary buffer 
  size_t e_buf;             // effective size of str_buffer in use
        
  size_t global_size;                       // global domain size for our calculation
  size_t local_size;                        // local domain size for our calculation

  const cl_uint num_platforms_ids = 10;             // max of allocatable platforms
  cl_platform_id platforms_ids[num_platforms_ids];      // array of platforms
  cl_uint n_platforms;                      // effective number of platforms in use
  const cl_uint num_devices_ids = 10;               // max of allocatable devices
  cl_device_id devices_ids[num_platforms_ids][num_devices_ids]; // array of devices
  cl_uint n_devices[num_platforms_ids];             // effective number of devices in use for each platform
    
  cl_device_id device_id;                           // compute device id 
  cl_context context;                               // compute context
  cl_command_queue command_queue;                   // compute command queue


  // If first parameter is verbose, then it will print the information of the platforms and devices.
  int verbose = 0;
  std::string img_file = "images/image.jpg";

  for (int i = 0; i < argc; i++){
    if (strcmp(argv[i], "verbose") == 0) verbose = 1;
    if (strcmp(argv[i], "-f") == 0) img_file = argv[++i];
  }
    

  // 1. Scan the available platforms:
  err = clGetPlatformIDs (num_platforms_ids, platforms_ids, &n_platforms);
  cl_error(err, "Error: Failed to Scan for Platforms IDs");
  if (verbose) printf("Number of available platforms: %d\n\n", n_platforms);

  for (int i = 0; i < n_platforms; i++ ){
    err = clGetPlatformInfo(platforms_ids[i], CL_PLATFORM_NAME, t_buf, str_buffer, &e_buf);
    cl_error (err, "Error: Failed to get info of the platform\n");
    if (verbose) printf( "\t[%d]-Platform Name: %s\n", i, str_buffer);

    // ***Task***: print on the screen the name, host_timer_resolution, vendor, versionm, ...

    err = clGetPlatformInfo(platforms_ids[i], CL_PLATFORM_HOST_TIMER_RESOLUTION, t_buf, str_buffer, &e_buf);
    cl_error (err, "Error: Failed to get info of the platform\n");
    if (verbose) printf( "\t[%d]-Platform Host Timer Resolution: %s\n", i, str_buffer);

    err = clGetPlatformInfo(platforms_ids[i], CL_PLATFORM_VENDOR, t_buf, str_buffer, &e_buf);
    cl_error (err, "Error: Failed to get info of the platform\n");
    if (verbose) printf( "\t[%d]-Platform Vendor: %s\n", i, str_buffer);

    err = clGetPlatformInfo(platforms_ids[i], CL_PLATFORM_VERSION, t_buf, str_buffer, &e_buf);
    cl_error (err, "Error: Failed to get info of the platform\n");
    if (verbose) printf( "\t[%d]-Platform Version: %s\n", i, str_buffer);

  }
  if (verbose) printf("\n");


    
  // 2. Scan for devices in each platform
  for (int i = 0; i < n_platforms; i++ ){
    err = clGetDeviceIDs(platforms_ids[i], CL_DEVICE_TYPE_ALL, num_devices_ids, devices_ids[i], &(n_devices[i]));
    cl_error(err, "Error: Failed to Scan for Devices IDs");
    if (verbose) printf("\t[%d]-Platform. Number of available devices: %d\n", i, n_devices[i]);

    for(int j = 0; j < n_devices[i]; j++){
      err = clGetDeviceInfo(devices_ids[i][j], CL_DEVICE_NAME, sizeof(str_buffer), &str_buffer, NULL);
      cl_error(err, "clGetDeviceInfo: Getting device name");
      if (verbose) printf("\t\t [%d]-Platform [%d]-Device CL_DEVICE_NAME: %s\n", i, j,str_buffer);

      cl_uint max_compute_units_available;
      cl_ulong cache_size, global_mem_size, local_mem_size, max_work_group_size, profiling_timer_resolution;
      err = clGetDeviceInfo(devices_ids[i][j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(max_compute_units_available), &max_compute_units_available, NULL);
      cl_error(err, "clGetDeviceInfo: Getting device max compute units available");
      if (verbose) printf("\t\t [%d]-Platform [%d]-Device CL_DEVICE_MAX_COMPUTE_UNITS: %d\n", i, j, max_compute_units_available);

      // ***Task***: print on the screen the cache size, global mem size, local memsize, max work group size, profiling timer resolution and ... of each device

      err = clGetDeviceInfo(devices_ids[i][j], CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(cache_size), &cache_size, NULL);
      cl_error(err, "clGetDeviceInfo: Getting device cache size");
      if (verbose) printf("\t\t [%d]-Platform [%d]-Device CL_DEVICE_GLOBAL_MEM_CACHE_SIZE: %lu\n", i, j, cache_size);

      err = clGetDeviceInfo(devices_ids[i][j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(global_mem_size), &global_mem_size, NULL);
      cl_error(err, "clGetDeviceInfo: Getting device global mem size");
      if (verbose) printf("\t\t [%d]-Platform [%d]-Device CL_DEVICE_GLOBAL_MEM_SIZE: %lu\n", i, j, global_mem_size);

      err = clGetDeviceInfo(devices_ids[i][j], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(local_mem_size), &local_mem_size, NULL);
      cl_error(err, "clGetDeviceInfo: Getting device local mem size");
      if (verbose) printf("\t\t [%d]-Platform [%d]-Device CL_DEVICE_LOCAL_MEM_SIZE: %lu\n", i, j, local_mem_size);

      err = clGetDeviceInfo(devices_ids[i][j], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(max_work_group_size), &max_work_group_size, NULL);
      cl_error(err, "clGetDeviceInfo: Getting device max work group size");
      if (verbose) printf("\t\t [%d]-Platform [%d]-Device CL_DEVICE_MAX_WORK_GROUP_SIZE: %lu\n", i, j, max_work_group_size);

      err = clGetDeviceInfo(devices_ids[i][j], CL_DEVICE_PROFILING_TIMER_RESOLUTION, sizeof(profiling_timer_resolution), &profiling_timer_resolution, NULL);
      cl_error(err, "clGetDeviceInfo: Getting device profiling timer resolution");
      if (verbose) printf("\t\t [%d]-Platform [%d]-Device CL_DEVICE_PROFILING_TIMER_RESOLUTION: %lu\n\n", i, j, profiling_timer_resolution);

    }
  } 


  // 3. Create a context, with a device
  cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platforms_ids[0], 0};
  context = clCreateContext(properties, 1, &devices_ids[0][0], NULL, NULL, &err);
  cl_error(err, "Failed to create a compute context\n");

  // 4. Create a command queue
  cl_command_queue_properties proprt[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
  command_queue = clCreateCommandQueueWithProperties(context, devices_ids[0][0]
    , proprt, &err);
  cl_error(err, "Failed to create a command queue\n");

  /* It is still missing the runtime part of the OpenCL program: createBuffers, createProgram, createKernel, setKernelArg, ... */

  // Calculate size of the file
  FILE *fileHandler = fopen("kernel.cl", "r");
  fseek(fileHandler, 0, SEEK_END);
  size_t fileSize = ftell(fileHandler);
  rewind(fileHandler);

  // read kernel source into buffer
  char * sourceCode = (char*) malloc(fileSize + 1);
  sourceCode[fileSize] = '\0';
  fread(sourceCode, sizeof(char), fileSize, fileHandler);
  fclose(fileHandler);

  // create program from buffer
  cl_program program = clCreateProgramWithSource(context, 1, (const char**)&sourceCode, &fileSize, &err);
  cl_error(err, "Failed to create program with source\n");
  free(sourceCode);

    // Build the executable and check errors
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS){
    size_t len;
    char buffer[2048];

    printf("Error: Some error at building process.\n");
    clGetProgramBuildInfo(program, devices_ids[0][0], CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
    printf("%s\n", buffer);
    exit(-1);
  }

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
  img_gray.display("Original Image");


  // Create and initialize the input and output arrays at the host memory. Taking
  // into account the kernel definition, the data type of these arrays should be floating point.
  size_t VECTOR_SIZE = img_w * img_h * img_s;
  float *input = (float*) malloc(sizeof(float) * VECTOR_SIZE);
  float *output = (float*) malloc(sizeof(float) * VECTOR_SIZE);


  unsigned iter = 0;
  for (int channel = 0; channel < img_s; channel++) {
    for (int j = 0; j < img_h; j++){
      for (int i = 0; i < img_w; i++){
        input[iter++] = (float)img(i, j, 0, channel);
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
    {-1, -2, -1},
    {0, 0, 0},
    {1, 2, 1}
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

  // Copiar los datos del host al dispositivo
  err = clEnqueueWriteBuffer(command_queue, in_device_object, CL_TRUE, 0, sizeof(float) * VECTOR_SIZE, input, 0, NULL, NULL);
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
  local_size = 128;
  global_size = VECTOR_SIZE;
  err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
  cl_error(err, "Failed to launch kernel to the device\n");

  // Read data form device memory back to host memory
  err = clEnqueueReadBuffer(command_queue, out_device_object, CL_TRUE, 0, VECTOR_SIZE * sizeof(float), output, 0, NULL, NULL);
  cl_error(err, "Failed to enqueue a read command\n");

  // Volcar los valores de salida es una imagen cimg
  CImg<unsigned char> output_img(img_w, img_h, 1, img_s);
  iter = 0;
  for (int channel = 0; channel < img_s; channel++) {
    for (int j = 0; j < img_h; j++){
      for (int i = 0; i < img_w; i++){
        output_img(i, j, 0, channel) = (unsigned char)output[iter++];
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

  output_img.display("Sobel filter");

  return 0;
}

