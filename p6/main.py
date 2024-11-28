"""
Simular un Flujo de Imágenes en Memoria

Descripción: Cargar una imagen desde el disco y convertirla a un array de píxeles.



"""

from PIL import Image
import numpy as np
import random
import utils.cpu as cpu
import utils.gpu as gpu


def modify_image(image):
    # Cambiar brillo con un factor aleatorio
    factor = random.uniform(0.8, 1.2)  # Factor de brillo entre 80% y 120%
    modified_image = np.clip(image * factor, 0, 255).astype(np.uint8)
    return modified_image


if __name__ == "__main__":
    # Cargar la imagen base desde el disco
    base_image = Image.open("../images/dog.jpg")
    base_image_array = np.array(base_image) 
    print(base_image_array.shape)

    # Replicar imagen en memoria
    num_images = 1
    replicated_stream = np.tile(base_image_array, (num_images, 1, 1, 1))  # Repetir en un nuevo eje
    print(replicated_stream.shape) 

    # Introducir variaciones (opcional)
    varied_stream = [modify_image(base_image_array) for _ in range(num_images)]
    
    # Validar la simulación
    print("Dimensiones de la simulación:", replicated_stream.shape)  # (5000, alto, ancho, canales)
    print("Primera imagen:", replicated_stream[0])

    test_image = np.random.randint(0, 255, (256, 256, 3), dtype=np.uint8)

    result_cpu = cpu.grayscale_cpu(test_image)
    result_gpu = gpu.grayscale_gpu(test_image)

    print("Resultados iguales:", np.array_equal(result_cpu, result_gpu))


