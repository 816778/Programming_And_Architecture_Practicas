#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <thread>
#include <mutex>
#include <random>
#include <chrono>
#include <numeric>
#include <thread_pool_alpha.hpp>
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
    int start, end;
    int length() const { return end - start; }
};

struct ChunkList {
    std::vector<Chunk> chunks;
    std::mutex mutex;
    unsigned int _iter, elements, num_chunks;

    ChunkList(int num_chunks, int num_elements) {
        _iter = 0;
        elements = num_elements;
        this->num_chunks = num_chunks;
        for (int i = 0; i < num_chunks; ++i) {
            int start = i * num_elements / num_chunks;
            int end = (i + 1) * num_elements / num_chunks;
            chunks.push_back({start, end});
        }
    }

    Chunk getChunk() {
        std::lock_guard<std::mutex> lock(mutex);
        if (_iter >= num_chunks) return {0, 0};
        return chunks[_iter++];
    }
};

enum ParallelOption {
    ONE_DEVICE_0,
    ONE_DEVICE_1,
    TWO_DEVICES,
    TWO_DEVICES_BALANCED,
    USE_CHUNKS
};


/**
 * Auxiliar functions
 */

void writeOutputImages(const std::vector<CImg<unsigned char>>& images, const std::vector<std::string>& files, bool verbose = true) {
    if (!verbose) return;

    for (size_t i = 0; i < images.size(); i++) {
        size_t file_index = i % files.size();  // Índice del archivo original
        std::cout << "Imagen " << i + 1 << ": " << files[file_index] << " (" 
                    << images[i].width() << "x" << images[i].height() << ")\n";
    }
}

void writeOutputChunks(const ChunkList& chunks, bool verbose = true) {
    if (!verbose) return;

    for (size_t i = 0; i < chunks.chunks.size(); i++) {
        const Chunk& chunk = chunks.chunks[i];
        std::cout << "Chunk " << i + 1 << ": from " << chunk.start << " to " << chunk.end << " (" << chunk.length() << " images)\n";
    }
}


