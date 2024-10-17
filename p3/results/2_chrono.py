import matplotlib.pyplot as plt

RESULTS_FOLDER = "results/"

def extract_data(filename):
    x_values = []
    times = []

    with open(filename, 'r') as file:
        for line in file:
            parts = line.strip().split(',')
            
            # Convertir la primera parte a entero (x_value, que puede ser steps o threads) y la segunda a float (time)
            x_value = int(parts[0])
            time = float(parts[1])

            # Agregar los valores a las listas
            x_values.append(x_value)
            times.append(time)

    return x_values, times

def analyze_scalability(x_values, times, x_label="Steps"):
    print(f"{x_label} Ratio\tTime Ratio")
    
    for i in range(1, len(x_values)):
        x_ratio = x_values[i] / x_values[i - 1]
        time_ratio = times[i] / times[i - 1]
        print(f"{x_ratio:.2f}\t\t{time_ratio:.2f}")

def plot_data_vs_time(x_values, times, x_label="Steps", log=False):
    plt.figure(figsize=(10, 6))
    plt.plot(x_values, times, marker='o', linestyle='-', color='r')

    # Configurar etiquetas y título de manera dinámica
    plt.xlabel(x_label)
    plt.ylabel('Time (seconds)')
    plt.title(f'Execution Time vs {x_label}')
    
    if log:
        plt.yscale('log')
    
    plt.xscale('log')  
    plt.grid(True, which="both", linestyle='--', linewidth=0.5)
    plt.show()

if __name__ == "__main__":
    filename = RESULTS_FOLDER + '4_execution_times.txt'
    
    # Cambia el x_label a "Threads" si se está graficando en función de los threads
    x_label = "Threads"  # o "Threads" o Steps
    
    x_values, times = extract_data(filename)
    analyze_scalability(x_values, times, x_label=x_label)
    plot_data_vs_time(x_values, times, x_label=x_label)
