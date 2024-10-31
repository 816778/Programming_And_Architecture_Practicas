import matplotlib.pyplot as plt

RESULTS_FOLDER="results/"

def plot_time_vs_pixels(time_data):
    # Extraer datos de time_data
    x = [entry["time_ms"] for entry in time_data]  # Tiempo en milisegundos
    y = [entry["column"] * entry["row"] for entry in time_data]  # Tamaño del bloque (pixeles)
    
    # Crear la gráfica
    plt.figure(figsize=(10, 6))
    plt.scatter(x, y, color='blue', marker='o', label="Tiempo vs. Tamaño de bloque")
    plt.plot(x, y, linestyle='--', color='gray', alpha=0.5)  # Añadir línea para seguir los puntos

    # Etiquetas y título
    plt.xlabel("Tiempo (ms)")
    plt.ylabel("Tamaño del bloque (columnas * filas)")
    plt.title("Relación entre el tiempo de ejecución y el tamaño del bloque")
    plt.legend()
    plt.grid(True)

    # Mostrar la gráfica
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


if __name__ == "__main__":
    filename = RESULTS_FOLDER + 'ex_3.txt'

    time_data = read_time_data(filename)
    plot_time_vs_pixels(time_data)

