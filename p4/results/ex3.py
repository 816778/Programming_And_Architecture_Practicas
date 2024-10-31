import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np


RESULTS_FOLDER="results/"


def plot_time_vs_regions(time_data):
    # Extract data from time_data
    x = [entry["time_ms"] for entry in time_data]  # Execution time in milliseconds
    y = [entry["column"] * entry["row"] for entry in time_data]  # Number of regions (columns * rows)
    
    # Create the plot
    plt.figure(figsize=(10, 6))
    plt.scatter(x, y, color='blue', marker='o', label="Execution Time vs. Number of Regions")
    plt.plot(x, y, linestyle='--', color='gray', alpha=0.5)  # Add line to connect points

    # Labels and title
    plt.xlabel("Execution Time (ms)")
    plt.ylabel("Number of Regions (columns * rows)")
    plt.title("Relationship between Execution Time and Number of Image Regions")
    plt.legend()
    plt.grid(True)
    plt.show()


def read_time_data(filename):
    data = []
    with open(filename, 'r') as file:
        for line in file:
            # Eliminar espacios y saltos de línea al final de cada línea
            line = line.strip()
            # Dividir la línea en tres partes: columna, fila y tiempo
            column, row, time_ms = line.split(',')
            # Convertir los valores a enteros y almacenar en un diccionario
            data.append({
                "column": int(column),
                "row": int(row),
                "time_ms": int(time_ms)
            })
    return data



def plot_3d_time_mesh(time_data):
    # Extraer los datos de columna, fila y tiempo en listas separadas
    columns = np.array([entry["column"] for entry in time_data])
    rows = np.array([entry["row"] for entry in time_data])
    times = np.array([entry["time_ms"] for entry in time_data])

    # Crear una figura en 3D
    fig = plt.figure(figsize=(10, 7))
    ax = fig.add_subplot(111, projection='3d')

    # Graficar la malla
    ax.plot_trisurf(columns, rows, times, cmap="viridis", edgecolor='none')

    # Etiquetas de los ejes
    ax.set_xlabel("Columnas")
    ax.set_ylabel("Filas")
    ax.set_zlabel("Tiempo (ms)")
    ax.set_title("Tiempo de Ejecución en función de Columnas y Filas")

    # Mostrar el gráfico
    plt.show()




if __name__ == "__main__":
    filename = RESULTS_FOLDER + 'ex_3.txt'

    time_data = read_time_data(filename)
    plot_3d_time_mesh(time_data)
    plot_time_vs_regions(time_data)

