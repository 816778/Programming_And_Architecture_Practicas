#!/bin/bash

# Example command:
# ./execute.sh 1 1000 2 1003 50

# Salida
# size  mean_real  mean_user  mean_sys  stddev_real  stddev_user  stddev_sys


# Verifica si el número correcto de argumentos ha sido pasado
if [ "$#" -ne 5 ]; then
    echo "Uso: $0 <min_value> <max_value> <size_inicial> <size_final> <incremento>"
    exit 1
fi

# Asignar los argumentos a variables
min_value=$1
max_value=$2
size_inicial=$3
size_final=$4
incremento=$5

# Número de ejecuciones por experimento
n_runs=10  # Cambiar según necesidad

FOLDER_EXE="executable"
FOLDER_RESULT="results"

echo "Compilando ficheros..."

# Compilar los programas de matrices
g++ src/matrix.cpp -o $FOLDER_EXE/matrix
g++ src/matrix.cpp -o $FOLDER_EXE/2_matrix
g++ -O2 -Wall -I p1/eigen-3.4.0/ src/matrix_eigen.cpp -o $FOLDER_EXE/eigen_matrix

# Archivos para guardar los tiempos de ejecución
output_file="$FOLDER_RESULT/time_matrix.txt"
output_file_2="$FOLDER_RESULT/time_matrix_2.txt"
output_file_3="$FOLDER_RESULT/time_matrix_eg.txt"

# Limpiar los archivos de salida y añadir encabezados
echo "size  mean_real  mean_user  mean_sys  stddev_real  stddev_user  stddev_sys" > $output_file
echo "size  mean_real  mean_user  mean_sys  stddev_real  stddev_user  stddev_sys" > $output_file_2
echo "size  mean_real  mean_user  mean_sys  stddev_real  stddev_user  stddev_sys" > $output_file_3

echo "Ejecutando multiplicaciones..."

# Función para calcular la media
calculate_mean() {
    local total=0
    for val in "$@"; do
        total=$(echo "$total + $val" | bc)
    done
    local mean=$(echo "scale=6; $total / $n_runs" | bc)
    echo "$mean"
}

# Función para calcular la desviación estándar
calculate_stddev() {
    local mean=$1
    shift
    local total=0
    for val in "$@"; do
        diff=$(echo "$val - $mean" | bc)
        total=$(echo "$total + ($diff * $diff)" | bc)
    done
    local variance=$(echo "scale=6; $total / $n_runs" | bc)
    local stddev=$(echo "scale=6; sqrt($variance)" | bc)
    echo "$stddev"
}

# Función para ejecutar un programa n veces y obtener los tiempos
run_experiment() {
    local exe=$1
    local size=$2
    local min_value=$3
    local max_value=$4
    local output_file=$5

    real_times=()
    user_times=()
    sys_times=()

    # Ejecutar n veces
    for (( run=1; run<=n_runs; run++ ))
    do
        { output=$(/usr/bin/time -f "%e %U %S" $exe $size $min_value $max_value 2>&1 >/dev/null); } 2>/dev/null
        real_time=$(echo $output | awk '{print $1}')
        user_time=$(echo $output | awk '{print $2}')
        sys_time=$(echo $output | awk '{print $3}')

        real_times+=($real_time)
        user_times+=($user_time)
        sys_times+=($sys_time)
    done

    # Calcular medias y desviaciones estándar
    mean_real=$(calculate_mean "${real_times[@]}")
    mean_user=$(calculate_mean "${user_times[@]}")
    mean_sys=$(calculate_mean "${sys_times[@]}")

    stddev_real=$(calculate_stddev $mean_real "${real_times[@]}")
    stddev_user=$(calculate_stddev $mean_user "${user_times[@]}")
    stddev_sys=$(calculate_stddev $mean_sys "${sys_times[@]}")

    # Escribir resultados
    echo "$size  $mean_real  $mean_user  $mean_sys  $stddev_real  $stddev_user  $stddev_sys" >> $output_file
}

# Bucle sobre los tamaños de las matrices
for (( size=$size_inicial; size<=$size_final; size+=$incremento ))
do
    echo "Size: $size..."
    run_experiment "./$FOLDER_EXE/matrix" $size $min_value $max_value $output_file
    run_experiment "./$FOLDER_EXE/2_matrix" $size $min_value $max_value $output_file_2
    run_experiment "./$FOLDER_EXE/eigen_matrix" $size $min_value $max_value $output_file_3
done

echo "Ejecución completa. Resultados guardados en archivos .txt."
