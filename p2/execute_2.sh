#!/bin/bash

# Example command:
# ./execute_2.sh 1 1000 2 1003 100

# Salida
# size  
# <time_init_matrix>
# <time_multiplication_matrix>

FOLDER_EXE="executable/"
FOLDER_RESULT="results"
FOLDER_RESULT_MY_MATRIX="$FOLDER_RESULT/my_matrix"
FOLDER_RESULT_EIGEN="$FOLDER_RESULT/eigen"


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

echo "Compiling files..."
g++ src/matrix.cpp -o $FOLDER_EXE/matrix
g++ -O2 -Wall -I p2/eigen-3.4.0/ src/matrix_eigen.cpp -o $FOLDER_EXE/eigen_matrix


# Archivos para guardar los tiempos de ejecución
output_file="$FOLDER_RESULT_MY_MATRIX/output_1.txt"
output_file_2="$FOLDER_RESULT_MY_MATRIX/output_2.txt"
time_output_file_3="$FOLDER_RESULT_MY_MATRIX/time_output.txt"

output_file_e="$FOLDER_RESULT_EIGEN/output_1.txt"
output_file_2_e="$FOLDER_RESULT_EIGEN/output_2.txt"
time_output_file_3_e="$FOLDER_RESULT_EIGEN/time_output.txt"

> $output_file
> $output_file_2
> $time_output_file_3
> $output_file_e
> $output_file_2_e
> $time_output_file_3_e


for (( size=$size_inicial; size<=$size_final; size+=$incremento ))
do
    echo "Size: $size..."
    
    echo "$size " >> $output_file
    echo "$size " >> $output_file_2
    echo "$size " >> $time_output_file_3
    echo "$size " >> $output_file_e
    echo "$size " >> $output_file_2_e
    echo "$size " >> $time_output_file_3_e

    ./$FOLDER_EXE/matrix $size $min_value $max_value >> $output_file && \
    (time ./$FOLDER_EXE/matrix $size $min_value $max_value >> $output_file_2) 2>> $time_output_file_3

    ./$FOLDER_EXE/eigen_matrix $size $min_value $max_value >> $output_file_e && \
    (time ./$FOLDER_EXE/eigen_matrix $size $min_value $max_value >> $output_file_2_e) 2>> $time_output_file_3_e
done



