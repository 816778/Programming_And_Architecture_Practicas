import csv
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt


RESULTS_FOLDER="results/"

def plot_surface(data, remove_largest=False):
    if remove_largest:
        # Sort the data by time_ms in descending order and remove the two largest entries
        data = sorted(data, key=lambda x: x["tiempo_ms"], reverse=True)[3:]
    # Convertir datos en arreglos separados
    rangos = np.array([entry['rango'] for entry in data])
    threads = np.array([entry['threads'] for entry in data])
    tiempos = np.array([entry['tiempo_ms'] for entry in data])

    # Crear figura y eje 3D
    fig = plt.figure(figsize=(10, 7))
    ax = fig.add_subplot(111, projection='3d')

    # Crear la superficie 3D
    surf = ax.plot_trisurf(threads, rangos, tiempos, cmap='viridis', edgecolor='none')

    # Etiquetas y título
    ax.set_xlabel('Number of Threads')
    ax.set_ylabel('Prime Search Range')
    ax.set_zlabel('Execution Time (ms)')
    ax.set_title('Execution Time vs. Prime Search Range and Number of Threads')
    
    fig.colorbar(surf, shrink=0.5, aspect=5)
    plt.show()


def plot_threads_vs_time(time_data):
    # Extraer el número de hilos y el tiempo de ejecución
    threads = [entry['threads'] for entry in time_data]
    times = [entry['tiempo_ms'] for entry in time_data]
    
    # Crear el gráfico
    plt.figure(figsize=(10, 6))
    plt.plot(threads, times, marker='o', linestyle='-', color='b', label='Execution Time')
    
    # Etiquetas y título
    plt.xlabel('Number of Threads')
    plt.ylabel('Execution Time (ms)')
    plt.title('Execution Time vs. Number of Threads')
    plt.legend()
    plt.grid(True)
    
    # Mostrar el gráfico
    plt.show()


def plot_heatmap(data, remove_largest=True):
    if remove_largest:
        data = sorted(data, key=lambda x: x["tiempo_ms"], reverse=True)[1:]

    df = pd.DataFrame(data)
    heatmap_data = df.pivot("rango", "threads", "tiempo_ms")

    # Crear el heatmap
    plt.figure(figsize=(10, 7))
    sns.heatmap(heatmap_data, cmap="YlGnBu", annot=True, fmt=".1f", cbar_kws={'label': 'Execution Time (ms)'})
    
    # Etiquetas y título
    plt.xlabel('Number of Threads')
    plt.ylabel('Prime Search Range')
    plt.title('Execution Time Heatmap: Prime Search Range vs. Threads')
    plt.show()



def read_time_data(filename):
    time_data = []
    with open(filename, 'r') as file:
        reader = csv.reader(file)
        for row in reader:
            # Convertir cada valor a un tipo de datos adecuado
            rango = int(row[0])
            threads = int(row[1])
            tiempo_ms = int(row[2])
            # Agregar un diccionario para cada fila en la lista
            time_data.append({
                'rango': rango,
                'threads': threads,
                'tiempo_ms': tiempo_ms
            })
    return time_data



def calculate_speedup(data):
    baseline_times = {}
    for entry in data:
        if entry['threads'] == 1:  # Solo 1 hilo (T1) para cada rango
            rango = entry['rango']
            baseline_times[rango] = entry['tiempo_ms']

    for entry in data:
        rango = entry['rango']
        tiempo_con_hilos = entry['tiempo_ms']
        # Calcular el speedup usando el tiempo base (T1) para ese rango
        entry['speedup'] = baseline_times[rango] / tiempo_con_hilos

    return data



def plot_speedup(data):
    # Obtener los valores únicos de rango
    rangos_unicos = sorted(set(entry['rango'] for entry in data))
    
    plt.figure(figsize=(10, 6))
    
    # Graficar el speedup para cada rango
    for rango in rangos_unicos:
        # Filtrar los datos para el rango actual
        threads = [entry['threads'] for entry in data if entry['rango'] == rango]
        speedups = [entry['speedup'] for entry in data if entry['rango'] == rango]
        
        # Graficar el rango actual
        plt.plot(threads, speedups, marker='o', label=f'Rango {rango}')
    
    # Configurar etiquetas y título
    plt.xlabel('Número de Hilos')
    plt.ylabel('Speedup')
    plt.title('Speedup vs. Número de Hilos para Diferentes Rangos')
    plt.legend()
    plt.grid(True)
    
    # Mostrar el gráfico
    plt.show()


if __name__ == "__main__":
    filename = RESULTS_FOLDER + 'ex_beta.txt'  # Nombre del archivo con los datos
    filename2 = RESULTS_FOLDER + 'ex_beta_speedup.txt'
    time_data = read_time_data(filename2)
    # plot_threads_vs_time(time_data)
    # plot_heatmap(time_data)
    plot_speedup(calculate_speedup(time_data))

