import numpy as np
import matplotlib.pyplot as plt


def parse_multiple_results(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    results = []  # Lista para almacenar todos los bloques de resultados
    current_result = {
        'time_init_matrix': None,
        'time_multiplication_matrix': None,
        'syscall_stats': []
    }
    
    in_syscall_table = False

    for line in lines:
        line = line.strip()

        # Detectar los tiempos de inicialización y multiplicación
        if current_result['time_init_matrix'] is None:
            try:
                current_result['time_init_matrix'] = float(line)
                continue
            except ValueError:
                pass
        
        if current_result['time_multiplication_matrix'] is None:
            try:
                current_result['time_multiplication_matrix'] = float(line)
                continue
            except ValueError:
                pass

        # Detectar el inicio de la tabla de estadísticas de llamadas al sistema
        if line.startswith('% time'):
            in_syscall_table = True
            continue

        if in_syscall_table:
            # Saltar las líneas de separadores o vacías
            if line.startswith('------') or not line:
                continue

            # Detectar si llegamos al final de una tabla de syscalls (línea que empieza con 'total')
            if 'total' in line:
                in_syscall_table = False
                # Añadir este bloque de resultados a la lista
                results.append(current_result)
                # Reiniciar el bloque actual para el siguiente conjunto de resultados
                current_result = {
                    'time_init_matrix': None,
                    'time_multiplication_matrix': None,
                    'syscall_stats': []
                }
                continue

            # Separar las columnas de la tabla de syscalls
            parts = line.split()
            if len(parts) >= 5:
                # Estructura: % time, seconds, usecs/call, calls, errors (opcional), syscall
                syscall_data = {
                    'percentage_time': float(parts[0].replace(',', '.')),
                    'seconds': float(parts[1].replace(',', '.')),
                    'usecs_per_call': int(parts[2]),
                    'calls': int(parts[3]),
                    'errors': int(parts[4]) if parts[4].isdigit() else 0,  # Verificar si hay columna de errores
                    'syscall': parts[5] if len(parts) > 5 else parts[4]   # El nombre de la syscall puede estar en la 5ta o 6ta posición
                }
                current_result['syscall_stats'].append(syscall_data)

    return results



def calculate_average_results(results):
    # Inicializar listas para los tiempos
    time_init_matrix_values = []
    time_multiplication_matrix_values = []
    
    # Inicializar un diccionario para almacenar los datos de las llamadas al sistema
    syscall_totals = {}

    # Recorrer los resultados
    for result in results:
        # Añadir los tiempos a las listas
        time_init_matrix_values.append(result['time_init_matrix'])
        time_multiplication_matrix_values.append(result['time_multiplication_matrix'])

        # Sumar los valores de las llamadas al sistema
        for syscall in result['syscall_stats']:
            syscall_name = syscall['syscall']
            if syscall_name not in syscall_totals:
                syscall_totals[syscall_name] = {
                    'percentage_time': [],
                    'seconds': [],
                    'usecs_per_call': [],
                    'calls': [],
                    'errors': []
                }
            syscall_totals[syscall_name]['percentage_time'].append(syscall['percentage_time'])
            syscall_totals[syscall_name]['seconds'].append(syscall['seconds'])
            syscall_totals[syscall_name]['usecs_per_call'].append(syscall['usecs_per_call'])
            syscall_totals[syscall_name]['calls'].append(syscall['calls'])
            syscall_totals[syscall_name]['errors'].append(syscall['errors'])

    # Calcular la media y desviación estándar de los tiempos
    avg_time_init_matrix = np.mean(time_init_matrix_values)
    std_time_init_matrix = np.std(time_init_matrix_values)

    avg_time_multiplication_matrix = np.mean(time_multiplication_matrix_values)
    std_time_multiplication_matrix = np.std(time_multiplication_matrix_values)

    # Calcular la media y desviación estándar de las llamadas al sistema
    avg_syscall_stats = {}
    for syscall_name, values in syscall_totals.items():
        avg_syscall_stats[syscall_name] = {
            'percentage_time': (np.mean(values['percentage_time']), np.std(values['percentage_time'])),
            'seconds': (np.mean(values['seconds']), np.std(values['seconds'])),
            'usecs_per_call': (np.mean(values['usecs_per_call']), np.std(values['usecs_per_call'])),
            'calls': (np.mean(values['calls']), np.std(values['calls'])),
            'errors': (int(np.mean(values['errors'])), np.std(values['errors']))  # Redondear a entero
        }

    return (
        (avg_time_init_matrix, std_time_init_matrix),
        (avg_time_multiplication_matrix, std_time_multiplication_matrix),
        avg_syscall_stats
    )


def write_results_to_file(file_path, avg_time_init_matrix, avg_time_multiplication_matrix, avg_syscall_stats):
    with open(file_path, 'a') as file:  # Usar 'a' para agregar al final del archivo
        file.write("\nResults Average (Mean and Standard Deviation):\n")
        # Escribir los tiempos promedio (5 decimales) junto con la desviación estándar
        avg_init, std_init = avg_time_init_matrix
        avg_mult, std_mult = avg_time_multiplication_matrix
        file.write(f"Matrix Initialization Time: {avg_init:.5f} (std: {std_init:.5f})\n")
        file.write(f"Matrix Multiplication Time: {avg_mult:.5f} (std: {std_mult:.5f})\n")
        
        # Escribir la tabla de llamadas al sistema con media y desviación estándar
        file.write(f"{'%-time (avg)':<15} {'seconds (avg)':<15} {'usecs/call (avg)':<20} "
                   f"{'calls (avg)':<15} {'errors (avg)':<15} syscall\n")
        file.write(f"{'%-time (std)':<15} {'seconds (std)':<15} {'usecs/call (std)':<20} "
                   f"{'calls (std)':<15} {'errors (std)':<15}\n")
        file.write("--------------  -------------   ----------------   --------------   --------------   ----------------\n")

        for syscall_name, avg_syscall in avg_syscall_stats.items():
            avg_percentage_time, std_percentage_time = avg_syscall['percentage_time']
            avg_seconds, std_seconds = avg_syscall['seconds']
            avg_usecs_per_call, std_usecs_per_call = avg_syscall['usecs_per_call']
            avg_calls, std_calls = avg_syscall['calls']
            avg_errors, std_errors = avg_syscall['errors']

            # Escribir la media con alineación
            file.write(f"{avg_percentage_time:>15.2f} {avg_seconds:>15.5f} {avg_usecs_per_call:>20.2f} "
                       f"{avg_calls:>15.2f} {avg_errors:>15.2f} {syscall_name}\n")
            
            # Escribir la desviación estándar con alineación
            file.write(f"({std_percentage_time:>13.2f}) ({std_seconds:>13.5f}) ({std_usecs_per_call:>18.2f}) "
                       f"({std_calls:>13.2f}) ({std_errors:>13.2f})\n")

        file.write("--------------  -------------   ----------------   --------------   --------------   ----------------\n")
        
        # Calcular el total para las medias
        total_seconds = sum([stat['seconds'][0] for stat in avg_syscall_stats.values()])
        total_calls = sum([stat['calls'][0] for stat in avg_syscall_stats.values()])
        total_errors = sum([stat['errors'][0] for stat in avg_syscall_stats.values()])
        
        file.write(f"{'100.00':>15} {total_seconds:>15.5f} {int(total_calls):>20} {int(total_errors):>15} {'total':<15}\n")


def detect_high_deviation(avg_time_init_matrix, avg_time_multiplication_matrix, avg_syscall_stats, threshold=0.9):
    # Inicializar los vectores de resultados
    names = []       # Para almacenar "ini_matrix", "mult_matrix" o el nombre de la syscall
    measures = []    # Para almacenar "time" o el nombre de la medida (stat_name)
    means = []       # Para almacenar la media
    stddevs = []     # Para almacenar la desviación estándar

    # Verificar desviación estándar alta en los tiempos de inicialización de matriz
    time_init_avg, time_init_std = avg_time_init_matrix
    if time_init_std / time_init_avg > threshold:
        names.append("ini_matrix")
        measures.append("time")
        means.append(time_init_avg)
        stddevs.append(time_init_std)

    # Verificar desviación estándar alta en los tiempos de multiplicación de matriz
    time_mult_avg, time_mult_std = avg_time_multiplication_matrix
    if time_mult_std / time_mult_avg > threshold:
        names.append("mult_matrix")
        measures.append("time")
        means.append(time_mult_avg)
        stddevs.append(time_mult_std)

    # Verificar desviación estándar alta en las llamadas al sistema
    for syscall_name, stats in avg_syscall_stats.items():
        for stat_name, (mean, stddev) in stats.items():
            if stat_name != "percentage_time" and mean > 0.9 and stddev / mean > threshold:
                names.append(syscall_name)
                measures.append(stat_name)
                means.append(mean)
                stddevs.append(stddev)

    return names, measures, means, stddevs




def plot_means_and_stddevs(names, measures, means, stddevs):
    # Crear etiquetas combinando nombres y medidas (ini_matrix, mult_matrix, syscall_name)
    labels = [f"{name} ({measure})" for name, measure in zip(names, measures)]

    # Convertir los datos a arrays de numpy para manipulación más fácil
    means = np.array(means)
    stddevs = np.array(stddevs)

    # Crear posiciones en el eje X
    x = np.arange(len(labels))

    # Crear la figura y el gráfico
    fig, ax = plt.subplots(figsize=(10, 6))

    # Crear barras con la media, y agregar barras de error (stddev)
    ax.bar(x, means, yerr=stddevs, capsize=5, color='lightblue', label='Media', align='center')

    # Añadir etiquetas y título
    ax.set_xlabel('Metric')
    ax.set_ylabel('Value (Mean and Standard Deviation)')
    ax.set_title('Comparative Metrics: Standard vs. Eigen Implementation')

    # Rotar etiquetas del eje X para mejor legibilidad
    ax.set_xticks(x)
    ax.set_xticklabels(labels, rotation=45, ha="right")

    # Añadir una leyenda
    ax.legend()

    # Mostrar el gráfico
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    FOLDER_PATH = 'results/'
    FOLDER_PATH_MY_MATRIX = FOLDER_PATH + 'my_matrix/'
    FOLDER_PATH_EIGEN = FOLDER_PATH + 'eigen/'
    WRITE_ON_FILE = False

    files_path = [ FOLDER_PATH_MY_MATRIX + 'ex3_strace_matrix.txt' , FOLDER_PATH_EIGEN + 'ex3_strace_eigen.txt' ]

    for file_path in files_path:
        # print(f"####################################################\n{file_path}\n####################################################\n")
        results = parse_multiple_results(file_path)
        avg_time_init_matrix, avg_time_multiplication_matrix, avg_syscall_stats = calculate_average_results(results)
        if WRITE_ON_FILE:
            write_results_to_file(file_path, avg_time_init_matrix, avg_time_multiplication_matrix, avg_syscall_stats)
        names, measures, means, stddevs = detect_high_deviation(avg_time_init_matrix, avg_time_multiplication_matrix, avg_syscall_stats)
        if names:
            plot_means_and_stddevs(names, measures, means, stddevs)
        else:
            print("No one high deviation was found in any metric.")