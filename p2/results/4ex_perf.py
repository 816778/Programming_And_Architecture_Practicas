import re

def parse_perf_output(file_path, num_repeticiones):
    # Inicializar listas para almacenar los tiempos de inicialización y multiplicación de matrices
    time_init_matrix = []
    time_multiplication_matrix = []

    # Inicializar un diccionario para almacenar las estadísticas de rendimiento y desviaciones estándar
    perf_stats = {
        'task_clock': {'values': [], 'stddev': []},
        'context_switches': {'values': [], 'stddev': []},
        'cpu_migrations': {'values': [], 'stddev': []},
        'page_faults': {'values': [], 'stddev': []},
        'cycles': {'values': [], 'stddev': []},
        'instructions': {'values': [], 'stddev': []},
        'branches': {'values': [], 'stddev': []},
        'branch_misses': {'values': [], 'stddev': []},
        'tma_backend_bound': {'values': [], 'stddev': []},
        'tma_bad_speculation': {'values': [], 'stddev': []},
        'tma_frontend_bound': {'values': [], 'stddev': []},
        'tma_retiring': {'values': [], 'stddev': []},
        'elapsed_time': {'values': [], 'stddev': []}
    }

    with open(file_path, 'r') as file:
        lines = file.readlines()

        # Regex para extraer las métricas de rendimiento y desviaciones estándar
        metric_with_stddev_regex = r'(\d+[\d.,]*)\s+.*\s+\( +- (\d+,\d+)% \)'  # Captura valor y desviación estándar en formato %

        task_clock_regex = r'(\d+,\d+)\s+msec task-clock'
        context_switches_regex = r'(\d+)\s+context-switches'
        cpu_migrations_regex = r'(\d+)\s+cpu-migrations'
        page_faults_regex = r'(\d+)\s+page-faults'
        cycles_regex = r'(\d[\d,.]*)\s+cycles'
        instructions_regex = r'(\d[\d,.]*)\s+instructions'
        branches_regex = r'(\d[\d,.]*)\s+branches'
        branch_misses_regex = r'(\d[\d,.]*)\s+branch-misses'
        elapsed_time_regex = r'(\d+,\d+)\s+seconds time elapsed'

        # TMA metrics regex (topdown metrics)
        tma_regex = r'(\d+,\d+)\s+%\s+tma_(\w+)'

        # Variables para controlar si estamos en la parte de los tiempos o en las métricas
        is_perf_section = False

        for line in lines:
            line = line.strip()

            # Leer los tiempos de inicialización y multiplicación de matrices
            try:
                if not is_perf_section:
                    time_value = float(line.replace(",", "."))
                    if len(time_init_matrix) < num_repeticiones:
                        time_init_matrix.append(time_value)
                    else:
                        time_multiplication_matrix.append(time_value)
                    continue
            except ValueError:
                pass  # Si no es un número, seguimos adelante

            # Cuando llegamos a la parte de las estadísticas de rendimiento (perf output)
            if 'Performance counter stats' in line:
                is_perf_section = True
                continue

            if is_perf_section:
                # Extraer métricas con regex
                if match := re.search(task_clock_regex, line):
                    perf_stats['task_clock']['values'].append(float(match.group(1).replace(",", ".")))
                    if stddev_match := re.search(r'\( +- (\d+,\d+)% \)', line):
                        perf_stats['task_clock']['stddev'].append(float(stddev_match.group(1).replace(",", ".")))
                elif match := re.search(context_switches_regex, line):
                    perf_stats['context_switches']['values'].append(int(match.group(1).replace(",", ".")))
                    if stddev_match := re.search(r'\( +- (\d+,\d+)% \)', line):
                        perf_stats['context_switches']['stddev'].append(float(stddev_match.group(1).replace(",", ".")))
                elif match := re.search(cpu_migrations_regex, line):
                    perf_stats['cpu_migrations']['values'].append(int(match.group(1).replace(",", ".")))
                    if stddev_match := re.search(r'\( +- (\d+,\d+)% \)', line):
                        perf_stats['cpu_migrations']['stddev'].append(float(stddev_match.group(1).replace(",", ".")))
                elif match := re.search(page_faults_regex, line):
                    perf_stats['page_faults']['values'].append(int(match.group(1).replace(",", ".")))
                    if stddev_match := re.search(r'\( +- (\d+,\d+)% \)', line):
                        perf_stats['page_faults']['stddev'].append(float(stddev_match.group(1).replace(",", ".")))
                elif match := re.search(cycles_regex, line):
                    perf_stats['cycles']['values'].append(float(match.group(1).replace(".", "").replace(",", ".")))
                    if stddev_match := re.search(r'\( +- (\d+,\d+)% \)', line):
                        perf_stats['cycles']['stddev'].append(float(stddev_match.group(1).replace(",", ".")))
                elif match := re.search(instructions_regex, line):
                    perf_stats['instructions']['values'].append(float(match.group(1).replace(".", "").replace(",", ".")))
                    if stddev_match := re.search(r'\( +- (\d+,\d+)% \)', line):
                        perf_stats['instructions']['stddev'].append(float(stddev_match.group(1).replace(",", ".")))
                elif match := re.search(branches_regex, line):
                    perf_stats['branches']['values'].append(float(match.group(1).replace(".", "").replace(",", ".")))
                    if stddev_match := re.search(r'\( +- (\d+,\d+)% \)', line):
                        perf_stats['branches']['stddev'].append(float(stddev_match.group(1).replace(",", ".")))
                elif match := re.search(branch_misses_regex, line):
                    perf_stats['branch_misses']['values'].append(float(match.group(1).replace(".", "").replace(",", ".")))
                    if stddev_match := re.search(r'\( +- (\d+,\d+)% \)', line):
                        perf_stats['branch_misses']['stddev'].append(float(stddev_match.group(1).replace(",", ".")))
                elif match := re.search(elapsed_time_regex, line):
                    perf_stats['elapsed_time']['values'].append(float(match.group(1).replace(",", ".")))
                    if stddev_match := re.search(r'\( +- (\d+,\d+)% \)', line):
                        perf_stats['elapsed_time']['stddev'].append(float(stddev_match.group(1).replace(",", ".")))
                elif match := re.search(tma_regex, line):
                    metric_type = match.group(2)  # backend_bound, bad_speculation, etc.
                    value = float(match.group(1).replace(",", "."))
                    if metric_type == 'backend_bound':
                        perf_stats['tma_backend_bound']['values'].append(value)
                    elif metric_type == 'bad_speculation':
                        perf_stats['tma_bad_speculation']['values'].append(value)
                    elif metric_type == 'frontend_bound':
                        perf_stats['tma_frontend_bound']['values'].append(value)
                    elif metric_type == 'retiring':
                        perf_stats['tma_retiring']['values'].append(value)

    return time_init_matrix, time_multiplication_matrix, perf_stats



FOLDER_PATH = 'results/'
FOLDER_PATH_MY_MATRIX = FOLDER_PATH + 'my_matrix/'
FOLDER_PATH_EIGEN = FOLDER_PATH + 'eigen/'

file_path = FOLDER_PATH_MY_MATRIX +  "ex4_perf_matrix.txt"
num_repeticiones = 10  # Número de repeticiones

time_init_matrix, time_multiplication_matrix, perf_stats = parse_perf_output(file_path, num_repeticiones)

# Imprimir los resultados
print("Tiempos de inicialización de matriz:", time_init_matrix)
print("Tiempos de multiplicación de matriz:", time_multiplication_matrix)
print("Estadísticas de rendimiento (perf):", perf_stats)
