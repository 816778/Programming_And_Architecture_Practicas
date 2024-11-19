import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np


RESULTS_FOLDER="results/"


def plot_time_vs_regions(time_data):
    # Extract data from time_data.
    x = [entry["time_ms"] for entry in time_data]  # Execution time in milliseconds.
    y = [entry["column"] * entry["row"] for entry in time_data]  # Number of regions (columns * rows).
    
    # Create the plot.
    plt.figure(figsize=(10, 6))
    plt.scatter(x, y, color='blue', marker='o', label="Execution Time vs. Number of Regions")
    plt.plot(x, y, linestyle='--', color='gray', alpha=0.5)  # Add line to connect points.

    # Labels and title.
    plt.xlabel("Execution Time (ms)")
    plt.ylabel("Number of Regions (columns * rows)")
    plt.title("Relationship between Execution Time and Number of Image Regions")
    plt.legend()
    plt.grid(True)
    plt.show()


def read_time_data(filename):
    data = []
    with open(filename, 'r') as file:
        for line in file:
            # Remove spaces and line breaks at the end of each line.
            line = line.strip()
            # Split the line into three parts: column, row, and time.
            column, row, time_ms = line.split(',')
            # Convert the values to integers and store them in a dictionary.
            data.append({
                "column": int(column),
                "row": int(row),
                "time_ms": int(time_ms)
            })
    return data



def plot_3d_time_mesh(time_data, remove_largest=False):

    if remove_largest:
        # Sort the data by time_ms in descending order and remove the two largest entries.
        time_data = sorted(time_data, key=lambda x: x["time_ms"], reverse=True)[3:]

    # Get the data from the time_data.
    columns = np.array([entry["column"] for entry in time_data])
    rows = np.array([entry["row"] for entry in time_data])
    times = np.array([entry["time_ms"] for entry in time_data])

    # Create a 3D figure and axis.
    fig = plt.figure(figsize=(10, 7))
    ax = fig.add_subplot(111, projection='3d')

    # Grapf the mesh.
    ax.plot_trisurf(columns, rows, times, cmap="viridis", edgecolor='none')

    # Tags and title.
    ax.set_xlabel("Columns")
    ax.set_ylabel("Rows")
    ax.set_zlabel("Time (ms)")
    ax.set_title("Execution Time as a Function of Columns and Rows")
    # ax.set_zlim(3000, 8000)

    # Show the plot.
    plt.show()




if __name__ == "__main__":
    filename = RESULTS_FOLDER + 'ex_3.txt'

    time_data = read_time_data(filename)
    plot_3d_time_mesh(time_data)
    plot_time_vs_regions(time_data)

