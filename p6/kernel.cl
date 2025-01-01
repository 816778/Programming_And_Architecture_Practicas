__kernel void pow_of_two (
  __global float *in,
  __global float *out,
  const unsigned int count)
{

  int i = get_global_id(0);  // Get the global ID for the first dimension (1D kernel)

  if(i < count){
    out[i] = in[i] * in[i];
  }
}



__kernel void sobel_filter(
    __global const float* input,  // Imagen de entrada
    __global float* output,       // Imagen de salida
    const unsigned int width,     // Ancho de la imagen
    const unsigned int height     // Altura de la imagen
) {
    // Definir las matrices Sobel
    const float sobel_x[9] = {
        -1.0f, 0.0f, 1.0f,
        -2.0f, 0.0f, 2.0f,
        -1.0f, 0.0f, 1.0f
    };

    const float sobel_y[9] = {
        1.0f,  2.0f,  1.0f,
        0.0f,  0.0f,  0.0f,
        -1.0f, -2.0f, -1.0f
    };


    // Tamaño del bloque local (16x16 + bordes)
    __local float local_mem[18][18]; // Memoria local con márgenes para bordes

    // Índices globales
    int gx = get_global_id(0); // Índice global X
    int gy = get_global_id(1); // Índice global Y

    // Índices locales
    int lx = get_local_id(0);  // Índice local X
    int ly = get_local_id(1);  // Índice local Y

    // Tamaño del grupo de trabajo
    int local_size_x = get_local_size(0); // Normalmente 16
    int local_size_y = get_local_size(1); // Normalmente 16

    //printf("Work-item (%d, %d): local_id = (%d, %d)\n", get_local_id(0), get_local_id(1), local_size_x, local_size_y);

    // Índices en memoria local (ajustados para bordes)
    int local_x = lx + 1;
    int local_y = ly + 1;

    // Carga del píxel central en memoria local
    if (gx < width && gy < height) {
        local_mem[local_y][local_x] = input[gy * width + gx];
    } else {
        local_mem[local_y][local_x] = 0.0f;
    }

    // Carga de bordes horizontales
    if (ly == 0 && gy > 0) {
        local_mem[0][local_x] = input[(gy - 1) * width + gx];
    } else if (ly == local_size_y - 1 && gy < height - 1) {
        local_mem[local_y + 1][local_x] = input[(gy + 1) * width + gx];
    }

    // Carga de bordes verticales
    if (lx == 0 && gx > 0) {
        local_mem[local_y][0] = input[gy * width + (gx - 1)];
    } else if (lx == local_size_x - 1 && gx < width - 1) {
        local_mem[local_y][local_x + 1] = input[gy * width + (gx + 1)];
    }

    // Cargar las esquinas
    if (lx == 0 && ly == 0 && gx > 0 && gy > 0) {
        local_mem[0][0] = input[(gy - 1) * width + (gx - 1)];
    }
    if (lx == local_size_x - 1 && ly == 0 && gx < width - 1 && gy > 0) {
        local_mem[0][local_x + 1] = input[(gy - 1) * width + (gx + 1)];
    }
    if (lx == 0 && ly == local_size_y - 1 && gx > 0 && gy < height - 1) {
        local_mem[local_y + 1][0] = input[(gy + 1) * width + (gx - 1)];
    }
    if (lx == local_size_x - 1 && ly == local_size_y - 1 && gx < width - 1 && gy < height - 1) {
        local_mem[local_y + 1][local_x + 1] = input[(gy + 1) * width + (gx + 1)];
    }

    // Sincronizar memoria local
    barrier(CLK_LOCAL_MEM_FENCE);

    // Aplicar el filtro Sobel usando las matrices sobel_x y sobel_y
    if (gx > 0 && gx < width - 1 && gy > 0 && gy < height - 1) {
        float Gx = 0.0f;
        float Gy = 0.0f;

        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                float pixel = local_mem[local_y + i][local_x + j];
                Gx += sobel_x[(i + 1) * 3 + (j + 1)] * pixel;
                Gy += sobel_y[(i + 1) * 3 + (j + 1)] * pixel;
            }
        }

        // Magnitud del gradiente
        output[gy * width + gx] = sqrt(Gx * Gx + Gy * Gy);
    }
}






__kernel void sobel_filter_global(
    __global const float* input,    // Input image
    __global float* output,         // Output image
    const unsigned int width,       // Image width
    const unsigned int height)      // Image height
{

    const float sobel_x[3][3] = {
        {-1.0f, 0.0f, 1.0f},
        {-2.0f, 0.0f, 2.0f},
        {-1.0f, 0.0f, 1.0f}
    };
    const float sobel_y[3][3] = {
        { 1.0f,  2.0f,  1.0f},
        { 0.0f,  0.0f,  0.0f},
        {-1.0f, -2.0f, -1.0f}
    };

    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x >= width || y >= height) return;

    float gx = 0.0f;
    float gy = 0.0f;

    // Apply the Sobel filter
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int nx = x + i;
            int ny = y + j;

            // Verify borders
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                float pixel = input[ny * width + nx];
                gx += pixel * sobel_x[i + 1][j + 1];
                gy += pixel * sobel_y[i + 1][j + 1];
            }
        }
    }

    // Compute and store the magnitude of the gradient
    float magnitude = sqrt(gx * gx + gy * gy);
    output[y * width + x] = magnitude;
}



