import matplotlib.pyplot as plt


def read_times_from_file(filename):
    size = []
    real_times = []
    user_times = []
    sys_times = []

    # Abrir el archivo en modo lectura
    with open(filename, 'r') as file:
        current_size = None  # Almacena el tamaño actual
        for line in file:
            line = line.strip()
            if line.startswith("# size"):
                continue  # Saltar el encabezado
            elif line.startswith("#"):
                continue  # Saltar comentarios
            elif line.isdigit():
                # Leer el tamaño de la matriz
                current_size = int(line)
                size.append(current_size)
            elif line.startswith("real"):
                # Extraer tiempo real
                time_str = line.split()[1]
                real_time = parse_time_string_to_ms(time_str)
                real_times.append(real_time)
            elif line.startswith("user"):
                # Extraer tiempo de usuario
                time_str = line.split()[1]
                user_time = parse_time_string_to_ms(time_str)
                user_times.append(user_time)
            elif line.startswith("sys"):
                # Extraer tiempo de sistema
                time_str = line.split()[1]
                sys_time = parse_time_string_to_ms(time_str)
                sys_times.append(sys_time)

    return size, real_times, user_times, sys_times

def parse_time_string_to_ms(time_str):
    """Convierte un string de tiempo (como '0m2,524s') a milisegundos en formato float."""
    minutes, seconds = time_str.split('m')
    seconds = seconds.replace('s', '').replace(',', '.')  # Reemplazar coma por punto decimal
    total_seconds = float(minutes) * 60 + float(seconds)
    return total_seconds * 1000  # Convertir a milisegundos



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


file_names = ['results/time_matrix.txt', 'results/time_matrix_2.txt', 'results/time_matrix_eg.txt', 'results/time_matrix_eg2.txt']

# Bucle para procesar cada archivo y graficar los resultados
for file_name in file_names:
    size, real_times, user_times, sys_times = read_times_from_file(file_name)
    plot_times(size, real_times, user_times, sys_times, title=file_name)
