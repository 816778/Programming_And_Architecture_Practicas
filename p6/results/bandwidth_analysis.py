import pandas as pd
import matplotlib.pyplot as plt

def analyze_bandwidth_file(file_path, gpu_number):
    """
    Analiza un archivo de ancho de banda con la estructura:
    i:data_size_bytes;write_bandwidth;read_bandwidth
    
    Args:
        file_path (str): Ruta al archivo.
        
    Returns:
        pd.DataFrame: DataFrame con los datos procesados.
    """
    # Leer los datos del archivo
    df = pd.read_csv(file_path, sep='[:;]', header=None, engine='python', 
                     names=['i', 'data_size_bytes', 'write_bandwidth', 'read_bandwidth'])
    
    # Convertir las columnas a los tipos de datos correctos
    df['i'] = df['i'].astype(int)
    df['data_size_bytes'] = df['data_size_bytes'].astype(float)
    df['write_bandwidth'] = df['write_bandwidth'].astype(float)
    df['read_bandwidth'] = df['read_bandwidth'].astype(float)
    
    # Calcular métricas
    metrics = {
        'Total Data Size (Bytes)': df['data_size_bytes'].sum(),
        'Average Write Bandwidth (MB/s)': df['write_bandwidth'].mean() / 1e6,
        'Average Read Bandwidth (MB/s)': df['read_bandwidth'].mean() / 1e6,
        'Max Write Bandwidth (MB/s)': df['write_bandwidth'].max() / 1e6,
        'Min Write Bandwidth (MB/s)': df['write_bandwidth'].min() / 1e6,
        'Max Read Bandwidth (MB/s)': df['read_bandwidth'].max() / 1e6,
        'Min Read Bandwidth (MB/s)': df['read_bandwidth'].min() / 1e6,
    }
    
    # Imprimir métricas
    print(f"Análisis del archivo de ancho de banda GPU: {gpu_number}")
    for key, value in metrics.items():
        print(f"{key}: {value:.2f}")
    print("\n")

    return df


def compare_bandwidth_files(df1, df2, label1='File 1', label2='File 2'):
    """
    Genera gráficos comparativos de ancho de banda entre dos DataFrames.

    Args:
        df1 (pd.DataFrame): DataFrame del primer fichero con columnas ['i', 'data_size_bytes', 'write_bandwidth', 'read_bandwidth'].
        df2 (pd.DataFrame): DataFrame del segundo fichero con columnas similares.
        label1 (str): Etiqueta para el primer DataFrame.
        label2 (str): Etiqueta para el segundo DataFrame.
    """
    # Gráfica comparativa de ancho de banda de escritura
    plt.figure(figsize=(10, 6))
    plt.plot(df1['i'], df1['write_bandwidth'] / 1e6, label=f'Write Bandwidth {label1} (MB/s)', color='blue', alpha=0.7)
    plt.plot(df2['i'], df2['write_bandwidth'] / 1e6, label=f'Write Bandwidth {label2} (MB/s)', color='cyan', alpha=0.7)
    plt.xlabel('Operation Index (i)')
    plt.ylabel('Write Bandwidth (MB/s)')
    plt.title('Comparison of Write Bandwidth')
    plt.legend()
    plt.grid()
    plt.show()

    # Gráfica comparativa de ancho de banda de lectura
    plt.figure(figsize=(10, 6))
    plt.plot(df1['i'], df1['read_bandwidth'] / 1e6, label=f'Read Bandwidth {label1} (MB/s)', color='orange', alpha=0.7)
    plt.plot(df2['i'], df2['read_bandwidth'] / 1e6, label=f'Read Bandwidth {label2} (MB/s)', color='red', alpha=0.7)
    plt.xlabel('Operation Index (i)')
    plt.ylabel('Read Bandwidth (MB/s)')
    plt.title('Comparison of Read Bandwidth')
    plt.legend()
    plt.grid()
    plt.show()

    # Gráfica combinada de escritura y lectura
    plt.figure(figsize=(10, 6))
    plt.plot(df1['i'], df1['write_bandwidth'] / 1e6, label=f'Write Bandwidth {label1} (MB/s)', color='blue', alpha=0.7)
    plt.plot(df1['i'], df1['read_bandwidth'] / 1e6, label=f'Read Bandwidth {label1} (MB/s)', color='orange', alpha=0.7)
    plt.plot(df2['i'], df2['write_bandwidth'] / 1e6, label=f'Write Bandwidth {label2} (MB/s)', color='cyan', alpha=0.7)
    plt.plot(df2['i'], df2['read_bandwidth'] / 1e6, label=f'Read Bandwidth {label2} (MB/s)', color='red', alpha=0.7)
    plt.xlabel('Operation Index (i)')
    plt.ylabel('Bandwidth (MB/s)')
    plt.title('Comparison of Write and Read Bandwidth')
    plt.legend()
    plt.grid()
    plt.show()


