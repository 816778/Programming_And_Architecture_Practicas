import pandas as pd
import matplotlib.pyplot as plt
import os
from PIL import Image
import numpy as np

def list_image(folder_path):
    """
    Lista el tamaño de cada imagen dentro de una carpeta, ordenadas por peso de archivo de mayor a menor.

    Args:
        folder_path (str): Ruta de la carpeta que contiene las imágenes.

    Returns:
        None: Imprime los tamaños y pesos de las imágenes en la consola.
    """
    if not os.path.isdir(folder_path):
        print(f"Error: La ruta '{folder_path}' no es una carpeta válida.")
        return
    
    # Crear una lista para almacenar la información de las imágenes
    image_info = []

    # Procesar cada archivo en la carpeta
    for file_name in os.listdir(folder_path):
        file_path = os.path.join(folder_path, file_name)
        if os.path.isfile(file_path):
            try:
                with Image.open(file_path) as img:
                    width, height = img.size
                file_size = os.path.getsize(file_path)  # Tamaño en bytes
                image_info.append((file_name, width, height, file_size))
            except Exception as e:
                print(f"{file_name}: No es una imagen válida ({e})")

    # Ordenar las imágenes por tamaño de archivo en bytes (de mayor a menor)
    image_info.sort(key=lambda x: x[3], reverse=True)

    # Imprimir las imágenes ordenadas
    print(f"Tamaños de imágenes en la carpeta '{folder_path}', ordenadas por peso:")
    for file_name, width, height, file_size in image_info:
        print(f"{file_name}: {width}x{height}, {file_size / 1024:.2f} KB")



def plot_opencl_metrics(df):
    """
    Genera gráficos a partir del DataFrame con las métricas calculadas.
    
    Args:
        df (pd.DataFrame): DataFrame con las columnas 'i', 'write_time', 'kernel_time', 'read_time', 'total_time'.
    """
    # Gráfico de tiempos individuales por índice
    plt.figure(figsize=(10, 6))
    plt.plot(df['i'], df['write_time'], label='Write Time', marker='o')
    plt.plot(df['i'], df['kernel_time'], label='Kernel Time', marker='s')
    plt.plot(df['i'], df['read_time'], label='Read Time', marker='^')
    plt.plot(df['i'], df['total_time'], label='Total Time', linestyle='--', color='black')
    plt.xlabel('Operation Index (i)')
    plt.ylabel('Time (ms)')
    plt.title('OpenCL Execution Times by Operation Index')
    plt.legend()
    plt.grid()
    plt.show()
    
    # Gráfico de barras apiladas para proporciones de tiempos
    plt.figure(figsize=(10, 6))
    bottom_bar = df['write_time']
    plt.bar(df['i'], df['write_time'], label='Write Time', color='blue')
    plt.bar(df['i'], df['kernel_time'], label='Kernel Time', bottom=bottom_bar, color='green')
    bottom_bar += df['kernel_time']
    plt.bar(df['i'], df['read_time'], label='Read Time', bottom=bottom_bar, color='red')
    plt.xlabel('Operation Index (i)')
    plt.ylabel('Time (ms)')
    plt.title('Proportional Execution Times per Operation')
    plt.legend()
    plt.grid()
    plt.show()


def process_opencl_output(file_path):
    """
    Procesa un archivo de salida de OpenCL con formato:
    i;write_times[i];kernel_times[i];read_times[i]

    Args:
        file_path (str): Ruta al archivo de entrada.

    Returns:
        pd.DataFrame: DataFrame con las métricas calculadas.
    """
    # Leer los datos del archivo
    df = pd.read_csv(file_path, sep=';', header=None, names=['i', 'write_time', 'kernel_time', 'read_time'])

    # Calcular métricas clave
    df['total_time'] = df['write_time'] + df['kernel_time'] + df['read_time']
    summary = {
        'total_operations': len(df),
        'average_write_time': df['write_time'].mean(),
        'average_kernel_time': df['kernel_time'].mean(),
        'average_read_time': df['read_time'].mean(),
        'average_total_time': df['total_time'].mean(),
        'max_total_time': df['total_time'].max(),
        'min_total_time': df['total_time'].min()
    }

    # Imprimir el resumen
    print("Resumen del archivo procesado:")
    for key, value in summary.items():
        print(f"{key}: {value}")

    return df, summary



def compare_opencl_metrics(df1, df2, label1='GPU 0', label2='GPU 1'):
    """
    Genera gráficos comparativos agregados de métricas entre dos DataFrames.
    Resume las métricas en promedios para facilitar la visualización.
    
    Args:
        df1 (pd.DataFrame): DataFrame con métricas del primer conjunto de datos.
        df2 (pd.DataFrame): DataFrame con métricas del segundo conjunto de datos.
        label1 (str): Etiqueta para el primer DataFrame.
        label2 (str): Etiqueta para el segundo DataFrame.
    """
    # Calcular estadísticas agregadas para cada métrica
    metrics = ['write_time', 'kernel_time', 'read_time', 'total_time']
    avg_metrics1 = [df1[metric].mean() for metric in metrics]
    avg_metrics2 = [df2[metric].mean() for metric in metrics]
    
    # Crear un gráfico de barras agrupadas para promedios
    x = np.arange(len(metrics))  # Posiciones en el eje X
    width = 0.35  # Ancho de las barras
    
    plt.figure(figsize=(10, 6))
    plt.bar(x - width/2, avg_metrics1, width, label=label1, color='blue')
    plt.bar(x + width/2, avg_metrics2, width, label=label2, color='orange')
    
    plt.xlabel('Metrics')
    plt.ylabel('Average Time (ms)')
    plt.title('Average Execution Times by Metric')
    plt.xticks(ticks=x, labels=metrics)
    plt.legend()
    plt.grid(axis='y')
    plt.show()
    
    # Crear un gráfico de líneas para una comparación más visual
    plt.figure(figsize=(10, 6))
    plt.plot(df1['i'], df1['total_time'], label=f'Total Time {label1}', alpha=0.5, color='blue')
    plt.plot(df2['i'], df2['total_time'], label=f'Total Time {label2}', alpha=0.5, color='orange')
    plt.xlabel('Operation Index (i)')
    plt.ylabel('Total Time (ms)')
    plt.title('Comparison of Total Execution Times (Smoothed)')
    plt.legend()
    plt.grid()
    plt.show()



if __name__ == "__main__":
    # list_image("images")
    DATA_PATH = "results/data/"
    files_path = [DATA_PATH + "times_gpu_0.txt", DATA_PATH + "times_gpu_1.txt", 
                  DATA_PATH + "times_gpu_10.txt", DATA_PATH + "times_gpu_11.txt",
                  DATA_PATH + "times_gpu_20.txt", DATA_PATH + "times_gpu_21.txt",]
    df = {}

    for i, file_path in enumerate(files_path):
        df[i], summary = process_opencl_output(file_path)
        print("\n")

    compare_opencl_metrics(df[0], df[1], label1='GPU 0', label2='GPU 1')
    compare_opencl_metrics(df[2], df[3], label1='GPU 10', label2='GPU 11')
    compare_opencl_metrics(df[4], df[5], label1='GPU 20', label2='GPU 21')