/**
 * Image functions
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


std::vector<CImg<unsigned char>> loadImagesFromFilesConcurrentPool(
    const std::vector<std::string>& file_paths, int copy_each_image = 1) {
    std::vector<CImg<unsigned char>> images;
    threadsafe_queue<std::pair<int, std::string>> task_queue;
    std::mutex image_mutex;

    // Crear un pool de hilos con un número de hilos igual a la concurrencia de hardware
    thread_pool pool(std::thread::hardware_concurrency());

    // Llenar la cola de tareas con los archivos de imágenes
    for (int i = 0; i < file_paths.size(); ++i) {
        task_queue.push({i, file_paths[i]});
    }

    // Función que los hilos usarán para procesar las imágenes
    auto worker = [&]() {
        std::pair<int, std::string> task;
        while (task_queue.try_pop(task)) { // Obtener la siguiente tarea
            try {
                // Cargar la imagen
                CImg<unsigned char> img(task.second.c_str());
                CImg<unsigned char> img_gray = img.get_RGBtoYCbCr().get_channel(0);

                // Proteger el acceso a `images` con un mutex
                std::lock_guard<std::mutex> lock(image_mutex);
                for (int i = 0; i < copy_each_image; ++i) {
                    images.push_back(img_gray);
                }
            } catch (const cimg_library::CImgIOException& e) {
                std::cerr << "Error al cargar la imagen: " << task.second
                          << " (" << e.what() << ")\n";
            }
        }
    };

    // Enviar tareas al pool de hilos
    for (size_t i = 0; i < std::thread::hardware_concurrency(); ++i) {
        pool.submit(worker);
    }

    // Esperar a que todos los hilos completen su trabajo
    pool.wait();

    std::vector<size_t> indices(images.size());
    for (size_t i = 0; i < images.size(); ++i) {
        indices[i] = i;
    }
    std::mt19937 rng(42);
    std::shuffle(indices.begin(), indices.end(), rng);
    std::vector<CImg<unsigned char>> shuffled_images;
    for (size_t i : indices) {
        shuffled_images.push_back(images[i]);
    }
    return shuffled_images;
}



size_t calculate_load(const std::vector<CImg<unsigned char>>& images, const unsigned int num_image_types, const std::vector<unsigned short int>& device, unsigned short int this_device) {
    size_t total_load = 0;
    unsigned int i = 0;
    for (auto d : device) {
        if (d == this_device) {
            total_load += images[i % num_image_types].width() * images[i % num_image_types].height();
        }
        i++;
    }
    return total_load;
}

size_t calculate_load(const std::vector<CImg<unsigned char>>& images, const unsigned int num_image_types, unsigned int start, unsigned int end) {
    size_t total_load = 0;
    for (unsigned int i = start; i < end; ++i) {
        total_load += images[i % num_image_types].width() * images[i % num_image_types].height(); // Carga basada en el área de la imagen
    }
    return total_load;
}

size_t count_value(const std::vector<unsigned short int>& device, unsigned short int this_device) {
    size_t count = 0;
    for (const auto& d : device) {
        if (d == this_device) {
            count++;
        }
    }
    return count;
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

void balance_images_bandwidth(const std::vector<CImg<unsigned char>>& images,
                              std::vector<unsigned short int>& device,
                              int num_images, int num_image_types, 
                              double write_bandwidth_gpu0, double read_bandwidth_gpu0,
                              double write_bandwidth_gpu1, double read_bandwidth_gpu1) {
    // images_gpu0.clear();
    // images_gpu1.clear();

    // Inicializar carga acumulada de cada GPU
    double total_time_gpu0 = 0.0;
    double total_time_gpu1 = 0.0;

    // Calcular tiempo estimado para cada imagen y asignarla dinámicamente
    for (size_t i = 0; i < num_images; ++i) {
        auto img = images[i % num_image_types];
        size_t img_size = img.width() * img.height();

        // Estimar tiempos para cada GPU
        double time_gpu0 = img_size / write_bandwidth_gpu0 + img_size / read_bandwidth_gpu0;
        double time_gpu1 = img_size / write_bandwidth_gpu1 + img_size / read_bandwidth_gpu1;

        // Asignar la imagen a la GPU con menor carga acumulada
        if (total_time_gpu0 + time_gpu0 <= total_time_gpu1 + time_gpu1) {
            device.push_back(0);
            total_time_gpu0 += time_gpu0;
        } else {
            device.push_back(1);
            total_time_gpu1 += time_gpu1;
        }
    }

    std::cout << "Tiempo total estimado GPU_0: " << total_time_gpu0 << " ms\n";
    std::cout << "Tiempo total estimado GPU_1: " << total_time_gpu1 << " ms\n";
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

            const char* type_str = 
                (device_type == CL_DEVICE_TYPE_GPU) ? "GPU" :
                (device_type == CL_DEVICE_TYPE_CPU) ? "CPU" : "Other";
            if (verbose) printf("\t\t  Type: %s\n", type_str);

            cl_uint compute_units;
            err = clGetDeviceInfo(devices_ids[i][j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
            cl_error(err, "Error: Failed to get compute units");
            if (verbose) printf("\t\t  Compute Units: %d\n", compute_units);

            cl_ulong global_mem_size;
            err = clGetDeviceInfo(devices_ids[i][j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(global_mem_size), &global_mem_size, NULL);
            cl_error(err, "Error: Failed to get global memory size");
            if (verbose) printf("\t\t  Global Memory: %.2f MB\n", global_mem_size / (1024.0 * 1024.0));
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



void processImagesOnGPU_optimized(cl_context context, cl_device_id device_id, cl_program program, const unsigned int n_image_types, const std::vector<CImg<unsigned char>>& images, const unsigned int first, const unsigned int last, int gpu_id, bool watch) {
    auto start_chrono = std::chrono::high_resolution_clock::now();

    int err;

    cl_kernel kernel = clCreateKernel(program, "sobel_filter", &err);
    cl_error(err, "Failed to create kernel from the program\n");

    double total_write_time = 0.0;
    double total_kernel_time = 0.0;
    double total_read_time = 0.0;

    cl_command_queue write_queue = createCommandQueue(context, device_id);
    cl_command_queue kernel_queue = createCommandQueue(context, device_id);
    cl_command_queue read_queue = createCommandQueue(context, device_id);

    std::vector<double> write_times;
    std::vector<double> kernel_times;
    std::vector<double> read_times;

    double total_data = 0.0;
    double total_write_bandwidth = 0.0;
    double total_read_bandwidth = 0.0;
    std::vector<double> data_size_bytes;
    std::vector<double> write_bandwidth;
    std::vector<double> read_bandwidth;

    for (size_t img_index = first; img_index < last; ++img_index) {
        const auto& img = images[img_index % n_image_types];
        size_t width = img.width();
        size_t height = img.height();
        size_t img_s = img.spectrum();

        data_size_bytes.push_back(width * height * img_s * sizeof(float));
        total_data += data_size_bytes.back();

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

        cl_event write_event, kernel_event, read_event;

        // Write buffer (non-blocking)
        err = clEnqueueWriteBuffer(write_queue, in_device_object, CL_FALSE, 0, sizeof(float) * VECTOR_SIZE, input, 0, NULL, &write_event);
        cl_error(err, "Failed to enqueue a write command to the device memory\n");

        // Set the arguments to our compute kernel
        err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &in_device_object);
        cl_error(err, "Failed to set argument 0\n");
        err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &out_device_object);
        cl_error(err, "Failed to set argument 3\n");
        err = clSetKernelArg(kernel, 2, sizeof(unsigned int), &width);
        cl_error(err, "Failed to set argument 2\n");
        err = clSetKernelArg(kernel, 3, sizeof(unsigned int), &height);
        cl_error(err, "Failed to set argument 3\n");
        
        // Launch Kernel
        size_t local_size[2] = {16, 16};
        size_t global_size[2] = {width, height};
        global_size[0] = ((width + local_size[0] - 1) / local_size[0]) * local_size[0];
        global_size[1] = ((height + local_size[1] - 1) / local_size[1]) * local_size[1];
        err = clEnqueueNDRangeKernel(kernel_queue, kernel, 2, NULL, global_size, local_size, 1, &write_event, &kernel_event);
        cl_error(err, "Failed to launch kernel to the device\n");

        // Read data form device memory back to host memory
        err = clEnqueueReadBuffer(read_queue, out_device_object, CL_FALSE, 0, sizeof(float) * VECTOR_SIZE, output, 1, &kernel_event, &read_event);
        cl_error(err, "Failed to enqueue a read command\n");

        clWaitForEvents(1, &read_event);

        // Measure times 
        cl_ulong write_start, write_end, kernel_start, kernel_end, read_start, read_end;
        clGetEventProfilingInfo(write_event, CL_PROFILING_COMMAND_START, sizeof(write_start), &write_start, NULL);
        clGetEventProfilingInfo(write_event, CL_PROFILING_COMMAND_END, sizeof(write_end), &write_end, NULL);
        write_times.push_back((write_end - write_start) * 1e-6); // Convert to ms

        clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_START, sizeof(kernel_start), &kernel_start, NULL);
        clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_END, sizeof(kernel_end), &kernel_end, NULL);
        kernel_times.push_back((kernel_end - kernel_start) * 1e-6); // Convert to ms

        clGetEventProfilingInfo(read_event, CL_PROFILING_COMMAND_START, sizeof(read_start), &read_start, NULL);
        clGetEventProfilingInfo(read_event, CL_PROFILING_COMMAND_END, sizeof(read_end), &read_end, NULL);
        read_times.push_back((read_end - read_start) * 1e-6); // Convert to ms

        total_write_time += write_times.back();
        total_kernel_time += kernel_times.back();
        total_read_time += read_times.back();

        write_bandwidth.push_back(data_size_bytes.back() / (write_times.back() * 1e-3));
        read_bandwidth.push_back(data_size_bytes.back() / (read_times.back() * 1e-3));
        total_write_bandwidth += write_bandwidth.back();
        total_read_bandwidth += read_bandwidth.back();

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

        /*if (img_index == 0 || img_index == images.size() - 1) {
            std::string filename = "results/images/output_gpu" + std::to_string(gpu_id) + "_img" + std::to_string(img_index) + ".png";
            output_img.save_png(filename.c_str());
            std::cout << "Image saved: " << filename << std::endl;
        }*/

        clReleaseMemObject(in_device_object);
        clReleaseMemObject(out_device_object); 
        free(input);
        free(output);
        if (watch){
            output_img.display("Sobel filter");
        }
    }
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(write_queue);
    clReleaseCommandQueue(kernel_queue);
    clReleaseCommandQueue(read_queue);
    clReleaseContext(context);

    auto end_chrono = std::chrono::high_resolution_clock::now(); // Finaliza el temporizador
    std::chrono::duration<double, std::milli> elapsed = end_chrono - start_chrono; 
    std::cout << "GPU_" << gpu_id << " Total time: " << elapsed.count() << " ms\n";

    std::ofstream output_file("results/times_gpu_" + std::to_string(gpu_id) + ".txt");
    std::ofstream bandwidth_file("results/bandwidth_gpu_" + std::to_string(gpu_id) + ".txt");

    if (output_file.is_open() && bandwidth_file.is_open()) {
        for (size_t i = 0; i < images.size(); ++i) {
            output_file << i << ";" << write_times[i] << ";" << kernel_times[i] << ";" << read_times[i] << "\n";
            bandwidth_file << i << ":" << data_size_bytes[i] << ";" << write_bandwidth[i] << ";" << read_bandwidth[i] << "\n";
        }
        output_file.close();
    } else {
        std::cerr << "Error: Unable to open file for writing.\n";
    }

    double total_write_bandwidth_global = total_data / (total_write_time * 1e-3); // Bytes / Segundos
    double total_read_bandwidth_global = total_data / (total_read_time * 1e-3);  // Bytes / Segundos
    total_write_bandwidth_global /= (1024 * 1024); // MB/s
    total_read_bandwidth_global /= (1024 * 1024); // MB/s

    double avg_write = std::accumulate(write_times.begin(), write_times.end(), 0.0) / images.size();
    double avg_kernel = std::accumulate(kernel_times.begin(), kernel_times.end(), 0.0) / images.size();
    double avg_read = std::accumulate(read_times.begin(), read_times.end(), 0.0) / images.size();

    // Total sized procesed
    std::cout << "GPU_" << gpu_id << " Total data processed: " << total_data / (1024 * 1024) << " MB\n";
    std::cout << "GPU_" << gpu_id << " Average Write Time: " << avg_write << " ms\n";
    std::cout << "GPU_" << gpu_id << " Average Kernel Time: " << avg_kernel << " ms\n";
    std::cout << "GPU_" << gpu_id << " Average Read Time: " << avg_read << " ms\n";

    std::cout << "GPU_" << gpu_id << " Global Write Bandwidth: " << total_write_bandwidth_global << " MB/s\n";
    std::cout << "GPU_" << gpu_id << " Global Read Bandwidth: " << total_read_bandwidth_global << " MB/s\n";

}


