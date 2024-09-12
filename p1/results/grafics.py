import matplotlib.pyplot as plt


def read_times_from_file(filename):
    size = []
    real_times = []
    user_times = []
    sys_times = []

    # Abrir el archivo en modo lectura
    with open(filename, 'r') as file:
        for line in file:
            # Dividir la línea por espacios
            parts = line.split()
            if len(parts) == 4:
                # Agregar los valores a las listas correspondientes
                size.append(int(parts[0]))
                real_times.append(float(parts[1]))
                user_times.append(float(parts[2]))
                sys_times.append(float(parts[3]))

    return size, real_times, user_times, sys_times


# Función para graficar los tiempos real, user y sys en función del tamaño de la matriz
def plot_times(size, real_times, user_times, sys_times, title):
    plt.figure(figsize=(10, 6))

    # Graficar cada uno de los tiempos con su etiqueta
    plt.plot(size, real_times, label="Real Time", marker='o')
    plt.plot(size, user_times, label="User Time", marker='o')
    plt.plot(size, sys_times, label="Sys Time", marker='o')

    # Título y etiquetas de los ejes
    plt.title(f'Comparación de Tiempos: {title}')
    plt.xlabel('Tamaño de la Matriz (Size)')
    plt.ylabel('Tiempo (Segundos)')

    # Añadir una leyenda para identificar las líneas
    plt.legend()

    # Mostrar la cuadrícula
    plt.grid(True)

    # Mostrar la gráfica
    plt.show()


file_names = ['time_matrix.txt', 'time_matrix_2.txt', 'time_matrix_eg.txt', 'time_matrix_eg2.txt']

# Bucle para procesar cada archivo y graficar los resultados
for file_name in file_names:
    size, real_times, user_times, sys_times = read_times_from_file(file_name)
    plot_times(size, real_times, user_times, sys_times, title=file_name)
