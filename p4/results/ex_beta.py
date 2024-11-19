import csv
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt


RESULTS_FOLDER="results/"

def plot_surface(data, remove_largest=False):

    if remove_largest:
        # Sort the data by time_ms in descending order and remove the two largest entries.
        data = sorted(data, key=lambda x: x["tiempo_ms"], reverse=True)[3:]

    # Convert the data to NumPy arrays.
    ranges = np.array([entry['rango'] for entry in data])
    threads = np.array([entry['threads'] for entry in data])
    times = np.array([entry['tiempo_ms'] for entry in data])

    # Create a 3D figure and axis.
    fig = plt.figure(figsize=(10, 7))
    ax = fig.add_subplot(111, projection='3d')

    # Create the 3D surface.
    surf = ax.plot_trisurf(threads, ranges, times, cmap='viridis', edgecolor='none')

    # Tags and title.
    ax.set_xlabel('Number of Threads')
    ax.set_ylabel('Prime Search Range')
    ax.set_zlabel('Execution Time (ms)')
    ax.set_title('Execution Time vs. Prime Search Range and Number of Threads')
    
    # Add a color bar which maps values to colors, and show the plot.
    fig.colorbar(surf, shrink=0.5, aspect=5)
    plt.show()


def plot_threads_vs_time(time_data):

    # Get for number of threads and runtime.
    threads = [entry['threads'] for entry in time_data]
    times = [entry['tiempo_ms'] for entry in time_data]
    
    # Create the plot.
    plt.figure(figsize=(10, 6))
    plt.plot(threads, times, marker='o', linestyle='-', color='b', label='Execution Time')
    
    # Tags and title.
    plt.xlabel('Number of Threads')
    plt.ylabel('Execution Time (ms)')
    plt.title('Execution Time vs. Number of Threads')
    plt.legend()
    plt.grid(True)
    
    # Show the graphic.
    plt.show()


def plot_heatmap(data, remove_largest=True):
    if remove_largest:
        data = sorted(data, key=lambda x: x["tiempo_ms"], reverse=True)[1:]

    df = pd.DataFrame(data)
    heatmap_data = df.pivot("rango", "threads", "tiempo_ms")

    # Create the heatmap.
    plt.figure(figsize=(10, 7))
    sns.heatmap(heatmap_data, cmap="YlGnBu", annot=True, fmt=".1f", cbar_kws={'label': 'Execution Time (ms)'})
    
    # Tags and title.
    plt.xlabel('Number of Threads')
    plt.ylabel('Prime Search Range')
    plt.title('Execution Time Heatmap: Prime Search Range vs. Threads')
    plt.show()



def read_time_data(filename):
    time_data = []
    with open(filename, 'r') as file:
        reader = csv.reader(file)
        for row in reader:
            # Convert each value to an integer and store in a dictionary.
            range_ = int(row[0])
            threads = int(row[1])
            time_ms = int(row[2])
            # Add a dictionary for each row in the list.
            time_data.append({
                'range_': range_,
                'threads': threads,
                'time_ms': time_ms
            })
    return time_data



def calculate_speedup(data):
    baseline_times = {}
    for entry in data:
        if entry['threads'] == 1:  # Only one thread (T1) for each range.
            rango = entry['range_']
            baseline_times[rango] = entry['time_ms']

    for entry in data:
        rango = entry['range_']
        tiempo_con_hilos = entry['time_ms']
        # Compute speedup using the base time (T1) for that range.
        entry['speedup'] = baseline_times[rango] / tiempo_con_hilos

    return data



def plot_speedup(data):

    # Get the unique range values.
    unique_ranges = sorted(set(entry['range_'] for entry in data))

    plt.figure(figsize=(10, 6))

    # Graph the speedup for each range.
    for range_ in unique_ranges:
        # Filter data for the current range.
        threads = [entry['threads'] for entry in data if entry['range_'] == range_]
        speedups = [entry['speedup'] for entry in data if entry['range_'] == range_]

        # Graph the current range.
        plt.plot(threads, speedups, marker='o', label=f'Range {range_}')

    # Tags and title.
    plt.xlabel('Number of Threads')
    plt.ylabel('Speedup')
    plt.title('Speedup vs. Number of Threads for Different Ranges')
    plt.legend()
    plt.grid(True)

    # Show the plot.
    plt.show()


if __name__ == "__main__":

    # File names with the data.
    filename = RESULTS_FOLDER + 'ex_beta.txt'
    filename2 = RESULTS_FOLDER + 'ex_beta_speedup.txt'

    time_data = read_time_data(filename2)
    # plot_threads_vs_time(time_data)
    # plot_heatmap(time_data)
    plot_speedup(calculate_speedup(time_data))

