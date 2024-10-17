#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <chrono>
#include <fstream>

using my_float = long double;

void
pi_taylor_chunk(my_float &output,
        size_t thread_id, size_t start_step, size_t stop_step) {

    output = 0.0f;
    for (size_t i = start_step; i < stop_step; i++) output += (i % 2 == 0 ? 1 : -1) / (2.0f * i + 1);
    output *= 4.0f;
}

std::pair<size_t, size_t>
usage(int argc, const char *argv[]) {
    // read the number of steps from the command line
    if (argc < 3 || argc > 4) {
        std::cerr << "Usage: pi_taylor_parallel <steps> <threads> [output_file]" << std::endl;
        exit(1);
    }

    size_t steps = std::stoll(argv[1]);
    size_t threads = std::stoll(argv[2]);

    if (steps < threads){
        std::cerr << "The number of steps should be larger than the number of threads" << std::endl;
        exit(1);

    }
    return std::make_pair(steps, threads);
}

int main(int argc, const char *argv[]) {


    auto ret_pair = usage(argc, argv);
    auto steps = ret_pair.first;
    auto threads = ret_pair.second;

    my_float pi = 0, *pi_branch = new my_float[threads];

    // please complete missing parts
    std::thread* branch = new std::thread[threads];

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < threads; i++) {
        branch[i] = std::thread(pi_taylor_chunk, std::ref(pi_branch[i]), i, steps * i / threads, steps * (i + 1) / threads);
    }

    for (size_t i = 0; i < threads; i++) {
        branch[i].join();
        pi += pi_branch[i];
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "For " << steps << ", pi value: "
        << std::setprecision(std::numeric_limits<long double>::digits10 + 1)
        << pi << std::endl;

    std::string output_file = (argc == 4) ? argv[3] : "results/4_execution_times.txt";

    std::ofstream outfile(output_file, std::ios::app); // Modo append
    if (outfile.is_open()) {
        outfile << threads << "," << elapsed.count() << std::endl;
        outfile.close();
    } else {
        std::cerr << "Error opening file!" << std::endl;
    }
}

