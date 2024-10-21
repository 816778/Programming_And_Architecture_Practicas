#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <chrono>

using my_float = float;

void
pi_taylor_chunk_0(my_float &output,
        size_t thread_id, size_t start_step, size_t stop_step) {

    output = 0.0f;
    for (size_t i = start_step; i < stop_step; i++) output += (i % 2 == 0 ? 1 : -1) / (2.0f * i + 1);
    output *= 4.0f;
}

void pi_taylor_chunk(std::vector<my_float> &output, size_t thread_id, size_t start_step, size_t stop_step) {
    my_float sum = 0.0;
    my_float c = 0.0; 

    for (size_t i = start_step; i < stop_step; i++) {
        my_float y = (i % 2 == 0 ? 1.0f : -1.0f) / (2.0f * i + 1) - c;
        my_float t = sum + y;
        c = (t - sum) - y; // Se calcula la nueva compensación
        sum = t;
    }
    output[thread_id] = sum;
}


void save_file(std::string output_file, int threads, std::chrono::duration<double> elapsed){
    std::cout << threads << "," << elapsed.count() << std::endl;
}


std::pair<size_t, size_t>usage(int argc, const char *argv[]) {
    // read the number of steps from the command line
    if (argc != 3) {
        std::cerr << "Invalid syntax: pi_taylor <steps> <threads>" << std::endl;
        exit(1);
    }

    size_t steps = std::stoll(argv[1]);
    size_t threads = std::stoll(argv[2]);

    if (steps < threads ){
        std::cerr << "The number of steps should be larger than the number of threads" << std::endl;
        exit(1);

    }
    return std::make_pair(steps, threads);
}


int main(int argc, const char *argv[]) {


    auto ret_pair = usage(argc, argv);
    auto steps = ret_pair.first;
    auto threads = ret_pair.second;

    std::vector<my_float> pi_branch(threads);
    std::vector<std::thread> branch;
    auto start = std::chrono::high_resolution_clock::now();

    my_float pi = 0;

    for (size_t i = 0; i < threads; ++i) {
        size_t start_step = steps * i / threads;
        size_t stop_step = steps * (i + 1) / threads;

        // Create a thread for each chunk
        branch.emplace_back(pi_taylor_chunk, std::ref(pi_branch), i, start_step, stop_step);
    }

    // Join threads
    for (size_t i = 0; i < threads; i++) {
        branch[i].join();
        pi += pi_branch[i];
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    pi *= 4.0;
    

    std::cout << "For " << steps << ", pi value: "
        << std::setprecision(std::numeric_limits<long double>::digits10 + 1)
        << pi << std::endl;
    save_file("", threads, elapsed);
}

