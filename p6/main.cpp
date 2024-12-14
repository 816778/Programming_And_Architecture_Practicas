////////////////////////////////////////////////////////////////////
//File: basic_environ.c
//
//Description: base file for environment exercises with openCL
//g++ main.cpp -o main -lOpenCL -I -lm -lpthread -lX11 -ljpeg
//  
////////////////////////////////////////////////////////////////////
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
#include "CImg.h"
using namespace cimg_library;

int main(int argc, char** argv) {
    // Cargar imagen base
    std::string img_file = "images/image.jpg";
    CImg<unsigned char> img(img_file.c_str());
    CImg<unsigned char> img_gray = img.get_RGBtoYCbCr().get_channel(0);

    // Crear flujo de imágenes simuladas (5000 copias)
    const int num_images = 5000;
    std::vector<CImg<unsigned char>> image_stream;
    for (int i = 0; i < num_images; i++) {
        image_stream.push_back(img_gray);
    }

    fprintf(stderr, "Flujo de imágenes simulado: %d imágenes de %d x %d\n",
            num_images, img_gray.width(), img_gray.height());
    
    return 0;
}
