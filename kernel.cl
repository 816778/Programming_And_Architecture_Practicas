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

    __local float cache[1024];  // Local memory for the work group

    int x = get_global_id(0);  // Get the global ID for the first dimension (2D kernel)
    int y = get_global_id(1);  // Get the global ID for the second dimension (2D kernel)
    int xl = get_local_id(0) + 1;  // Get the local ID for the first dimension (2D kernel)
    int yl = get_local_id(1) + 1;  // Get the local ID for the second dimension (2D kernel)

    // Load the data from the global memory to the local memory
    if (xl == 1) {
      cache[xl-1 + yl*width] = in[x + y*width] / 255.0;
      if (yl == 1) cache[xl-1 + (yl-1)*width] = in[x + y*width] / 255.0;
      if (yl == height) cache[xl-1 + (yl+1)*width] = in[x + y*width] / 255.0;
    }
    if (xl == width) {
      cache[xl+1 + yl*width] = in[x + y*width] / 255.0;
      if (yl == 1) cache[xl+1 + (yl-1)*width] = in[x + y*width] / 255.0;
      if (yl == height) cache[xl+1 + (yl+1)*width] = in[x + y*width] / 255.0;
    }
    if (yl == 1) cache[xl + (yl-1)*width] = in[x + y*width] / 255.0;
    if (yl == height) cache[xl + (yl+1)*width] = in[x + y*width] / 255.0;
    cache[xl+1 + yl*width] = in[x + y*width] / 255.0;

    // Wait for all the threads in the work group to finish copying
    barrier(CLK_LOCAL_MEM_FENCE);
    
    // Apply the sobel filter for the x-axis and the y-axis
    float dx = 0.0, dy = 0.0;
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        dx += cache[xl+i-1 + (yl+j-1)*width] * sobel_x[i + j*3];
        dy += cache[xl+i-1 + (yl+j-1)*width] * sobel_y[i + j*3];
      }
    }

    // Wait for all the threads in the work group to finish applying the sobel filter
    barrier(CLK_LOCAL_MEM_FENCE);

    // Compute the magnitude of the gradient
    out[x + y*width] = sqrt(dx*dx + dy*dy);
    out[x + y*width] = out[x + y*width] * 63.75 + 127.5;
    if (out[x + y*width] > 255) out[x + y*width] = 255;
    if (out[x + y*width] < 0) out[x + y*width] = 0;

}

