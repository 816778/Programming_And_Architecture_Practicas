#!/bin/bash

# Asumiendo que el directorio actual es build-debug/Laboratory-3
BUILD_FOLDER="build"
RESULTS_FOLDER="results"

# Compilar el programa
make all

output_file="$RESULTS_FOLDER/ex_β.txt"
> $output_file

echo "Running..."
> $output_file

steps=1048576
thread=$(nproc)
echo "Número de núcles = $thread"
./$BUILD_FOLDER/pi_taylor_parallel_extra "$steps" "$thread" 

echo "##################################################################"
./$BUILD_FOLDER/pi_taylor_parallel_extra "$steps" 64