#define cimg_use_jpeg
#include <iostream>
#include "CImg.h"

using namespace cimg_library;

void task1(CImg<unsigned char> img){
     // Dump the image data into an array
    int width = img.width();
    int height = img.height();
    int depth = img.depth();
    int spectrum = img.spectrum();
    
    // Create an array to hold image data
    unsigned char* imageData = new unsigned char[width * height * spectrum];
    int index = 0;
    
    // Copy the image data into the array
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int c = 0; c < spectrum; ++c) {
                imageData[index++] = img(x, y, 0, c);
            }
        }
    }
    
    // Prompt the user for a pixel coordinate
    int i, j;
    std::cout << "Enter the x-coordinate of the pixel: ";
    std::cin >> i;
    std::cout << "Enter the y-coordinate of the pixel: ";
    std::cin >> j;

    // Check if the coordinates are within the image boundaries
    if (i >= 0 && i < width && j >= 0 && j < height) {
        // Display the RGB values of the pixel
        std::cout << "Pixel value at (" << i << ", " << j << "): ";
        std::cout << "Red: " << (int)img(i, j, 0, 0) << ", ";
        std::cout << "Green: " << (int)img(i, j, 0, 1) << ", ";
        std::cout << "Blue: " << (int)img(i, j, 0, 2) << std::endl;
    } else {
        std::cout << "Coordinates are out of image bounds." << std::endl;
    }

    // Clean up
    delete[] imageData;
}

void task2(CImg<unsigned char> img){

    int width = img.width();
    int height = img.height();
    int centerX = width / 2;
    int centerY = height / 2;

    // Modify the image by adding a blue cross at the center
    unsigned char blue[3] = {0, 0, 255}; // Blue color (R=0, G=0, B=255)

    // Draw a vertical line at centerX
    for (int y = 0; y < height; ++y) {
        img(centerX, y, 0, 0) = blue[0]; // Red channel
        img(centerX, y, 0, 1) = blue[1]; // Green channel
        img(centerX, y, 0, 2) = blue[2]; // Blue channel
    }

    // Draw a horizontal line at centerY
    for (int x = 0; x < width; ++x) {
        img(x, centerY, 0, 0) = blue[0]; // Red channel
        img(x, centerY, 0, 1) = blue[1]; // Green channel
        img(x, centerY, 0, 2) = blue[2]; // Blue channel
    }

    // Display the modified image
    img.display("Modified Image with Blue Cross");
}

int main() {
    // Load the image
    CImg<unsigned char> img("images/image.jpg");
    task1(img);
    task2(img); 
    return 0;
}
