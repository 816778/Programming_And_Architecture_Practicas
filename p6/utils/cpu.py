import numpy as np
from concurrent.futures import ThreadPoolExecutor


def grayscale_cpu(image):
    # Asume que la imagen está en formato (alto, ancho, canales)
    return np.dot(image[..., :3], [0.299, 0.587, 0.114]).astype(np.uint8)




def process_images_cpu(images):
    with ThreadPoolExecutor() as executor:
        results = list(executor.map(grayscale_cpu, images))
    return results

