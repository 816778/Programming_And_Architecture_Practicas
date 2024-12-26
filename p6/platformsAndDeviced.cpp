#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#ifdef __APPLE__
  #include <OpenCL/opencl.h>
#else
  #include <CL/cl.h>
#endif

#define cimg_use_jpeg
#include "CImg.h"

using namespace std;
using namespace cimg_library;

// ABOUT ERRORS
// Remmember to check error codes from every OpenCL API call
// Info about error codes can be found at the reference manual of OpenCL
// At the following url, you can find name for each error code
//  https://gist.github.com/bmount/4a7144ce801e5569a0b6
//  https://streamhpc.com/blog/2013-04-28/opencl-error-codes/
// Following function checks errors, and in such a case, it prints the code, the string and it exits

// g++ platformsAndDeviced.cpp -o platformsAndDeviced -lOpenCL -I -lm -lpthread -lX11 -ljpeg


struct Chunk {
    int start_x;
    int start_y;
    int width;
    int height;
    CImg<unsigned char>* image;
};

enum WriteOption {
    WRITE_IMAGES,
    WRITE_CHUNKS
};

enum ParallelOption {
    ONE_DEVICE,
    SIMPLE,
    SIMPLE_BALANCED,
    USE_CHUNKS
};


/**
 * Auxiliar functions
 */

void writeOutput(const std::vector<CImg<unsigned char>>& images, const std::vector<std::string>& files, const std::vector<Chunk>& chunks, WriteOption option, bool verbose = true) {
    if (!verbose){
        return;
    }

    if (option == WRITE_IMAGES) {
        for (size_t i = 0; i < images.size(); i++) {
            size_t file_index = i % files.size();  // Índice del archivo original
            std::cout << "Imagen " << i + 1 << ": " << files[file_index] << " (" 
                      << images[i].width() << "x" << images[i].height() << ")\n";
        }
    } else if (option == WRITE_CHUNKS) {
        for (size_t i = 0; i < chunks.size(); ++i) {
            const Chunk& chunk = chunks[i];
            cout << "Chunk " << i + 1 << ": Imagen " << chunk.image
                 << ", Posición (" << chunk.start_x << ", " << chunk.start_y << ")"
                 << ", Tamaño (" << chunk.width << "x" << chunk.height << ")\n";
        }
    }
}

/**
 * Chunck functions
 */
std::vector<Chunk> createChunksForImage(CImg<unsigned char>& image, int max_chunk_height) {
    std::vector<Chunk> chunks;
    int img_width = image.width();
    int img_height = image.height();

    for (int start_y = 0; start_y < img_height; start_y += max_chunk_height) {
        int chunk_height = std::min(max_chunk_height, img_height - start_y);
        chunks.push_back({0, start_y, img_width, chunk_height, &image});
    }

    return chunks;
}


std::vector<Chunk> createChunksForAllImages(std::vector<CImg<unsigned char>>& images, int max_chunk_height) {
    std::vector<Chunk> all_chunks;

    for (auto& image : images) {
        std::vector<Chunk> chunks = createChunksForImage(image, max_chunk_height);
        all_chunks.insert(all_chunks.end(), chunks.begin(), chunks.end());
    }

    return all_chunks;
}


/**
 * Image funtions
 */
std::mutex image_mutex;


std::vector<std::string> listFilesInDirectory(const std::string& folder_path) {
    std::vector<std::string> files;
    DIR* dir = opendir(folder_path.c_str());
    if (!dir) {
        std::cerr << "Error: No se pudo abrir el directorio " << folder_path << std::endl;
        return files;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string file_name = entry->d_name;

        // Ignorar entradas especiales "." y ".."
        if (file_name != "." && file_name != "..") {
            files.push_back(folder_path + "/" + file_name);  // Ruta completa
        }
    }
    closedir(dir);
    return files;
}

