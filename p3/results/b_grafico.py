import pandas as pd
import matplotlib.pyplot as plt

RESULTS_FOLDER = "results/"

def plot_balance_general(path_csv):
    # Cargar los datos del archivo CSV
    

    # Crear un gráfico de barras con el tiempo de ejecución de cada hilo
    plt.bar(data['thread_id'], data['execution_time'], color='skyblue')

    # Añadir etiquetas y título
    plt.xlabel('Thread ID')
    plt.ylabel('Execution Time (seconds)')
    plt.title('Thread Execution Time')

    # Mostrar el gráfico
    plt.show()


def plot_init_end_times(data):
    data['end_time'] =  data['end_time'] - data['start_time']
    data['start_time'] = 0

    # Crear el gráfico de Gantt
    plt.barh(data['thread_id'], data['end_time'] - data['start_time'], left=data['start_time'], color='lightgreen')

    # Añadir etiquetas y título
    plt.xlabel('Time (nanoseconds)')
    plt.ylabel('Thread ID')
    plt.title('Thread Execution Timeline (Gantt Chart)')

    # Mostrar el gráfico
    plt.show()

if __name__ == "__main__":
    paths_csv = [RESULTS_FOLDER + 'ex_β_4.csv', RESULTS_FOLDER + 'ex_β_nucleo.csv']

    for path_csv in paths_csv:
        data = pd.read_csv(path_csv)
        plot_balance_general(data)
        plot_init_end_times(data)
    