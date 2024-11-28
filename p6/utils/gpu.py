import pycuda.driver as cuda
import pycuda.autoinit
from pycuda.compiler import SourceModule
import numpy as np

mod = SourceModule("""
__global__ void grayscale(unsigned char *input, unsigned char *output, int width, int height) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x < width && y < height) {
        int idx = (y * width + x) * 3;  // Índice para R, G, B
        unsigned char r = input[idx];
        unsigned char g = input[idx + 1];
        unsigned char b = input[idx + 2];
        unsigned char gray = (unsigned char)(0.299f * r + 0.587f * g + 0.114f * b);
        output[y * width + x] = gray;
    }
}
""")

grayscale = mod.get_function("grayscale")

def grayscale_gpu(input_image):
    height, width, _ = input_image.shape
    input_gpu = cuda.mem_alloc(input_image.nbytes)
    output_gpu = cuda.mem_alloc(height * width)
    cuda.memcpy_htod(input_gpu, input_image)

    block = (16, 16, 1)
    grid = (width // block[0] + 1, height // block[1] + 1, 1)

    grayscale(input_gpu, output_gpu, np.int32(width), np.int32(height), block=block, grid=grid)

    output_image = np.empty((height, width), dtype=np.uint8)
    cuda.memcpy_dtoh(output_image, output_gpu)
    return output_image
