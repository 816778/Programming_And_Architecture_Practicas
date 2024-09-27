import matplotlib.pyplot as plt


def read_times_from_file(filename):
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

def extract_matrix_times(filename):
    # Inicializar listas vacías para los datos
    sizes = []
    time_init_matrix = []
    time_multiplication_matrix = []

    # Abrir el archivo y leer línea por línea
    with open(filename, 'r') as file:
        lines = file.readlines()

        # Iterar sobre las líneas del archivo, en bloques de 3 líneas
        i = 0
        while i < len(lines):
            # Extraer el size
            size = int(lines[i].strip())
            sizes.append(size)

            # Extraer el tiempo de inicialización
            init_time = float(lines[i+1].strip())
            time_init_matrix.append(init_time)

            # Extraer el tiempo de multiplicación
            multiplication_time = float(lines[i+2].strip())
            time_multiplication_matrix.append(multiplication_time)

            i += 3

    return sizes, time_init_matrix, time_multiplication_matrix

# Función para graficar los tiempos real, user y sys en función del tamaño de la matriz
def plot_times(size, real_times, user_times, sys_times, title):
    plt.figure(figsize=(10, 6))

    plt.plot(size, real_times, label="Real Time", marker='o')
    plt.plot(size, user_times, label="User Time", marker='o')
    plt.plot(size, sys_times, label="Sys Time", marker='o')

    plt.title(f'Comparación de Tiempos: {title}')
    plt.xlabel('Tamaño de la Matriz (Size)')
    plt.ylabel('Tiempo (Segundos)')

    plt.legend()
    plt.grid(True)
    plt.show()

def compare_two_files(file1, file2, file3, title):
    size1, real_times1, user_times1, sys_times1 = read_times_from_file(file1)
    size2, time_init_matrix, time_multiplication_matrix = extract_matrix_times(file2)
    size3, time_init_matrix_3, time_multiplication_matrix_3 = extract_matrix_times(file3)

    time_init_matrix = [t * 1000 for t in time_init_matrix]
    time_multiplication_matrix = [t * 1000 for t in time_multiplication_matrix]
    time_init_matrix_3 = [t * 1000 for t in time_init_matrix_3]
    time_multiplication_matrix_3 = [t * 1000 for t in time_multiplication_matrix_3]
    

    plt.figure(figsize=(10, 6))

    plt.plot(size1, real_times1, label=f"Real Time My implementation", marker='o', linestyle='-', color='b')
    plt.plot(size1, user_times1, label=f"User Time My implementation", marker='o', linestyle='--', color='b')
    plt.plot(size1, sys_times1, label=f"Sys Time My implementation", marker='o', linestyle=':', color='b')

    plt.plot(size2, time_init_matrix, label="Time Init Matrix", marker='o', linestyle='-', color='r')
    plt.plot(size2, time_multiplication_matrix, label="Time Multiplication Matrix", marker='o', linestyle='--', color='r')

    plt.plot(size2, time_init_matrix_3, label="Time Init Matrix time", marker='o', linestyle='-', color='g')
    plt.plot(size2, time_multiplication_matrix_3, label="Time Multiplication Matrix time", marker='o', linestyle='--', color='g')

    plt.title(title)
    plt.xlabel('Size matrix')
    plt.ylabel('Time (Miliseconds)')

    plt.subplots_adjust(
        top=0.945,
        bottom=0.095,
        left=0.08,
        right=0.98,
        hspace=0.2,
        wspace=0.2
    )
    
    plt.legend()
    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    FOLDER_PATH = 'results/'
    FOLDER_PATH_MY_MATRIX = FOLDER_PATH + 'my_matrix/'
    FOLDER_PATH_EIGEN = FOLDER_PATH + 'eigen/'

    compare_two_files(FOLDER_PATH_MY_MATRIX + 'time_output.txt', FOLDER_PATH_MY_MATRIX + 'output_1.txt', FOLDER_PATH_MY_MATRIX + 'output_2.txt', 'Time comparacion My implementation')
    compare_two_files(FOLDER_PATH_EIGEN + 'time_output.txt', FOLDER_PATH_EIGEN + 'output_1.txt', FOLDER_PATH_EIGEN + 'output_2.txt', 'Time comparacion Eigen')
    exit()
    