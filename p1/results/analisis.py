import numpy as np
import matplotlib.pyplot as plt

# Función para leer el archivo con los datos
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

            if len(parts) == 7:
                size.append(int(parts[0]))
                real_times.append(float(parts[1]))
                user_times.append(float(parts[2]))
                sys_times.append(float(parts[3]))
                stddev_real_times.append(float(parts[4]))
                stddev_user_times.append(float(parts[5]))
                stddev_sys_times.append(float(parts[6]))

    return size, real_times, user_times, sys_times, stddev_real_times, stddev_user_times, stddev_sys_times


# Función para hacer el análisis de los datos
def analyze_data(size, real_times, user_times, sys_times, stddev_real, stddev_user, stddev_sys):
    # Análisis de tendencias en los tiempos de ejecución (mean_real, mean_user, mean_sys)
    print("1. Tendencias de tiempo real:")
    real_increase = np.diff(real_times)
    print(f"Incremento promedio en tiempo real a medida que aumenta el tamaño de la matriz: {np.mean(real_increase):.4f} segundos")
    print(f"Máximo tiempo real registrado: {np.max(real_times):.4f} segundos para una matriz de tamaño {size[np.argmax(real_times)]}")

    print("\n2. Tendencias de tiempo de usuario:")
    user_increase = np.diff(user_times)
    print(f"Incremento promedio en tiempo de usuario: {np.mean(user_increase):.4f} segundos")
    print(f"Máximo tiempo de usuario registrado: {np.max(user_times):.4f} segundos para una matriz de tamaño {size[np.argmax(user_times)]}")

    print("\n3. Tiempos de sistema:")
    if all(t == 0 for t in sys_times):
        print("El tiempo de sistema es 0 en todos los casos, lo que indica que el programa no está usando recursos de sistema intensivamente.")
    else:
        print(f"Máximo tiempo de sistema registrado: {np.max(sys_times):.4f} segundos para una matriz de tamaño {size[np.argmax(sys_times)]}")

    # Análisis de la desviación estándar
    print("\n4. Desviación estándar:")
    print(f"Desviación estándar promedio del tiempo real: {np.mean(stddev_real):.4f} segundos")
    print(f"Desviación estándar máxima del tiempo real: {np.max(stddev_real):.4f} segundos para una matriz de tamaño {size[np.argmax(stddev_real)]}")
    
    if np.max(stddev_real) > 0.5:
        print("Nota: La desviación estándar del tiempo real es alta para algunas matrices. Esto podría deberse a interferencias externas (carga del sistema operativo, procesos en segundo plano, etc.) o a la falta de optimización del algoritmo.")
    
    print(f"Desviación estándar promedio del tiempo de usuario: {np.mean(stddev_user):.4f} segundos")
    print(f"Desviación estándar máxima del tiempo de usuario: {np.max(stddev_user):.4f} segundos")


# Función para graficar los datos
def plot_data(size, real_times, user_times, sys_times, stddev_real, stddev_user, stddev_sys):
    plt.figure(figsize=(10, 6))

    # Graficar tiempos medios
    plt.plot(size, real_times, label="Tiempo Real (mean_real)", marker='o')
    plt.plot(size, user_times, label="Tiempo de Usuario (mean_user)", marker='o')
    plt.plot(size, sys_times, label="Tiempo de Sistema (mean_sys)", marker='o')

    # Graficar desviaciones estándar
    plt.errorbar(size, real_times, yerr=stddev_real, fmt='o', label="Stddev Tiempo Real", alpha=0.3)
    plt.errorbar(size, user_times, yerr=stddev_user, fmt='o', label="Stddev Tiempo de Usuario", alpha=0.3)

    plt.xlabel('Tamaño de la Matriz (size)')
    plt.ylabel('Tiempo (segundos)')
    plt.title('Comparación de Tiempos')
    plt.legend()
    plt.grid(True)

    plt.show()


def plot_data_comparison(file_name_1, file_name_2, label1="Mi implementacion", label2="Eigen"):
    size1, real_times1, user_times1, sys_times1, stddev_real1, stddev_user1, stddev_sys1 = read_times_from_file(file_name_1)
    size2, real_times2, user_times2, sys_times2, stddev_real2, stddev_user2, stddev_sys2 = read_times_from_file(file_name_2)

    plt.plot(size1, real_times1, label=f"Tiempo Real {label1}", marker='o', color='b')
    plt.plot(size1, user_times1, label=f"Tiempo Usuario {label1}", marker='s', color='b')
    plt.plot(size1, sys_times1, label=f"Tiempo Sistema {label1}", marker='^', color='b')

    plt.errorbar(size1, real_times1, yerr=stddev_real1, fmt='o', color='c', alpha=0.3, label=f"Stddev Tiempo Real {label1}")
    plt.errorbar(size1, user_times1, yerr=stddev_user1, fmt='s', color='c', alpha=0.3, label=f"Stddev Tiempo Usuario {label1}")

    plt.plot(size2, real_times2, label=f"Tiempo Real {label2}", marker='o', color='r')
    plt.plot(size2, user_times2, label=f"Tiempo Usuario {label2}", marker='s', color='r')
    plt.plot(size2, sys_times2, label=f"Tiempo Sistema {label2}", marker='^', color='r')

    plt.errorbar(size2, real_times2, yerr=stddev_real2, fmt='o', color='m', alpha=0.3, label=f"Stddev Tiempo Real {label2}")
    plt.errorbar(size2, user_times2, yerr=stddev_user2, fmt='s', color='m', alpha=0.3, label=f"Stddev Tiempo Usuario {label2}")

    plt.xlabel('Tamaño de la Matriz (size)')
    plt.ylabel('Tiempo (segundos)')
    plt.title('Comparación de Tiempos de Ejecución entre Archivos')
    plt.legend()
    plt.grid(True)
    plt.show()


if __name__ == "__main__":

    plot_data_comparison('results/time_matrix.txt', 'results/time_matrix_eg.txt')
    file_names = ['results/time_matrix.txt', 'results/time_matrix_2.txt', 'results/time_matrix_eg.txt']

    for file_name in file_names:
        print(file_name)
        size, real_times, user_times, sys_times, stddev_real, stddev_user, stddev_sys = read_times_from_file(file_name)
        analyze_data(size, real_times, user_times, sys_times, stddev_real, stddev_user, stddev_sys)
        plot_data(size, real_times, user_times, sys_times, stddev_real, stddev_user, stddev_sys)
        print("####################################################################################################\n")