void processImagesOnGPU_balanced(cl_context context, cl_device_id device_id, cl_program program, const unsigned int n_image_types, const std::vector<CImg<unsigned char>>& images, const unsigned int first, const unsigned int last, int gpu_id, bool watch, const std::vector <unsigned short int>& device, unsigned short int this_device) {
    auto start_chrono = std::chrono::high_resolution_clock::now();

    int err;

    cl_kernel kernel = clCreateKernel(program, "sobel_filter", &err);
    cl_error(err, "Failed to create kernel from the program\n");

    double total_write_time = 0.0;
    double total_kernel_time = 0.0;
    double total_read_time = 0.0;

    cl_command_queue write_queue = createCommandQueue(context, device_id);
    cl_command_queue kernel_queue = createCommandQueue(context, device_id);
    cl_command_queue read_queue = createCommandQueue(context, device_id);

    std::vector<double> write_times;
    std::vector<double> kernel_times;
    std::vector<double> read_times;

    double total_data = 0.0;
    double total_write_bandwidth = 0.0;
    double total_read_bandwidth = 0.0;
    std::vector<double> data_size_bytes;
    std::vector<double> write_bandwidth;
    std::vector<double> read_bandwidth;

    for (size_t img_index = first; img_index < last; ++img_index) {
        if (device[img_index] != this_device) continue;
        const auto& img = images[img_index % n_image_types];
        size_t width = img.width();
        size_t height = img.height();
        size_t img_s = img.spectrum();

        data_size_bytes.push_back(width * height * img_s * sizeof(float));
        total_data += data_size_bytes.back();

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

        cl_event write_event, kernel_event, read_event;

        // Write buffer (non-blocking)
        err = clEnqueueWriteBuffer(write_queue, in_device_object, CL_FALSE, 0, sizeof(float) * VECTOR_SIZE, input, 0, NULL, &write_event);
        cl_error(err, "Failed to enqueue a write command to the device memory\n");

        // Set the arguments to our compute kernel
        err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &in_device_object);
        cl_error(err, "Failed to set argument 0\n");
        err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &out_device_object);
        cl_error(err, "Failed to set argument 3\n");
        err = clSetKernelArg(kernel, 2, sizeof(unsigned int), &width);
        cl_error(err, "Failed to set argument 2\n");
        err = clSetKernelArg(kernel, 3, sizeof(unsigned int), &height);
        cl_error(err, "Failed to set argument 3\n");
        
        // Launch Kernel
        size_t local_size[2] = {16, 16};
        size_t global_size[2] = {width, height};
        global_size[0] = ((width + local_size[0] - 1) / local_size[0]) * local_size[0];
        global_size[1] = ((height + local_size[1] - 1) / local_size[1]) * local_size[1];
        err = clEnqueueNDRangeKernel(kernel_queue, kernel, 2, NULL, global_size, local_size, 1, &write_event, &kernel_event);
        cl_error(err, "Failed to launch kernel to the device\n");

        // Read data form device memory back to host memory
        err = clEnqueueReadBuffer(read_queue, out_device_object, CL_FALSE, 0, sizeof(float) * VECTOR_SIZE, output, 1, &kernel_event, &read_event);
        cl_error(err, "Failed to enqueue a read command\n");

        clWaitForEvents(1, &read_event);

        // Measure times 
        cl_ulong write_start, write_end, kernel_start, kernel_end, read_start, read_end;
        clGetEventProfilingInfo(write_event, CL_PROFILING_COMMAND_START, sizeof(write_start), &write_start, NULL);
        clGetEventProfilingInfo(write_event, CL_PROFILING_COMMAND_END, sizeof(write_end), &write_end, NULL);
        write_times.push_back((write_end - write_start) * 1e-6); // Convert to ms

        clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_START, sizeof(kernel_start), &kernel_start, NULL);
        clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_END, sizeof(kernel_end), &kernel_end, NULL);
        kernel_times.push_back((kernel_end - kernel_start) * 1e-6); // Convert to ms

        clGetEventProfilingInfo(read_event, CL_PROFILING_COMMAND_START, sizeof(read_start), &read_start, NULL);
        clGetEventProfilingInfo(read_event, CL_PROFILING_COMMAND_END, sizeof(read_end), &read_end, NULL);
        read_times.push_back((read_end - read_start) * 1e-6); // Convert to ms

        total_write_time += write_times.back();
        total_kernel_time += kernel_times.back();
        total_read_time += read_times.back();

        write_bandwidth.push_back(data_size_bytes.back() / (write_times.back() * 1e-3));
        read_bandwidth.push_back(data_size_bytes.back() / (read_times.back() * 1e-3));
        total_write_bandwidth += write_bandwidth.back();
        total_read_bandwidth += read_bandwidth.back();

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

        /*if (img_index == 0 || img_index == images.size() - 1) {
            std::string filename = "results/images/output_gpu" + std::to_string(gpu_id) + "_img" + std::to_string(img_index) + ".png";
            output_img.save_png(filename.c_str());
            std::cout << "Image saved: " << filename << std::endl;
        }*/

        clReleaseMemObject(in_device_object);
        clReleaseMemObject(out_device_object); 
        free(input);
        free(output);
        if (watch){
            output_img.display("Sobel filter");
        }
    }
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(write_queue);
    clReleaseCommandQueue(kernel_queue);
    clReleaseCommandQueue(read_queue);
    clReleaseContext(context);

    auto end_chrono = std::chrono::high_resolution_clock::now(); // Finaliza el temporizador
    std::chrono::duration<double, std::milli> elapsed = end_chrono - start_chrono; 
    std::cout << "GPU_" << gpu_id << " Total time: " << elapsed.count() << " ms\n";

    std::ofstream output_file("results/times_gpu_" + std::to_string(gpu_id) + ".txt");
    std::ofstream bandwidth_file("results/bandwidth_gpu_" + std::to_string(gpu_id) + ".txt");

    if (output_file.is_open() && bandwidth_file.is_open()) {
        for (size_t i = 0; i < images.size(); ++i) {
            output_file << i << ";" << write_times[i] << ";" << kernel_times[i] << ";" << read_times[i] << "\n";
            bandwidth_file << i << ":" << data_size_bytes[i] << ";" << write_bandwidth[i] << ";" << read_bandwidth[i] << "\n";
        }
        output_file.close();
    } else {
        std::cerr << "Error: Unable to open file for writing.\n";
    }

    double total_write_bandwidth_global = total_data / (total_write_time * 1e-3); // Bytes / Segundos
    double total_read_bandwidth_global = total_data / (total_read_time * 1e-3);  // Bytes / Segundos
    total_write_bandwidth_global /= (1024 * 1024); // MB/s
    total_read_bandwidth_global /= (1024 * 1024); // MB/s

    double avg_write = std::accumulate(write_times.begin(), write_times.end(), 0.0) / images.size();
    double avg_kernel = std::accumulate(kernel_times.begin(), kernel_times.end(), 0.0) / images.size();
    double avg_read = std::accumulate(read_times.begin(), read_times.end(), 0.0) / images.size();

    // Total sized procesed
    std::cout << "GPU_" << gpu_id << " Total data processed: " << total_data / (1024 * 1024) << " MB\n";
    std::cout << "GPU_" << gpu_id << " Average Write Time: " << avg_write << " ms\n";
    std::cout << "GPU_" << gpu_id << " Average Kernel Time: " << avg_kernel << " ms\n";
    std::cout << "GPU_" << gpu_id << " Average Read Time: " << avg_read << " ms\n";

    std::cout << "GPU_" << gpu_id << " Global Write Bandwidth: " << total_write_bandwidth_global << " MB/s\n";
    std::cout << "GPU_" << gpu_id << " Global Read Bandwidth: " << total_read_bandwidth_global << " MB/s\n";

}


