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
    __global const float* input,    // Input image
    __global const float* sobel_x,  // Sobel x filter
    __global const float* sobel_y,  // Sobel y filter
    __global float* output,         // Output image
    const unsigned int width,       // Image width
    const unsigned int height)      // Image height
{
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
                gx += pixel * sobel_x[(i + 1) * 3 + (j + 1)];
                gy += pixel * sobel_y[(i + 1) * 3 + (j + 1)];
            }
        }
    }

    // Compute and store the magnitude of the gradient
    float magnitude = sqrt(gx * gx + gy * gy);
    output[y * width + x] = magnitude;
}
