#!/bin/bash

# Example command:
# ./execute_3.sh 1 10000 1000 1

# Salida
# <time_init_matrix_1>
# <time_multiplication_matrix_1>
# ....
# <time_init_matrix_num_repeticiones>
# <time_multiplication_matrix_num_repeticiones>
# % time     seconds  usecs/call     calls    errors syscall
# 
# Performance counter stats for './executable/eigen_matrix 1500 1 10000' (10 runs):
#            545,97 msec task-clock                       #    0,997 CPUs utilized               ( +-  0,68% )

FOLDER_EXE="executable"
FOLDER_RESULT="results"
FOLDER_RESULT_MY_MATRIX="$FOLDER_RESULT/my_matrix"
FOLDER_RESULT_EIGEN="$FOLDER_RESULT/eigen"


if [ "$#" -ne 4 ]; then
    echo "Uso: $0 <min_value> <max_value> <N> <num_repeticiones>"
    exit 1
fi

# Asignar los argumentos a variables
min_value=$1
max_value=$2
N=$3
num_repeticiones=$4

echo "Compilando ficheros..."
g++ src/matrix.cpp -o $FOLDER_EXE/matrix
g++ -O2 -Wall -I p2/eigen-3.4.0/ src/matrix_eigen.cpp -o $FOLDER_EXE/eigen_matrix


output_file="$FOLDER_RESULT_MY_MATRIX/ex4_perf_matrix.txt"
output_file_e="$FOLDER_RESULT_EIGEN/ex4_perf_eigen.txt"

echo "Ejecutando..."
> $output_file
> $output_file_e


# (strace -c ./$FOLDER_EXE/matrix $N $min_value $max_value >> $output_file) 2>> $output_file
# (strace -c ./$FOLDER_EXE/eigen_matrix $N $min_value $max_value >> $output_file_e) 2>> $output_file_e
(sudo perf stat -r $num_repeticiones ./$FOLDER_EXE/matrix $N $min_value $max_value >> $output_file) 2>> $output_file
(sudo perf stat -r $num_repeticiones ./$FOLDER_EXE/eigen_matrix $N $min_value $max_value >> $output_file_e) 2>> $output_file_e


