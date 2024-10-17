#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using my_float = float;

void
pi_taylor_chunk_0(my_float &output,
        size_t thread_id, size_t start_step, size_t stop_step) {

    output = 0.0f;
    for (size_t i = start_step; i < stop_step; i++) output += (i % 2 == 0 ? 1 : -1) / (2.0f * i + 1);
    output *= 4.0f;
}

void pi_taylor_chunk(my_float &output, size_t start_step, size_t stop_step) {
    output = 0.0f;
    my_float c = 0.0f; // Compensación para Kahan Summation
    for (size_t i = start_step; i < stop_step; i++) {
        my_float y = (i % 2 == 0 ? 1.0f : -1.0f) / (2.0f * i + 1) - c;
        my_float t = output + y;
        c = (t - output) - y; // Se calcula la nueva compensación
        output = t;
    }
}

std::pair<size_t, size_t>
usage(int argc, const char *argv[]) {
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

    my_float pi = 0, *pi_branch = new my_float[threads], c = 0, y, t;

    // please complete missing parts
    std::thread* branch = new std::thread[threads];
    for (size_t i = 0; i < threads; i++) {
        branch[i] = std::thread(pi_taylor_chunk, std::ref(pi_branch[i]), i, steps * i / threads, steps * (i + 1) / threads);
    }

    for (size_t i = 0; i < threads; i++) {
        branch[i].join();
        y = pi_branch[i] - c;
        t = pi + y;
        c = (t - pi) - y;
        pi = t;
    }
    

    std::cout << "For " << steps << ", pi value: "
        << std::setprecision(std::numeric_limits<long double>::digits10 + 1)
        << pi << std::endl;
}