void manageChunks (ChunkList& chunks, cl_context context, cl_device_id device_id, cl_program program, const unsigned int n_image_types, const std::vector<CImg<unsigned char>>& images, int gpu_id, bool watch, double& load, unsigned int& size) {
    // duration vars set to zero
    auto start_chrono = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = start_chrono - start_chrono, elapsed_threads = start_chrono - start_chrono;
    double total_write_time = 0.0;
    double total_kernel_time = 0.0;
    double total_read_time = 0.0;

    start_chrono = std::chrono::high_resolution_clock::now();

    int err;

    cl_kernel kernel = clCreateKernel(program, "sobel_filter", &err);
    cl_error(err, "Failed to create kernel from the program\n");

    cl_command_queue write_queue = createCommandQueue(context, device_id);
    cl_command_queue kernel_queue = createCommandQueue(context, device_id);
    cl_command_queue read_queue = createCommandQueue(context, device_id);

    elapsed += std::chrono::high_resolution_clock::now() - start_chrono;
    start_chrono = std::chrono::high_resolution_clock::now();

    Chunk currentChunk = chunks.getChunk();
    std::list<Chunk> managed_chunks;

    elapsed_threads += std::chrono::high_resolution_clock::now() - start_chrono;
    start_chrono = std::chrono::high_resolution_clock::now();

    double total_data = 0.0;
    double total_write_bandwidth = 0.0;
    double total_read_bandwidth = 0.0;
    unsigned int managed_images = 0;
    
    std::vector<double> write_times;
    std::vector<double> kernel_times;
    std::vector<double> read_times;

    std::vector<double> data_size_bytes;
    std::vector<double> write_bandwidth;
    std::vector<double> read_bandwidth;

    while (currentChunk.length() > 0) {

        managed_chunks.push_back(currentChunk);
        managed_images += currentChunk.length();

        std::cout << "GPU_" << gpu_id << " manages the chunk: " << currentChunk.start << " - " << currentChunk.end << " (" << currentChunk.length() << " images)" << std::endl;

        for (size_t img_index = currentChunk.start; img_index < currentChunk.end; ++img_index) {
            const auto& img = images[img_index % n_image_types];
            size_t width = img.width();
            size_t height = img.height();
            size_t img_s = img.spectrum();

            data_size_bytes.push_back(width * height * img_s * sizeof(float));
            total_data += data_size_bytes.back();

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

            cl_event write_event, kernel_event, read_event;

            // Write buffer (non-blocking)
            err = clEnqueueWriteBuffer(write_queue, in_device_object, CL_FALSE, 0, sizeof(float) * VECTOR_SIZE, input, 0, NULL, &write_event);
            cl_error(err, "Failed to enqueue a write command to the device memory\n");

            // Set the arguments to our compute kernel
            err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &in_device_object);
            cl_error(err, "Failed to set argument 0\n");
            err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &out_device_object);
            cl_error(err, "Failed to set argument 3\n");
            err = clSetKernelArg(kernel, 2, sizeof(unsigned int), &width);
            cl_error(err, "Failed to set argument 2\n");
            err = clSetKernelArg(kernel, 3, sizeof(unsigned int), &height);
            cl_error(err, "Failed to set argument 3\n");
            
            // Launch Kernel
            size_t local_size[2] = {16, 16};
            size_t global_size[2] = {width, height};
            global_size[0] = ((width + local_size[0] - 1) / local_size[0]) * local_size[0];
            global_size[1] = ((height + local_size[1] - 1) / local_size[1]) * local_size[1];
            err = clEnqueueNDRangeKernel(kernel_queue, kernel, 2, NULL, global_size, local_size, 1, &write_event, &kernel_event);
            cl_error(err, "Failed to launch kernel to the device\n");

            // Read data form device memory back to host memory
            err = clEnqueueReadBuffer(read_queue, out_device_object, CL_FALSE, 0, sizeof(float) * VECTOR_SIZE, output, 1, &kernel_event, &read_event);
            cl_error(err, "Failed to enqueue a read command\n");

            clWaitForEvents(1, &read_event);

            // Measure times 
            cl_ulong write_start, write_end, kernel_start, kernel_end, read_start, read_end;
            clGetEventProfilingInfo(write_event, CL_PROFILING_COMMAND_START, sizeof(write_start), &write_start, NULL);
            clGetEventProfilingInfo(write_event, CL_PROFILING_COMMAND_END, sizeof(write_end), &write_end, NULL);
            write_times.push_back((write_end - write_start) * 1e-6); // Convert to ms

            clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_START, sizeof(kernel_start), &kernel_start, NULL);
            clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_END, sizeof(kernel_end), &kernel_end, NULL);
            kernel_times.push_back((kernel_end - kernel_start) * 1e-6); // Convert to ms

            clGetEventProfilingInfo(read_event, CL_PROFILING_COMMAND_START, sizeof(read_start), &read_start, NULL);
            clGetEventProfilingInfo(read_event, CL_PROFILING_COMMAND_END, sizeof(read_end), &read_end, NULL);
            read_times.push_back((read_end - read_start) * 1e-6); // Convert to ms

            total_write_time += write_times.back();
            total_kernel_time += kernel_times.back();
            total_read_time += read_times.back();

            write_bandwidth.push_back(data_size_bytes.back() / (write_times.back() * 1e-3));
            read_bandwidth.push_back(data_size_bytes.back() / (read_times.back() * 1e-3));
            total_write_bandwidth += write_bandwidth.back();
            total_read_bandwidth += read_bandwidth.back();

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

            /*if (img_index == 0 || img_index == images.size() - 1) {
                std::string filename = "results/images/output_gpu" + std::to_string(gpu_id) + "_img" + std::to_string(img_index) + ".png";
                output_img.save_png(filename.c_str());
                std::cout << "Image saved: " << filename << std::endl;
            }*/

            clReleaseMemObject(in_device_object);
            clReleaseMemObject(out_device_object); 
            free(input);
            free(output);
            if (watch){
                output_img.display("Sobel filter");
            }
        }

        load += calculate_load(images, n_image_types, currentChunk.start, currentChunk.end);

        elapsed += std::chrono::high_resolution_clock::now() - start_chrono;
        start_chrono = std::chrono::high_resolution_clock::now();

        currentChunk = chunks.getChunk();

        elapsed_threads += std::chrono::high_resolution_clock::now() - start_chrono;
        start_chrono = std::chrono::high_resolution_clock::now();

    }
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(write_queue);
    clReleaseCommandQueue(kernel_queue);
    clReleaseCommandQueue(read_queue);
    clReleaseContext(context);

    elapsed += std::chrono::high_resolution_clock::now() - start_chrono;

    std::cout << "GPU_" << gpu_id << " Processing time: " << elapsed.count() << " ms\n";
    std::cout << "GPU_" << gpu_id << " Threads time: " << elapsed_threads.count() << " ms\n";

    std::ofstream output_file("results/times_gpu_" + std::to_string(gpu_id) + ".txt");
    std::ofstream bandwidth_file("results/bandwidth_gpu_" + std::to_string(gpu_id) + ".txt");

    if (output_file.is_open() && bandwidth_file.is_open()) {
        for (Chunk c : managed_chunks) {
            for (size_t i = c.start; i < c.end; ++i) {
                output_file << i << ";" << write_times[i] << ";" << kernel_times[i] << ";" << read_times[i] << "\n";
                bandwidth_file << i << ":" << data_size_bytes[i] << ";" << write_bandwidth[i] << ";" << read_bandwidth[i] << "\n";
            }
        }
        output_file.close();
    } else {
        std::cerr << "Error: Unable to open file for writing.\n";
    }

    double total_write_bandwidth_global = total_data / (total_write_time * 1e-3); // Bytes / Segundos
    double total_read_bandwidth_global = total_data / (total_read_time * 1e-3);  // Bytes / Segundos
    total_write_bandwidth_global /= (1024 * 1024); // MB/s
    total_read_bandwidth_global /= (1024 * 1024); // MB/s

    double avg_write = std::accumulate(write_times.begin(), write_times.end(), 0.0);
    double avg_kernel = std::accumulate(kernel_times.begin(), kernel_times.end(), 0.0);
    double avg_read = std::accumulate(read_times.begin(), read_times.end(), 0.0);

    size = managed_images;

    // Total sized procesed
    std::cout << "GPU_" << gpu_id << " Total data processed: " << total_data / (1024 * 1024) << " MB\n";
    std::cout << "GPU_" << gpu_id << " Average Write Time: " << avg_write << " ms\n";
    std::cout << "GPU_" << gpu_id << " Average Kernel Time: " << avg_kernel << " ms\n";
    std::cout << "GPU_" << gpu_id << " Average Read Time: " << avg_read << " ms\n";

    std::cout << "GPU_" << gpu_id << " Global Write Bandwidth: " << total_write_bandwidth_global << " MB/s\n";
    std::cout << "GPU_" << gpu_id << " Global Read Bandwidth: " << total_read_bandwidth_global << " MB/s\n";
}



