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
    print("1. Real time tendencies:")
    real_increase = np.diff(real_times)
    print(f"Average increase in real time as matrix size increases: {np.mean(real_increase):.4f} seconds")
    print(f"Maximum real time recorded: {np.max(real_times):.4f} seconds for a matrix of size {size[np.argmax(real_times)]}")

    print("\n2. User time tendencies:")
    user_increase = np.diff(user_times)
    print(f"Average increase in user time as matrix size increases: {np.mean(user_increase):.4f} seconds")
    print(f"Maximum user time recorded: {np.max(user_times):.4f} seconds for a matrix of size {size[np.argmax(user_times)]}")

    print("\n3. System time tendencies:")
    if all(t == 0 for t in sys_times):
        print("All system times are zero. This could be due to the system not consuming system resources.")
    else:
        print(f"Maximum system time recorded: {np.max(sys_times):.4f} seconds for a matrix of size {size[np.argmax(sys_times)]}")

    # Análisis de la desviación estándar
    print("\n4. Standard deviation analysis:")
    print(f"Average standard deviation of real time: {np.mean(stddev_real):.4f} seconds")
    print(f"Maximum standard deviation of real time: {np.max(stddev_real):.4f} seconds for a matrix of size {size[np.argmax(stddev_real)]}")
    
    if np.max(stddev_real) > 0.5:
        print("Disclaimer: The standard deviation of real time is high for some matrices. This could be due to external interferences (OS loading, background processes, etc.) or lack of algorithm optimization.")
    
    print(f"Average standard deviation of user time: {np.mean(stddev_user):.4f} seconds")
    print(f"Average standard deviation of system time: {np.mean(stddev_sys):.4f} seconds")


# Función para graficar los datos
def plot_data(size, real_times, user_times, sys_times, stddev_real, stddev_user, stddev_sys):
    plt.figure(figsize=(10, 6))

    # Graficar tiempos medios
    plt.plot(size, real_times, label="Real time (mean_real)", marker='o')
    plt.plot(size, user_times, label="User time (mean_user)", marker='o')
    plt.plot(size, sys_times, label="System time (mean_sys)", marker='o')

    # Graficar desviaciones estándar
    plt.errorbar(size, real_times, yerr=stddev_real, fmt='o', label="Stddev Real time", alpha=0.3)
    plt.errorbar(size, user_times, yerr=stddev_user, fmt='o', label="Stddev User time", alpha=0.3)

    plt.xlabel('Matrix size')
    plt.ylabel('Time (seconds)')
    plt.title('Time comparison')
    plt.legend()
    plt.grid(True)

    plt.show()


def plot_data_comparison(file_name_1, file_name_2, label1="Manual implementation", label2="Eigen"):
    size1, real_times1, user_times1, sys_times1, stddev_real1, stddev_user1, stddev_sys1 = read_times_from_file(file_name_1)
    size2, real_times2, user_times2, sys_times2, stddev_real2, stddev_user2, stddev_sys2 = read_times_from_file(file_name_2)

    plt.plot(size1, real_times1, label=f"Real time {label1}", marker='o', color='b')
    plt.plot(size1, user_times1, label=f"User time {label1}", marker='s', color='b')
    plt.plot(size1, sys_times1, label=f"System time {label1}", marker='^', color='b')

    plt.errorbar(size1, real_times1, yerr=stddev_real1, fmt='o', color='c', alpha=0.3, label=f"Stddev Real time {label1}")
    plt.errorbar(size1, user_times1, yerr=stddev_user1, fmt='s', color='c', alpha=0.3, label=f"Stddev User time {label1}")

    plt.plot(size2, real_times2, label=f"Real time {label2}", marker='o', color='r')
    plt.plot(size2, user_times2, label=f"User time {label2}", marker='s', color='r')
    plt.plot(size2, sys_times2, label=f"System time {label2}", marker='^', color='r')

    plt.errorbar(size2, real_times2, yerr=stddev_real2, fmt='o', color='m', alpha=0.3, label=f"Stddev Real time {label2}")
    plt.errorbar(size2, user_times2, yerr=stddev_user2, fmt='s', color='m', alpha=0.3, label=f"Stddev User time {label2}")

    plt.xlabel('Matrix size')
    plt.ylabel('Time (seconds)')
    plt.title('Time comparison between manual and Eigen implementations')
    plt.legend()
    plt.grid(True)
    plt.show()

def plot_data_comparison_3(file_name_1, file_name_2, file_name_3, label1="Manual implementation", label2="Manual implementation 2", label3="Eigen"):
    size1, real_times1, user_times1, sys_times1, stddev_real1, stddev_user1, stddev_sys1 = read_times_from_file(file_name_1)
    size2, real_times2, user_times2, sys_times2, stddev_real2, stddev_user2, stddev_sys2 = read_times_from_file(file_name_2)
    size3, real_times3, user_times3, sys_times3, stddev_real3, stddev_user3, stddev_sys3 = read_times_from_file(file_name_3)

    plt.plot(size1, real_times1, label=f"Real time {label1}", marker='o', color='b')
    plt.plot(size1, user_times1, label=f"User time {label1}", marker='s', color='b')
    plt.plot(size1, sys_times1, label=f"System time {label1}", marker='^', color='b')

    plt.errorbar(size1, real_times1, yerr=stddev_real1, fmt='o', color='c', alpha=0.3, label=f"Stddev Real time {label1}")
    plt.errorbar(size1, user_times1, yerr=stddev_user1, fmt='s', color='c', alpha=0.3, label=f"Stddev User time {label1}")

    plt.plot(size2, real_times2, label=f"Real time {label2}", marker='o', color='g')
    plt.plot(size2, user_times2, label=f"User time {label2}", marker='s', color='g')
    plt.plot(size2, sys_times2, label=f"System time {label2}", marker='^', color='g')

    plt.errorbar(size2, real_times2, yerr=stddev_real2, fmt='o', color='g', alpha=0.3, label=f"Stddev Real time {label2}")
    plt.errorbar(size2, user_times2, yerr=stddev_user2, fmt='s', color='g', alpha=0.3, label=f"Stddev User time {label2}")

    plt.plot(size3, real_times3, label=f"Real time {label3}", marker='o', color='r')
    plt.plot(size3, user_times3, label=f"User time {label3}", marker='s', color='r')
    plt.plot(size3, sys_times3, label=f"System time {label3}", marker='^', color='r')

    plt.errorbar(size3, real_times3, yerr=stddev_real3, fmt='o', color='m', alpha=0.3, label=f"Stddev Real time {label3}")
    plt.errorbar(size3, user_times3, yerr=stddev_user3, fmt='s', color='m', alpha=0.3, label=f"Stddev User time {label3}")

    plt.xlabel('Matrix size')
    plt.ylabel('Time (seconds)')
    plt.title('Time comparison between manual and Eigen implementations')
    plt.legend()
    plt.grid(True)
    plt.show()


if __name__ == "__main__":

    plot_data_comparison_3('results/time_matrix.txt','results/time_matrix_2.txt', 'results/time_matrix_eg.txt')
    file_names = ['results/time_matrix.txt', 'results/time_matrix_2.txt', 'results/time_matrix_eg.txt']

    for file_name in file_names:
        print(file_name)
        size, real_times, user_times, sys_times, stddev_real, stddev_user, stddev_sys = read_times_from_file(file_name)
        analyze_data(size, real_times, user_times, sys_times, stddev_real, stddev_user, stddev_sys)
        plot_data(size, real_times, user_times, sys_times, stddev_real, stddev_user, stddev_sys)
        print("####################################################################################################\n")
