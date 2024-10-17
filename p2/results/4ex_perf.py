import numpy as np
import matplotlib.pyplot as plt


my_matrix = {
    "cycles": 85601614421,
    "context-switches": 830,
    "cpu_migrations": 22,
    "page_faults": 13331,
    "instructions": 187518885545,
    "branches": 3605133376,
    "branch_misses": 7024562,
}

eigen_matrix = {
    "cycles": 2151254227,
    "context-switches": 27,
    "cpu_migrations": 1,
    "page_faults": 15711,
    "instructions": 6194268590,
    "branches": 100932570,
    "branch_misses": 4628960,
}

def plot_comparative_bars(my_matrix, eigen_matrix):
    # Listas para almacenar las métricas donde hay más de 50% de diferencia
    metrics = []
    my_values = []
    eigen_values = []
    differences = []  # Para almacenar los porcentajes de diferencia

    # Recorrer las métricas y comparar
    for key in my_matrix:
        if key in eigen_matrix:  # Asegurarse de que la métrica exista en ambos diccionarios
            my_value = my_matrix[key]
            eigen_value = eigen_matrix[key]
            
            # Evitar divisiones por cero
            if my_value == 0 or eigen_value == 0:
                continue
            
            # Calcular el porcentaje de diferencia relativa
            diff_percentage = abs(my_value - eigen_value) / min(my_value, eigen_value)
            print(diff_percentage)
            if diff_percentage > 0.9:  # Si la diferencia es mayor del 50%
                metrics.append(key)
                my_values.append(my_value)
                eigen_values.append(eigen_value)
                differences.append(diff_percentage * 100)  # En porcentaje

    # Si no hay diferencias significativas, retornar sin hacer nada
    if not metrics:
        print("No hay métricas con más del 90% de diferencia.")
        return
    
    # Crear gráfico de barras
    x = np.arange(len(metrics))  # Etiquetas en el eje X
    width = 0.35  # Ancho de las barras

    fig, ax = plt.subplots(figsize=(10, 6))

    # Crear barras
    bars_my = ax.bar(x - width/2, my_values, width, label='my_matrix', color='b')
    bars_eigen = ax.bar(x + width/2, eigen_values, width, label='eigen_matrix', color='g')

    # Añadir etiquetas y título
    ax.set_xlabel('Metrics')
    ax.set_ylabel('Values')
    ax.set_title('Comparation my matrix and eigen_matrix (difference > 90%)')
    ax.set_xticks(x)
    ax.set_xticklabels(metrics, rotation=45, ha="right")

    # Añadir leyenda
    ax.legend()
    # Mostrar el gráfico
    plt.tight_layout()
    plt.show()


def normalize_matrices(my_matrix, eigen_matrix):
    # Diccionarios normalizados
    my_matrix_normalized = {}
    eigen_matrix_normalized = {}

    # Combinar las métricas de ambos diccionarios
    combined_keys = set(my_matrix.keys()).union(set(eigen_matrix.keys()))

    # Normalizar cada métrica
    for key in combined_keys:
        # Obtener valores de my_matrix y eigen_matrix, si no existen, usar 0
        my_value = my_matrix.get(key, 0)
        eigen_value = eigen_matrix.get(key, 0)

        # Encontrar el mínimo y máximo para normalización
        min_value = min(my_value, eigen_value)
        max_value = max(my_value, eigen_value)
        # Si min_value == max_value, significa que ambos valores son iguales, normalizamos a 1
        if max_value == min_value:
            my_matrix_normalized[key] = 1
            eigen_matrix_normalized[key] = 1
        else:
            if max_value == my_value:
                my_matrix_normalized[key] = 1
                eigen_matrix_normalized[key] = eigen_value / my_value
            else:
                eigen_matrix_normalized[key] = 1
                my_matrix_normalized[key] = my_value / eigen_value
    return my_matrix_normalized, eigen_matrix_normalized


def plot_grafic():
    # Preparar los datos para el gráfico
    metrics = list(my_matrix.keys())
    my_values = list(my_matrix.values())
    eigen_values = list(eigen_matrix.values())

    x = np.arange(len(metrics))  # Índices para las métricas
    width = 0.4  # Ancho de las barras

    # Crear el gráfico
    fig, ax = plt.subplots(figsize=(12, 6))

    # Gráfico de barras con escala logarítmica
    ax.bar(x - width/2, my_values, width, label='My Implementation', color='b', log=True)
    ax.bar(x + width/2, eigen_values, width, label='Eigen Implementation', color='g', log=True)

    # Añadir etiquetas y leyenda
    ax.set_xlabel('Metric')
    ax.set_ylabel('Value (Log Scale)')
    ax.set_title('Comparative Metrics: Standard vs. Eigen Implementation')
    ax.set_xticks(x)
    ax.set_xticklabels(metrics, rotation=45, ha='right')
    ax.legend()

    # Añadir valores encima de las barras (opcional)
    for i in range(len(metrics)):
        ax.text(x[i] - width/2, my_values[i], f'{my_values[i]:.0e}', ha='center', va='bottom', color='blue', fontsize=9)
        ax.text(x[i] + width/2, eigen_values[i], f'{eigen_values[i]:.0e}', ha='center', va='bottom', color='green', fontsize=9)

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    plot_grafic()
    exit()
    my_matrix, eigen_matrix = normalize_matrices(my_matrix, eigen_matrix)
    print(my_matrix)
    print(eigen_matrix)
    plot_comparative_bars(my_matrix, eigen_matrix)