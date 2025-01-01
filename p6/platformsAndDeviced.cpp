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
#include <string>
#include <thread>
#include <mutex>
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

void balance_images_bandwidth(const std::vector<CImg<unsigned char>>& images,
                    std::vector<CImg<unsigned char>>& images_gpu0,
                    std::vector<CImg<unsigned char>>& images_gpu1,
                    double bandwidth_gpu0, double bandwidth_gpu1) {
    images_gpu0.clear();
    images_gpu1.clear();

    // Calcular el peso de cada GPU basado en su ancho de banda
    double total_bandwidth = bandwidth_gpu0 + bandwidth_gpu1;
    double weight_gpu0 = bandwidth_gpu0 / total_bandwidth;
    double weight_gpu1 = bandwidth_gpu1 / total_bandwidth;

    size_t total_load = 0;
    for (const auto& img : images) {
        total_load += img.width() * img.height();
    }

    // Calcula la carga objetivo para cada GPU
    size_t target_load_gpu0 = static_cast<size_t>(total_load * weight_gpu0);
    size_t target_load_gpu1 = static_cast<size_t>(total_load * weight_gpu1);

    size_t load_gpu0 = 0, load_gpu1 = 0;

    for (const auto& img : images) {
        size_t img_load = img.width() * img.height();

        // Asignar la imagen a la GPU con menor desbalance respecto al objetivo
        if ((load_gpu0 + img_load <= target_load_gpu0) || (load_gpu1 + img_load > target_load_gpu1)) {
            images_gpu0.push_back(img);
            load_gpu0 += img_load;
        } else {
            images_gpu1.push_back(img);
            load_gpu1 += img_load;
        }
    }

    std::cout << "GPU_0 target load: " << target_load_gpu0 << ", assigned load: " << load_gpu0 << "\n";
    std::cout << "GPU_1 target load: " << target_load_gpu1 << ", assigned load: " << load_gpu1 << "\n";
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



void processImagesOnGPU_optimized_times(cl_context context, cl_device_id device_id, cl_program program, const std::vector<CImg<unsigned char>>& images, int gpu_id) {
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

    std::vector<double> write_times(images.size(), 0.0);
    std::vector<double> kernel_times(images.size(), 0.0);
    std::vector<double> read_times(images.size(), 0.0);

    double total_data = 0.0;
    double total_write_bandwidth = 0.0;
    double total_read_bandwidth = 0.0;
    std::vector<double> data_size_bytes(images.size(), 0.0);
    std::vector<double> write_bandwidth(images.size(), 0.0);
    std::vector<double> read_bandwidth(images.size(), 0.0);

    for (size_t img_index = 0; img_index < images.size(); ++img_index) {
        const auto& img = images[img_index];
        size_t width = img.width();
        size_t height = img.height();
        size_t img_s = img.spectrum();

        data_size_bytes[img_index] = width * height * img_s * sizeof(float);
        total_data += data_size_bytes[img_index];

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
        write_times[img_index] = (write_end - write_start) * 1e-6; // Convert to ms

        clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_START, sizeof(kernel_start), &kernel_start, NULL);
        clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_END, sizeof(kernel_end), &kernel_end, NULL);
        kernel_times[img_index] = (kernel_end - kernel_start) * 1e-6; // Convert to ms

        clGetEventProfilingInfo(read_event, CL_PROFILING_COMMAND_START, sizeof(read_start), &read_start, NULL);
        clGetEventProfilingInfo(read_event, CL_PROFILING_COMMAND_END, sizeof(read_end), &read_end, NULL);
        read_times[img_index] = (read_end - read_start) * 1e-6; // Convert to ms

        total_write_time += write_times[img_index];
        total_kernel_time += kernel_times[img_index];
        total_read_time += read_times[img_index];

        write_bandwidth[img_index] = data_size_bytes[img_index] / (write_times[img_index] * 1e-3);
        read_bandwidth[img_index] = data_size_bytes[img_index] / ( read_times[img_index] * 1e-3);
        total_write_bandwidth += write_bandwidth[img_index];
        total_read_bandwidth += read_bandwidth[img_index];

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
        //output_img.display("Sobel filter");
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

    size_t total_images = 5000;
    size_t num_images = files.size();
    size_t copy_each_image = total_images / num_images;

    std::vector<CImg<unsigned char>> images = loadThreads ? loadImagesFromFilesConcurrentPool(files, copy_each_image) : loadImagesFromFiles(files, copy_each_image);
    writeOutput(images, files, {}, WRITE_IMAGES, false);


    ParallelOption paralel_option = SIMPLE_BALANCED;
    
    // Scan platforms and devices
    scanPlatformsAndDevices(platforms_ids, devices_ids, &n_platforms, n_devices, verbose);

    cl_context context_gpu0 = createContext(platforms_ids[0], devices_ids[0][0]);
    cl_context context_gpu1 = createContext(platforms_ids[0], devices_ids[0][1]);

    cl_program program_gpu0 = loadAndBuildProgram(context_gpu0, devices_ids[0][0], "kernel.cl");
    cl_program program_gpu1 = loadAndBuildProgram(context_gpu1, devices_ids[0][1], "kernel.cl");

    auto start_chrono = std::chrono::high_resolution_clock::now();

    if (paralel_option == SIMPLE){
        size_t mid = images.size() / 2;
        std::vector<CImg<unsigned char>> images_gpu0(images.begin(), images.begin() + mid);
        std::vector<CImg<unsigned char>> images_gpu1(images.begin() + mid, images.end());

        std::thread gpu0_thread(processImagesOnGPU_optimized_times, context_gpu0, devices_ids[0][0], program_gpu0, std::ref(images_gpu0), 0);
        std::thread gpu1_thread(processImagesOnGPU_optimized_times, context_gpu1, devices_ids[0][1], program_gpu1, std::ref(images_gpu1), 1);

        gpu0_thread.join();
        gpu1_thread.join();

        double load_gpu0 = calculate_load(images_gpu0); // Implementa una función basada en total processing time
        double load_gpu1 = calculate_load(images_gpu1);

        std::cout << "GPU 0 Load: " << load_gpu0 << std::endl;
        std::cout << "GPU 1 Load: " << load_gpu1 << std::endl;

        std::cout << "GPU_0 processes " << images_gpu0.size() << " images.\n";
        std::cout << "GPU_1 processes " << images_gpu1.size() << " images.\n";

        double imbalance = std::abs(load_gpu0 - load_gpu1) / std::max(load_gpu0, load_gpu1) * 100.0;
        std::cout << "Workload imbalance: " << imbalance << "%\n";

    } 
    else if (paralel_option == USE_CHUNKS){
        const int max_chunk_height = 128;
        std::vector<Chunk> chunks = createChunksForAllImages(images, max_chunk_height);
        writeOutput(images, files, chunks, WRITE_CHUNKS, false);
    } 
    else if(paralel_option == ONE_DEVICE){
        //std::thread gpu0_thread(processImagesOnGPU_optimized_times, context_gpu0, devices_ids[0][0], program_gpu0, std::ref(images), 10);
        //gpu0_thread.join();
        std::thread gpu1_thread(processImagesOnGPU_optimized_times, context_gpu1, devices_ids[0][1], program_gpu1, std::ref(images), 11);
        gpu1_thread.join();
    }
    else if(paralel_option == SIMPLE_BALANCED){
        std::vector<CImg<unsigned char>> images_gpu0, images_gpu1;
        //balance_images(images, images_gpu0, images_gpu1);
        balance_images_bandwidth(images, images_gpu0, images_gpu1, 11500, 14000);

        std::thread gpu0_thread(processImagesOnGPU_optimized_times, context_gpu0, devices_ids[0][0], program_gpu0, std::ref(images_gpu0), 20);
        std::thread gpu1_thread(processImagesOnGPU_optimized_times, context_gpu1, devices_ids[0][1], program_gpu1, std::ref(images_gpu1), 21);

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