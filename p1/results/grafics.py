import matplotlib.pyplot as plt


def read_times_from_file_2(filename):
    size = []
    real_times = []
    user_times = []
    sys_times = []

    with open(filename, 'r') as file:
        current_size = None 
        for line in file:
            line = line.strip()
            if line.startswith("# size"):
                continue  # Saltar el encabezado
            elif line.startswith("#"):
                continue  # Saltar comentarios
            elif line.isdigit():
                current_size = int(line)
                size.append(current_size)
            elif line.startswith("real"):
                # tiempo real
                time_str = line.split()[1]
                real_time = parse_time_string_to_ms(time_str)
                real_times.append(real_time)
            elif line.startswith("user"):
                # tiempo de usuario
                time_str = line.split()[1]
                user_time = parse_time_string_to_ms(time_str)
                user_times.append(user_time)
            elif line.startswith("sys"):
                # tiempo de sistema
                time_str = line.split()[1]
                sys_time = parse_time_string_to_ms(time_str)
                sys_times.append(sys_time)

    return size, real_times, user_times, sys_times

def parse_time_string_to_ms(time_str):
    """Convierte un string de tiempo (como '0m2,524s') a milisegundos en formato float."""
    minutes, seconds = time_str.split('m')
    seconds = seconds.replace('s', '').replace(',', '.') 
    total_seconds = float(minutes) * 60 + float(seconds)
    return total_seconds * 1000 


def read_times_from_file(filename):
    size = []
    real_times = []
    user_times = []
    sys_times = []
    stddev_real_times = []
    stddev_user_times = []
    stddev_sys_times = []

    with open(filename, 'r') as file:
        for line in file:
            line = line.strip()

            # Saltar líneas vacías o encabezados
            if not line or line.startswith("size"):
                continue

            # Dividir la línea en partes separadas por espacio
            parts = line.split()

            # Asegurarse de que la línea tiene el número esperado de elementos
            if len(parts) == 7:
                current_size = int(parts[0])
                mean_real = float(parts[1])
                mean_user = float(parts[2])
                mean_sys = float(parts[3])
                stddev_real = float(parts[4])
                stddev_user = float(parts[5])
                stddev_sys = float(parts[6])

                # Añadir los valores a las listas correspondientes
                size.append(current_size)
                real_times.append(mean_real)
                user_times.append(mean_user)
                sys_times.append(mean_sys)
                stddev_real_times.append(stddev_real)
                stddev_user_times.append(stddev_user)
                stddev_sys_times.append(stddev_sys)

    return size, real_times, user_times, sys_times#, stddev_real_times, stddev_user_times, stddev_sys_times



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

    plt.legend()
    plt.grid(True)
    plt.show()

def compare_two_files(file1, file2):
    size1, real_times1, user_times1, sys_times1 = read_times_from_file(file1)
    size2, real_times2, user_times2, sys_times2 = read_times_from_file(file2)

    plt.figure(figsize=(10, 6))

    plt.plot(size1, real_times1, label=f"Real Time My implementation", marker='o', linestyle='-', color='b')
    plt.plot(size1, user_times1, label=f"User Time My implementation", marker='o', linestyle='--', color='b')
    plt.plot(size1, sys_times1, label=f"Sys Time My implementation", marker='o', linestyle=':', color='b')

    plt.plot(size2, real_times2, label=f"Real Time Eigen", marker='s', linestyle='-', color='r')
    plt.plot(size2, user_times2, label=f"User Time Eigen", marker='s', linestyle='--', color='r')
    plt.plot(size2, sys_times2, label=f"Sys Time Eigen", marker='s', linestyle=':', color='r')

    plt.title(f'Comparación de Tiempos entre {file1} y {file2}')
    plt.xlabel('Tamaño de la Matriz (Size)')
    plt.ylabel('Tiempo (Milisegundos)')

    plt.legend()
    plt.grid(True)
    plt.show()

compare_two_files('results/time_matrix_2.txt', 'results/time_matrix_eg.txt')
file_names = ['results/time_matrix.txt', 'results/time_matrix_2.txt', 'results/time_matrix_eg.txt']

for file_name in file_names:
    size, real_times, user_times, sys_times = read_times_from_file(file_name)
    plot_times(size, real_times, user_times, sys_times, title=file_name)