int main(int argc, char** argv) {
    // Initialization
    int err;   
    cl_platform_id platforms_ids[10];
    cl_device_id devices_ids[10][10];
    cl_uint n_platforms, n_devices[10];

    std::string folder_path = "./images";
    bool verbose = false, watch = false, loadThreads = false;
    ParallelOption paralel_option = TWO_DEVICES;
    unsigned int num_chunks = 32, n_images = 16384;

    for (int arg = 1; arg < argc; arg++) {
        if ((std::string(argv[arg]) == "-f" || std::string(argv[arg]) == "--folder") && arg + 1 < argc) {
            folder_path = argv[++arg];
        } else if (std::string(argv[arg]) == "-v" || std::string(argv[arg]) == "--verbose") {
            verbose = true;
        } else if (std::string(argv[arg]) == "-w" || std::string(argv[arg]) == "--watch") {
            watch = true;
        } else if (std::string(argv[arg]) == "-t" || std::string(argv[arg]) == "--threads") {
            loadThreads = true;
        } else if ((std::string(argv[arg]) == "-n" || std::string(argv[arg]) == "--num_images") && arg + 1 < argc) {
            n_images = std::stoi(argv[++arg]);
        } else if ((std::string(argv[arg]) == "-d" || std::string(argv[arg]) == "--device") && arg + 1 < argc) {
            std::string option = argv[++arg];
            if (option == "gpu0") paralel_option = ONE_DEVICE_0;
            else if (option == "gpu1") paralel_option = ONE_DEVICE_1;
            else if (option == "two") paralel_option = TWO_DEVICES;
            else if (option == "two_balanced") paralel_option = TWO_DEVICES_BALANCED;
            else if (option == "two_chunks") {
                paralel_option = USE_CHUNKS;
                if (arg + 1 < argc) num_chunks = std::stoi(argv[++arg]);
            }
        }
    }

    std::vector<std::string> files = listFilesInDirectory(folder_path);
    if (files.empty()) {
        std::cerr << "No se encontraron archivos en el directorio " << folder_path << std::endl;
        return -1;
    }

    size_t total_images = 20;
    size_t num_image_types = files.size();
    size_t copy_each_image = total_images / num_image_types;

    std::vector<CImg<unsigned char>> images = loadThreads ? loadImagesFromFilesConcurrentPool(files, copy_each_image) : loadImagesFromFiles(files, copy_each_image);
    writeOutputImages(images, files, verbose);
    
    // Scan platforms and devices
    scanPlatformsAndDevices(platforms_ids, devices_ids, &n_platforms, n_devices, verbose);

    cl_context context_gpu0 = createContext(platforms_ids[0], devices_ids[0][0]);
    cl_context context_gpu1 = createContext(platforms_ids[0], devices_ids[0][1]);

    cl_program program_gpu0 = loadAndBuildProgram(context_gpu0, devices_ids[0][0], "kernel.cl");
    cl_program program_gpu1 = loadAndBuildProgram(context_gpu1, devices_ids[0][1], "kernel.cl");

    auto start_chrono = std::chrono::high_resolution_clock::now();

    if(paralel_option == ONE_DEVICE_0){
        std::thread gpu0_thread(processImagesOnGPU_optimized, context_gpu0, devices_ids[0][0], program_gpu0, num_image_types, std::ref(images), 0, n_images, 0, watch);
        gpu0_thread.join();
    } 
    else if(paralel_option == ONE_DEVICE_1){
        std::thread gpu1_thread(processImagesOnGPU_optimized, context_gpu1, devices_ids[0][1], program_gpu1, num_image_types, std::ref(images), 0, n_images, 1, watch);
        gpu1_thread.join();
    } 
    else if (paralel_option == TWO_DEVICES){
        size_t mid = n_images / 2;

        std::thread gpu0_thread(processImagesOnGPU_optimized, context_gpu0, devices_ids[0][0], program_gpu0, num_image_types, std::ref(images), 0, mid, 10, watch);
        std::thread gpu1_thread(processImagesOnGPU_optimized, context_gpu1, devices_ids[0][1], program_gpu1, num_image_types, std::ref(images), mid, n_images, 11, watch);

        gpu0_thread.join();
        gpu1_thread.join();

        double load_gpu0 = calculate_load(images, num_image_types, 0, mid); // Implementa una función basada en total processing time
        double load_gpu1 = calculate_load(images, num_image_types, mid, n_images);

        std::cout << "GPU 0 Load: " << load_gpu0 << "\n";
        std::cout << "GPU 1 Load: " << load_gpu1 << "\n";

        std::cout << "GPU_0 processes " << mid << " images.\n";
        std::cout << "GPU_1 processes " << n_images - mid << " images.\n";

        double imbalance = std::abs(load_gpu0 - load_gpu1) / std::max(load_gpu0, load_gpu1) * 100.0;
        std::cout << "Workload imbalance: " << imbalance << "%\n";

    } 
    else if(paralel_option == TWO_DEVICES_BALANCED){
        std::vector<unsigned short int> device;
        //balance_images(images, images_gpu0, images_gpu1);
        balance_images_bandwidth(images, device, n_images, num_image_types, 10500, 10100, 14700, 13500);

        std::thread gpu0_thread(processImagesOnGPU_balanced, context_gpu0, devices_ids[0][0], program_gpu0, num_image_types, std::ref(images), 0, n_images, 20, watch, device, 0);
        std::thread gpu1_thread(processImagesOnGPU_balanced, context_gpu1, devices_ids[0][1], program_gpu1, num_image_types, std::ref(images), 0, n_images, 21, watch, device, 1);

        gpu0_thread.join();
        gpu1_thread.join();

        double load_gpu0 = calculate_load(images, num_image_types, device, 0); // Implementa una función basada en total processing time
        double load_gpu1 = calculate_load(images, num_image_types, device, 1);

        std::cout << "GPU 0 Load: " << load_gpu0 << "\n";
        std::cout << "GPU 1 Load: " << load_gpu1 << "\n";

        std::cout << "GPU_0 processes " << count_value(device, 0) << " images.\n";
        std::cout << "GPU_1 processes " << count_value(device, 1) << " images.\n";

        double imbalance = std::abs(load_gpu0 - load_gpu1) / std::max(load_gpu0, load_gpu1) * 100.0;
        std::cout << "Workload imbalance: " << imbalance << "%\n";

    }
    else if (paralel_option == USE_CHUNKS){
        // mutex for the chunks
        ChunkList chunks(num_chunks, n_images);
        writeOutputChunks(chunks, verbose);
        double load_gpu0 = 0.0, load_gpu1 = 0.0;
        unsigned int processes_gpu0 = 0, processes_gpu1 = 0;

        std::thread gpu0_thread(manageChunks, std::ref(chunks), context_gpu0, devices_ids[0][0], program_gpu0, num_image_types, std::ref(images), 30, watch, std::ref(load_gpu0), std::ref(processes_gpu0));
        std::thread gpu1_thread(manageChunks, std::ref(chunks), context_gpu1, devices_ids[0][1], program_gpu1, num_image_types, std::ref(images), 31, watch, std::ref(load_gpu1), std::ref(processes_gpu1));

        gpu0_thread.join();
        gpu1_thread.join();

        std::cout << "GPU 0 Load: " << load_gpu0 << "\n";
        std::cout << "GPU 1 Load: " << load_gpu1 << "\n";

        std::cout << "GPU_0 processes " << processes_gpu0 << " images.\n";
        std::cout << "GPU_1 processes " << processes_gpu1 << " images.\n";

        double imbalance = std::abs(load_gpu0 - load_gpu1) / std::max(load_gpu0, load_gpu1) * 100.0;
        std::cout << "Workload imbalance: " << imbalance << "%\n";

    } 

    auto end_chrono = std::chrono::high_resolution_clock::now(); // Finaliza el temporizador
    std::chrono::duration<double, std::milli> elapsed = end_chrono - start_chrono;
    std::cout << "Total processing time: " << elapsed.count() << " ms" << std::endl;

    return 0;
}