#include <iomanip>
#include <iostream>
#include <fstream>
#include <limits>
#include <chrono>

// Allow to change the floating point type
using my_float = long double;

my_float pi_taylor(size_t steps) {

    my_float pi = 0.0f;
    for (size_t i = 0; i < steps; i++) pi += (i % 2 == 0 ? 1 : -1) / (2.0f * i + 1);
    return 4.0f * pi;
}

int main(int argc, const char *argv[]) {

    // read the number of steps from the command line
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: pi_taylor <steps> [output_file]" << std::endl;
        exit(1);

    }

    size_t steps = std::stoll(argv[1]);
    auto start = std::chrono::high_resolution_clock::now();
    auto pi = pi_taylor(steps);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;

    std::cout << "For " << steps << ", pi value: "
        << std::setprecision(std::numeric_limits<my_float>::digits10 + 1)
        << pi << std::endl;

    std::string output_file = (argc == 3) ? argv[2] : "results/1_execution_times.txt";

    std::ofstream outfile(output_file, std::ios::app); // Modo append
    if (outfile.is_open()) {
        outfile << steps << "," << elapsed.count() << std::endl;
        outfile.close();
    } else {
        std::cerr << "Error opening file!" << std::endl;
    }
    return 0;
}