// Función para cargar imágenes usando CImg
std::vector<CImg<unsigned char>> loadImagesFromFiles(const std::vector<std::string>& file_paths, int copys_each_image = 1) {
    std::vector<CImg<unsigned char>> images;
    for (const auto& file : file_paths) {
        try {
            CImg<unsigned char> img(file.c_str());
            CImg <unsigned char> img_gray = img.get_RGBtoYCbCr().get_channel(0);
            // Replicar la imagen
            for (int i = 0; i < copys_each_image; i++){
                images.push_back(img_gray);
            }
            std::cout << "Imagen cargada: " << file << "\n";
        } catch (const cimg_library::CImgIOException& e) {
            std::cerr << "Error al cargar la imagen: " << file << " (" << e.what() << ")\n";
        }
    }
    return images;
}


void loadImageSubset(const vector<string>& file_paths, vector<CImg<unsigned char>>& images, int start, int end, int copys_each_image = 1) {
    for (int i = start; i < end; ++i) {
        try {
            CImg<unsigned char> img(file_paths[i].c_str());
            CImg <unsigned char> img_gray = img.get_RGBtoYCbCr().get_channel(0);
            
            {
                for (int i = 0; i < copys_each_image; i++){
                    std::lock_guard<std::mutex> lock(image_mutex);
                    images.push_back(img_gray);
                }
            }
            
            // cout << "Imagen cargada: " << file_paths[i] << endl;
        } catch (const cimg_library::CImgIOException& e) {
            cerr << "Error al cargar la imagen: " << file_paths[i] << " (" << e.what() << ")" << endl;
        }
    }
}