def plot_smoothed_bandwidth(df1, df2, label1='File 1', label2='File 2', window=50):
    """
    Grafica los anchos de banda suavizados usando un promedio móvil.

    Args:
        df1 (pd.DataFrame): DataFrame del primer archivo.
        df2 (pd.DataFrame): DataFrame del segundo archivo.
        label1 (str): Etiqueta para el primer archivo.
        label2 (str): Etiqueta para el segundo archivo.
        window (int): Tamaño de la ventana para el promedio móvil.
    """
    import matplotlib.pyplot as plt
    import pandas as pd

    # Suavizar los datos usando un promedio móvil
    smoothed_write1 = df1['write_bandwidth'].rolling(window=window).mean() / 1e6
    smoothed_read1 = df1['read_bandwidth'].rolling(window=window).mean() / 1e6
    smoothed_write2 = df2['write_bandwidth'].rolling(window=window).mean() / 1e6
    smoothed_read2 = df2['read_bandwidth'].rolling(window=window).mean() / 1e6

    # Gráfica combinada
    plt.figure(figsize=(10, 6))
    plt.plot(df1['i'], smoothed_write1, label=f'Smoothed Write Bandwidth {label1}', color='blue', alpha=0.8)
    plt.plot(df1['i'], smoothed_read1, label=f'Smoothed Read Bandwidth {label1}', color='orange', alpha=0.8)
    plt.plot(df2['i'], smoothed_write2, label=f'Smoothed Write Bandwidth {label2}', color='cyan', alpha=0.8)
    plt.plot(df2['i'], smoothed_read2, label=f'Smoothed Read Bandwidth {label2}', color='red', alpha=0.8)
    plt.xlabel('Operation Index (i)')
    plt.ylabel('Bandwidth (MB/s)')
    plt.title(f'Smoothed Bandwidth Comparison (Window = {window})')
    plt.legend()
    plt.grid()
    plt.show()


def plot_aggregated_bandwidth(df1, df2, label1='File 1', label2='File 2', num_segments=10):
    """
    Grafica los promedios del ancho de banda por segmentos de datos.

    Args:
        df1 (pd.DataFrame): DataFrame del primer archivo.
        df2 (pd.DataFrame): DataFrame del segundo archivo.
        label1 (str): Etiqueta para el primer archivo.
        label2 (str): Etiqueta para el segundo archivo.
        num_segments (int): Número de segmentos para dividir los datos.
    """
    import matplotlib.pyplot as plt
    import numpy as np

    # Dividir los datos en segmentos
    segment_size1 = len(df1) // num_segments
    segment_size2 = len(df2) // num_segments

    aggregated_write1 = [
        df1['write_bandwidth'][i:i + segment_size1].mean() / 1e6 for i in range(0, len(df1), segment_size1)
    ]
    aggregated_read1 = [
        df1['read_bandwidth'][i:i + segment_size1].mean() / 1e6 for i in range(0, len(df1), segment_size1)
    ]
    aggregated_write2 = [
        df2['write_bandwidth'][i:i + segment_size2].mean() / 1e6 for i in range(0, len(df2), segment_size2)
    ]
    aggregated_read2 = [
        df2['read_bandwidth'][i:i + segment_size2].mean() / 1e6 for i in range(0, len(df2), segment_size2)
    ]

    # Crear índices para los segmentos
    x = np.arange(len(aggregated_write1))

    # Gráfica de barras
    plt.figure(figsize=(10, 6))
    plt.bar(x - 0.2, aggregated_write1, width=0.4, label=f'Average Write Bandwidth {label1}', color='blue')
    plt.bar(x + 0.2, aggregated_write2, width=0.4, label=f'Average Write Bandwidth {label2}', color='cyan')
    plt.bar(x - 0.2, aggregated_read1, width=0.4, bottom=aggregated_write1, label=f'Average Read Bandwidth {label1}', color='orange')
    plt.bar(x + 0.2, aggregated_read2, width=0.4, bottom=aggregated_write2, label=f'Average Read Bandwidth {label2}', color='red')
    plt.xlabel('Segment')
    plt.ylabel('Bandwidth (MB/s)')
    plt.title('Aggregated Bandwidth by Segments')
    plt.legend()
    plt.grid()
    plt.show()




if __name__ == "__main__":
    DATA_PATH = "results/data/"
    files_path = [DATA_PATH + "bandwidth_gpu_30_4.txt", DATA_PATH + "bandwidth_gpu_31_4.txt",
                  DATA_PATH + "bandwidth_gpu_30_16.txt", DATA_PATH + "bandwidth_gpu_31_16.txt",
                  DATA_PATH + "bandwidth_gpu_30_32.txt", DATA_PATH + "bandwidth_gpu_31_32.txt",
                  DATA_PATH + "bandwidth_gpu_30_64.txt", DATA_PATH + "bandwidth_gpu_31_64.txt",
                  ]
    
    df = {}
    for i, file_path in enumerate(files_path):
        df[i] = analyze_bandwidth_file(file_path, i)
    
    # Comparar archivos de ancho de banda
    plot_smoothed_bandwidth(df[0], df[1], label1='GPU 30 4_chuncks', label2='GPU 31 4_chunks', window=50)
    plot_smoothed_bandwidth(df[2], df[3], label1='GPU 30 16_chuncks', label2='GPU 31 16_chunks', window=50)
    plot_smoothed_bandwidth(df[4], df[5], label1='GPU 30 32_chuncks', label2='GPU 31 32_chunks', window=50)
    plot_smoothed_bandwidth(df[6], df[7], label1='GPU 30 64_chuncks', label2='GPU 31 64_chunks', window=50)