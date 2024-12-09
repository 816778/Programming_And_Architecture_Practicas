__kernel void pow_of_two(
  __global float *in,
  __global float *out,
  const unsigned int count){

  int i = get_global_id(0);  // Get the global ID for the first dimension (1D kernel)

  if(i < count){
    out[i] = in[i] * in[i];
  }
}


__kernel void copy_image(
  __global const float *in,  // Imagen de entrada
  __global const float *sobel_x,  // No utilizado (para mantener la misma firma)
  __global const float *sobel_y,  // No utilizado (para mantener la misma firma)
  __global float *out,  // Imagen de salida
  const unsigned int width,  // Ancho de la imagen
  const unsigned int height   // Alto de la imagen
  ) {
    // Obtener coordenadas globales
    int x = get_global_id(0); // Coordenada x en el espacio global
    int y = get_global_id(1); // Coordenada y en el espacio global

    //printf("Thread (%d, %d)\n", x, y);

    // Verificar si las coordenadas están dentro de los límites de la imagen
    if (x < width && y < height) {
        // Calcular el índice lineal en el buffer
        int index = y * width + x;

        // Copiar el valor del píxel de entrada al de salida
        out[index] = in[index];
    }
}


__kernel void sobel_filter(
    __global const float* input,     // Imagen de entrada
    __global const float* sobel_x,  // Kernel Sobel X
    __global const float* sobel_y,  // Kernel Sobel Y
    __global float* output,         // Imagen de salida
    const unsigned int width,       // Ancho de la imagen
    const unsigned int height)      // Alto de la imagen
{
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x >= width || y >= height) return;

    // Coordenada central
    float gx = 0.0f;
    float gy = 0.0f;

    // Aplicar las máscaras Sobel (3x3)
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int nx = x + i;
            int ny = y + j;

            // Verificar bordes
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                float pixel = input[ny * width + nx];
                gx += pixel * sobel_x[(i + 1) * 3 + (j + 1)];
                gy += pixel * sobel_y[(i + 1) * 3 + (j + 1)];
            }
        }
    }

    // Magnitud del gradiente
    float magnitude = sqrt(gx * gx + gy * gy);

    // Guardar el resultado
    output[y * width + x] = magnitude;
}
