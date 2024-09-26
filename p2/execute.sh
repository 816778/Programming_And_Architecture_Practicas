#!/bin/bash

# Example command:
# ./execute.sh 1 1000 2 1003 100

# Salida
# size  mean_real  mean_user  mean_sys  stddev_real  stddev_user  stddev_sys

FOLDER_EXE="executable"
FOLDER_RESULT="results"


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


echo "Compilando ficheros..."
g++ src/matrix.cpp -o $FOLDER_EXE/matrix
g++ -O2 -Wall -I p2/eigen-3.4.0/ src/matrix_eigen.cpp -o $FOLDER_EXE/eigen_matrix

for (( size=$size_inicial; size<=$size_final; size+=$incremento ))
do
    echo "Size: $size..."
    ./$FOLDER_EXE/matrix $size $min_value $max_value && time ./$FOLDER_EXE/matrix $size $min_value $max_value
done

