#!/bin/bash

# Example command
# ./a.sh 1 1000 2 1003 50

# Salida
# size, real, user, sys


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

FOLDER_EXE="executable"
FOLDER_RESULT="results"

echo "Compilando ficheros..."

g++ src/matrix.cpp -o executable/matrix
g++ src/matrix.cpp -o executable/2_matrix
g++ -O2 -Wall -I p1/eigen-3.4.0/ src/matrix_eigen.cpp -o executable/eigen_matrix

# Archivo para guardar los tiempos de ejecución
output_file="$FOLDER_RESULT/time_matrix.txt"
output_file_2="$FOLDER_RESULT/time_matrix_2.txt"
output_file_3="$FOLDER_RESULT/time_matrix_eg.txt"

# Limpiar el archivo de salida
> $output_file
> $output_file_2
> $output_file_3


echo "Ejecutando multiplicaciones"

# Bucle para ejecutar el programa con tamaños de 2 a 200, incrementando de 20 en 20
for (( size=$size_inicial; size<=$size_final; size+=$incremento ))
do
    # Ejecutar el programa con el tamaño y los valores min_value y max_value, y guardar el tiempo
    echo "$size" >> $output_file
    { time ./$FOLDER_EXE/matrix $size $min_value $max_value; } 2>> $output_file
    echo "$size" >> $output_file_2
    { time ./$FOLDER_EXE/2_matrix $size $min_value $max_value; } 2>> $output_file_2
    echo "$size" >> $output_file_3
    { time ./$FOLDER_EXE/eigen_matrix $size $min_value $max_value; } 2>> $output_file_3

done

echo "Ejecución completa. Tiempos guardados"
