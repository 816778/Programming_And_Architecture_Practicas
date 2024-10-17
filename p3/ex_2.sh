#!/bin/bash

# Asumiendo que el directorio actual es build-debug/Laboratory-3
BUILD_FOLDER="build"
RESULTS_FOLDER="results"

# Compilar el programa
make all

time_output_file_1="$RESULTS_FOLDER/1_execution_times.txt"
> $time_output_file_1

# Array de valores de steps
steps_values=(16 128 1024 1048576 16777216 1073741824 4294967295)

# Ejecutar el programa para cada valor de steps
for steps in "${steps_values[@]}"; do
    echo "Running with steps = $steps"
    ./$BUILD_FOLDER/pi_taylor_sequential "$steps" "$time_output_file_1"
    echo "---------------------------------"
done
