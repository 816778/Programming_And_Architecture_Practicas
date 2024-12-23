#include <iostream>
#include <thread>
#include <vector>
#include <dirent.h>
#include <mutex>
#include "CImg.h"

using namespace cimg_library;

std::mutex console_mutex;

void create_directory_if_not_exists(const std::string& dir) {
    struct stat st;
    if (stat(dir.c_str(), &st) != 0) { // Verifica si el directorio no existe
        mkdir(dir.c_str(), 0777);     // Crea el directorio con permisos 0777
    }
}


void process_image(const std::string& input_path, const std::string& output_path) {
    try {
        CImg<unsigned char> img(input_path.c_str());
        CImg<unsigned char> img_gray = img.get_RGBtoYCbCr().get_channel(0);
        img_gray.save(output_path.c_str());

        // Sincronización para la salida en consola
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cout << "Procesada: " << input_path << " -> " << output_path << std::endl;
    } catch (const cimg_library::CImgException& e) {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cerr << "Error procesando " << input_path << ": " << e.what() << std::endl;
    }
}


void process_images_in_directory(const std::string& input_dir, const std::string& output_dir) {
    create_directory_if_not_exists(output_dir);

    DIR* dir = opendir(input_dir.c_str());
    if (!dir) {
        std::cerr << "Error: No se pudo abrir el directorio " << input_dir << std::endl;
        return;
    }

    std::vector<std::thread> threads; // Vector para almacenar los hilos
    struct dirent* entry;

    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        if (filename == "." || filename == "..") continue; // Ignorar directorios especiales

        std::string input_path = input_dir + "/" + filename;
        std::string output_path = output_dir + "/" + filename;

        // Crear un hilo para procesar cada imagen
        threads.emplace_back(process_image, input_path, output_path);
    }
    closedir(dir);

    // Esperar a que todos los hilos terminen
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    std::cout << "Todas las imágenes han sido procesadas de manera concurrente." << std::endl;
}

int main() {
    std::string input_dir = "images";  // Carpeta de entrada
    std::string output_dir = "grayscale_images";  // Carpeta de salida

    process_images_in_directory(input_dir, output_dir);
    return 0;

}
