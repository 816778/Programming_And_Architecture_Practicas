#!/bin/bash

# Assuming that the current directory is build-debug/Laboratory-3
BUILD_FOLDER="build"
RESULTS_FOLDER="results"

# Compile the code.
make 

output_file="$RESULTS_FOLDER/ex_beta_speedup.txt"
> $output_file

echo "Running..."

end_list=(1000000 5000000 10000000 50000000 100000000)
num_threads_list=(1 4 8 12 16 32)

for end in "${end_list[@]}"; do
    for num_threads in "${num_threads_list[@]}"; do
        echo -n "$end,$num_threads," >> "$output_file"
        ./$BUILD_FOLDER/find_primes 1 "$end" "$num_threads" >> "$output_file"  
    done
done