std::vector <CImg<unsigned char>> loadImagesFromFilesConcurrent(const std::vector<std::string>& file_paths, int copy_each_image = 1) {
    std::vector <CImg<unsigned char>> images;
    std::vector <std::thread> threads;
    int num_threads = std::thread::hardware_concurrency();
    cout << "Num threads: " << num_threads << endl;

    int images_per_thread = file_paths.size() / num_threads;
    int remaining_files = file_paths.size() % num_threads;

    for (int i = 0; i < num_threads; ++i) {
        int start = i * images_per_thread;
        int end = (i == num_threads - 1) ? (start + images_per_thread + remaining_files) : (start + images_per_thread);
        //cout << "Thread " << i << " will load images from " << start << " to " << end << endl;
        threads.emplace_back(loadImageSubset, std::ref(file_paths), std::ref(images), start, end, copy_each_image);
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    return images;
}


size_t calculate_load(const std::vector<CImg<unsigned char>>& images) {
    size_t total_load = 0;
    for (const auto& img : images) {
        total_load += img.width() * img.height(); // Carga basada en el área de la imagen
    }
    return total_load;
}

// Función para dividir imágenes equilibradamente entre dos GPUs
void balance_images(const std::vector<CImg<unsigned char>>& images,
                    std::vector<CImg<unsigned char>>& images_gpu0,
                    std::vector<CImg<unsigned char>>& images_gpu1) {
    images_gpu0.clear();
    images_gpu1.clear();

    size_t load_gpu0 = 0, load_gpu1 = 0;

    for (const auto& img : images) {
        size_t img_load = img.width() * img.height();

        // Asignar la imagen a la GPU con menor carga acumulada
        if (load_gpu0 <= load_gpu1) {
            images_gpu0.push_back(img);
            load_gpu0 += img_load;
        } else {
            images_gpu1.push_back(img);
            load_gpu1 += img_load;
        }
    }
}




/**
 * OpenCL functions
 */
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

void processImagesOnGPU(cl_command_queue command_queue, cl_context context, cl_device_id device_id, cl_program program, const std::vector<CImg<unsigned char>>& images, int gpu_id) {
    auto start_chrono = std::chrono::high_resolution_clock::now();

    int err;

    cl_kernel kernel = clCreateKernel(program, "sobel_filter", &err);
    cl_error(err, "Failed to create kernel from the program\n");

    bool first_image_saved = false;
    size_t img_index = 0;

    for (size_t img_index = 0; img_index < images.size(); ++img_index) {
        const auto& img = images[img_index];
        size_t width = img.width();
        size_t height = img.height();
        size_t img_s = img.spectrum();

        size_t VECTOR_SIZE = width * height * img_s;
        float *input = (float*) malloc(sizeof(float) * VECTOR_SIZE);
        float *output = (float*) malloc(sizeof(float) * VECTOR_SIZE);

        unsigned iter = 0;
        for (int channel = 0; channel < img_s; channel++) {
            for (int j = 0; j < height; j++){
                for (int i = 0; i < width; i++){
                    input[iter++] = (float)img(i, j, 0, channel);
                }
            }
        }
        cl_mem in_device_object = clCreateBuffer(context, CL_MEM_READ_ONLY, VECTOR_SIZE * sizeof(float), NULL, &err);
        cl_error(err, "Failed to create memory buffer at device\n");
        cl_mem out_device_object = clCreateBuffer(context, CL_MEM_WRITE_ONLY, VECTOR_SIZE * sizeof(float), NULL, &err);
        cl_error(err, "Failed to create memory buffer at device\n");

        err = clEnqueueWriteBuffer(command_queue, in_device_object, CL_TRUE, 0, sizeof(float) * VECTOR_SIZE, input, 0, NULL, NULL);
        cl_ulong write_start, write_end;
        cl_error(err, "Failed to enqueue a write command to the device memory\n");
    
        // Set the arguments to our compute kernel
        err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &in_device_object);
        cl_error(err, "Failed to set argument 0\n");
        err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &out_device_object);
        cl_error(err, "Failed to set argument 3\n");

        size_t local_size[2] = {16, 16};
        size_t local_mem_size = (local_size[0] + 2) * (local_size[1] + 2) * sizeof(float);
        
        // err = clSetKernelArg(kernel, 4, local_width * local_height * sizeof(float), NULL);
        // cl_error(err, "Failed to set local memory argument");
        err = clSetKernelArg(kernel, 2, sizeof(unsigned int), &width);
        cl_error(err, "Failed to set argument 2\n");
        err = clSetKernelArg(kernel, 3, sizeof(unsigned int), &height);
        cl_error(err, "Failed to set argument 3\n");
        
        // Launch Kernel
        size_t global_size[2] = {width, height}; // 2D global work size
        global_size[0] = ((width + local_size[0] - 1) / local_size[0]) * local_size[0];
        global_size[1] = ((height + local_size[1] - 1) / local_size[1]) * local_size[1];
        printf("Global size: %d %d\n", global_size[0], global_size[1]);
        err = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global_size, local_size, 0, NULL, NULL);
        cl_error(err, "Failed to launch kernel to the device\n");
        // Read data form device memory back to host memory
        err = clEnqueueReadBuffer(command_queue, out_device_object, CL_TRUE, 0, VECTOR_SIZE * sizeof(float), output, 0, NULL, NULL);
        cl_error(err, "Failed to enqueue a read command\n");

        // Create a new image with the output data
        CImg<unsigned char> output_img(width, height);
        iter = 0;
        for (int channel = 0; channel < img_s; channel++) {
            for (int j = 0; j < height; j++){
                for (int i = 0; i < width; i++){
                    output_img(i, j) = (unsigned char)output[iter++];
                }
            }
        }

        if (!first_image_saved) {
            std::string filename = "results/images/output_gpu" + std::to_string(gpu_id) + "_img" + std::to_string(img_index) + ".png";
            output_img.save_png(filename.c_str());
            std::cout << "Image saved: " << filename << std::endl;
            first_image_saved = true; // Marcar como guardada
        }

        clReleaseMemObject(in_device_object);
        clReleaseMemObject(out_device_object);
        clReleaseProgram(program);
        output_img.display("Sobel filter");
    }
    clReleaseKernel(kernel);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);

    auto end_chrono = std::chrono::high_resolution_clock::now(); // Finaliza el temporizador
    std::chrono::duration<double, std::milli> elapsed = end_chrono - start_chrono; 

    std::cout << "Total processing time on GPU: " << elapsed.count() << " ms" << std::endl;
}   

