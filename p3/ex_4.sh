#!/bin/bash

# Asumiendo que el directorio actual es build-debug/Laboratory-3
BUILD_FOLDER="build"
RESULTS_FOLDER="results"

# Compilar el programa
make all

time_output_file_4="$RESULTS_FOLDER/4_execution_times.txt"
> $time_output_file_4

# Array de valores de steps
thread_values=(1 2 4 8 16)
steps=4294967295
# Ejecutar el programa para cada valor de steps
for thread in "${thread_values[@]}"; do
    echo "Running with theads = $thread" 
    ./$BUILD_FOLDER/pi_taylor_parallel "$steps" "$thread" "$time_output_file_4"
    echo "---------------------------------"
done
