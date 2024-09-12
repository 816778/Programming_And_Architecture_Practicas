#!/bin/bash

# Example command
# ./run_matrix.sh 1 100 2 200 20

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

echo "Compilando ficheros..."

g++ matrix.cpp -o matrix
g++ matrix.cpp -o 2_matrix
g++ -O2 -Wall -I p1/eigen-3.4.0/ matrix.cpp -o eigen_matrix
g++ -O2 -Wall -I p1/eigen-3.4.0/ matrix_2.cpp -o matrix_eigen_2

# Archivo para guardar los tiempos de ejecución
output_file="time_matrix.txt"
output_file_2="time_matrix_2.txt"
output_file_3="time_matrix_eg.txt"
output_file_4="time_matrix_eg2.txt"

# Limpiar el archivo de salida
> $output_file
> $output_file_2
> $output_file_3
> $output_file_4


echo "Ejecutando multiplicaciones"

# Bucle para ejecutar el programa con tamaños de 2 a 200, incrementando de 20 en 20
for (( size=$size_inicial; size<=$size_final; size+=$incremento ))
do
    # Ejecutar el programa con el tamaño y los valores min_value y max_value, y guardar el tiempo
    { /usr/bin/time -f "$size %e %U %S" ./matrix $size $min_value $max_value; } 2>> $output_file
    { /usr/bin/time -f "$size %e %U %S" ./2_matrix $size $min_value $max_value; } 2>> $output_file_2
    { /usr/bin/time -f "$size %e %U %S" ./eigen_matrix $size $min_value $max_value; } 2>> $output_file_3
    { /usr/bin/time -f "$size %e %U %S" ./matrix_eigen_2 $size $min_value $max_value; } 2>> $output_file_4
done

echo "Ejecución completa. Tiempos guardados"
