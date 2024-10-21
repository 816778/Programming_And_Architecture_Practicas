#!/bin/bash

# Asumiendo que el directorio actual es build-debug/Laboratory-3
BUILD_FOLDER="build"
RESULTS_FOLDER="results"

# Compilar el programa
make all

output_file="$RESULTS_FOLDER/ex_6.txt"
> $output_file

echo "Running..."
> $output_file


(sudo perf stat -r 15 ./$BUILD_FOLDER/pi_taylor_parallel 4294967295 4  >> $output_file) 2>> $output_file
(sudo perf stat -r 15 ./$BUILD_FOLDER/pi_taylor_parallel 4294967295 8 >> $output_file) 2>> $output_file