int main(int argc, char** argv) {
    // Initialization
    int err;   
    cl_platform_id platforms_ids[10];
    cl_device_id devices_ids[10][10];
    cl_uint n_platforms, n_devices[10];

    std::string folder_path = "./images";

    bool verbose = true, watch = false, loadThreads = true;

    std::vector<std::string> files = listFilesInDirectory(folder_path);
    if (files.empty()) {
        std::cerr << "No se encontraron archivos en el directorio " << folder_path << std::endl;
        return -1;
    }

    size_t total_images = 15;
    size_t num_images = files.size();
    size_t copy_each_image = total_images / num_images;

    std::vector<CImg<unsigned char>> images = loadThreads ? loadImagesFromFilesConcurrent(files, copy_each_image) : loadImagesFromFiles(files, copy_each_image);
    writeOutput(images, files, {}, WRITE_IMAGES, false);


    ParallelOption paralel_option = SIMPLE;
    
    // Scan platforms and devices
    scanPlatformsAndDevices(platforms_ids, devices_ids, &n_platforms, n_devices, verbose);

    cl_context context_gpu0 = createContext(platforms_ids[0], devices_ids[0][0]);
    cl_context context_gpu1 = createContext(platforms_ids[0], devices_ids[0][1]);

    cl_command_queue queue_gpu0 = createCommandQueue(context_gpu0, devices_ids[0][0]);
    cl_command_queue queue_gpu1 = createCommandQueue(context_gpu1, devices_ids[0][1]);

    cl_program program_gpu0 = loadAndBuildProgram(context_gpu0, devices_ids[0][0], "kernel.cl");
    cl_program program_gpu1 = loadAndBuildProgram(context_gpu1, devices_ids[0][1], "kernel.cl");

    auto start_chrono = std::chrono::high_resolution_clock::now();

    if (paralel_option == SIMPLE){
        size_t mid = images.size() / 2;
        std::vector<CImg<unsigned char>> images_gpu0(images.begin(), images.begin() + mid);
        std::vector<CImg<unsigned char>> images_gpu1(images.begin() + mid, images.end());

        std::thread gpu0_thread(processImagesOnGPU, queue_gpu0, context_gpu0, devices_ids[0][0], program_gpu0, std::ref(images_gpu0), 0);
        std::thread gpu1_thread(processImagesOnGPU, queue_gpu1, context_gpu1, devices_ids[0][1], program_gpu1, std::ref(images_gpu1), 1);

        std::cout << "GPU 0 Load: " << calculate_load(images_gpu0) << std::endl;
        std::cout << "GPU 1 Load: " << calculate_load(images_gpu1) << std::endl;

        gpu0_thread.join();
        gpu1_thread.join();

    } 
    else if (paralel_option == USE_CHUNKS){
        const int max_chunk_height = 128;
        std::vector<Chunk> chunks = createChunksForAllImages(images, max_chunk_height);
        writeOutput(images, files, chunks, WRITE_CHUNKS, false);
    } 
    else if(paralel_option == ONE_DEVICE){
        std::thread gpu0_thread(processImagesOnGPU, queue_gpu0, context_gpu0, devices_ids[0][0], program_gpu0, std::ref(images), 10);
        gpu0_thread.join();
    }
    else if(paralel_option == SIMPLE_BALANCED){
        std::vector<CImg<unsigned char>> images_gpu0, images_gpu1;
        balance_images(images, images_gpu0, images_gpu1);

        std::thread gpu0_thread(processImagesOnGPU, queue_gpu0, context_gpu0, devices_ids[0][0], program_gpu0, std::ref(images_gpu0), 20);
        std::thread gpu1_thread(processImagesOnGPU, queue_gpu1, context_gpu1, devices_ids[0][1], program_gpu1, std::ref(images_gpu1), 21);

        std::cout << "GPU 0 Load: " << calculate_load(images_gpu0) << std::endl;
        std::cout << "GPU 1 Load: " << calculate_load(images_gpu1) << std::endl;

        gpu0_thread.join();
        gpu1_thread.join();
    }
    auto end_chrono = std::chrono::high_resolution_clock::now(); // Finaliza el temporizador
    std::chrono::duration<double, std::milli> elapsed = end_chrono - start_chrono;
    std::cout << "Total processing time: " << elapsed.count() << " ms" << std::endl;

    return 0;
}