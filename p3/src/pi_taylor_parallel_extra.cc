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
pi_taylor_chunk(std::vector<my_float> &output,
        size_t thread_id, size_t start_step, size_t stop_step){
    
    auto start_time = std::chrono::high_resolution_clock::now();

    my_float sum = 0.0;

    for (size_t i = start_step; i < stop_step; ++i) {
        my_float term = (i % 2 == 0 ? 1.0 : -1.0) / (2 * i + 1);
        sum += term;
    }
    output[thread_id] = sum;
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> exec_time = end_time - start_time;

    // Imprimir el identificador del hilo y su tiempo de ejecución
    std::cout << "Thread " + std::to_string(thread_id) + 
             "," + std::to_string(start_time.time_since_epoch().count()) +
             "," + std::to_string(end_time.time_since_epoch().count()) +       
            "," + std::to_string(exec_time.count()) + "\n";  //execution time:  

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


void save_file(std::string output_file, int threads, std::chrono::duration<double> elapsed){
    std::ofstream outfile(output_file, std::ios::app); // Modo append
    if (outfile.is_open()) {
        outfile << threads << "," << elapsed.count() << std::endl;
        outfile.close();
    } else {
        std::cerr << "Error opening file!" << std::endl;
    }
}


int main(int argc, const char *argv[]) {


    auto ret_pair = usage(argc, argv);
    auto steps = ret_pair.first;
    auto threads = ret_pair.second;

    std::vector<my_float> pi_branch(threads);
    std::vector<std::thread> branch;

    my_float pi = 0;
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < threads; ++i) {
        size_t start_step = steps * i / threads;
        size_t stop_step = steps * (i + 1) / threads;
    
        // Create a thread for each chunk
        branch.emplace_back(pi_taylor_chunk, std::ref(pi_branch), i, start_step, stop_step);
    }
    
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

    //std::string output_file = (argc == 4) ? argv[3] : "results/4_execution_times.txt";
    // save_file(output_file, threads, elapsed);
}

