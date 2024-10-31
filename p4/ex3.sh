#!/bin/bash

# Asumiendo que el directorio actual es build-debug/Laboratory-3
BUILD_FOLDER="build"
RESULTS_FOLDER="results"

# Compilar el programa
make 

output_file="$RESULTS_FOLDER/ex_3.txt"
> $output_file

echo "Running..."


columns=(1 2 3 4 8 12 16)
rows=(1 2 3 4 8 12 16)

for col in "${columns[@]}"; do
    for row in "${rows[@]}"; do
        echo -n "$col,$row," >> "$output_file"
        ./$BUILD_FOLDER/smallpt_thread_pool "$col" "$row" >> "$output_file"  
    done
done
