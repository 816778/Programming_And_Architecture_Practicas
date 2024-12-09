__kernel void pow_of_two(
  __global float *in,
  __global float *out,
  const unsigned int count){

  int i = get_global_id(0);  // Get the global ID for the first dimension (1D kernel)

  if(i < count){
    out[i] = in[i] * in[i];
  }
}


// sobel filter with bidimensional kernel and input/output images
__kernel void sobel_filter(
  __global float *in,
  __global float *sobel_x,
  __global float *sobel_y,
  __global float *out,
  const unsigned int width,
  const unsigned int height
  ){

    int x = get_global_id(0);  // Get the global ID for the first dimension (2D kernel)
    int y = get_global_id(1);  // Get the global ID for the second dimension (2D kernel)
    
    // Apply the sobel filter for the x-axis and the y-axis
    float dx = 0.0, dy = 0.0;
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        int coord_x = x+i-1, coord_y = y+j-1;
        if (coord_x < 0) coord_x = 0;
        if (coord_y < 0) coord_y = 0;
        if (coord_x >= width) coord_x = width-1;
        if (coord_y >= height) coord_y = height-1;
        dx += in[coord_x, coord_y*width] * sobel_x[i + j*3];
        dy += in[coord_x, coord_y*width] * sobel_y[i + j*3];
      }
    }

    // Compute the magnitude of the gradient
    out[x + y*width] = sqrt(dx*dx + dy*dy);
    out[x + y*width] = out[x + y*width] * 63.75 + 127.5;
    if (out[x + y*width] > 255) out[x + y*width] = 255;
    if (out[x + y*width] < 0) out[x + y*width] = 0;

}

