#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <fstream>
#include <string>
#include <mutex>
#include <chrono>
#include "thread_pool_alpha.hpp"  // Assume thread_pool.hpp includes the thread pool and threadsafe_queue implementation

std::mutex result_mutex;  // Mutex to protect access to the results vector

// Function to check if a number is prime
bool is_prime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

// Task function to find primes in a given range
void find_primes_in_range(int start, int end, std::vector<int>& primes) {
    std::vector<int> local_primes;
    for (int num = start; num <= end; ++num) {
        if (is_prime(num)) {
            local_primes.push_back(num);
        }
    }
    // Lock and transfer local primes to the shared result vector
    std::lock_guard<std::mutex> lock(result_mutex);
    primes.insert(primes.end(), local_primes.begin(), local_primes.end());
}

// Function to search for primes in a range using a thread pool
std::vector<int> parallel_prime_search(int start, int end, int num_threads) {
    thread_pool pool(num_threads);
    std::vector<int> primes;
    int range = end - start + 1;
    int interval_size = range / num_threads;

    // Submit tasks for each sub-range
    for (int i = 0; i < num_threads; ++i) {
        int range_start = start + i * interval_size;
        int range_end = (i == num_threads - 1) ? end : range_start + interval_size - 1;
        pool.submit([range_start, range_end, &primes]() {
            find_primes_in_range(range_start, range_end, primes);
        });
    }

    pool.wait();  // Wait for all tasks to complete
    return primes;
}

int main(int argc, char *argv[]) {
    if (!(argc == 4)) {
        std::cerr << "Invalid syntax: find_primes <start> <end> <num_threads>" << std::endl;
        exit(1);
    }

    int start = std::stol(argv[1]);
    int end = std::stol(argv[2]);
    int num_threads = std::stol(argv[3]);

    auto start_crono = std::chrono::steady_clock::now();

    std::vector<int> primes = parallel_prime_search(start, end, num_threads);

    auto stop = std::chrono::steady_clock::now();
    std::cout  << std::chrono::duration_cast<std::chrono::milliseconds>(stop-start_crono).count() << "" << std::endl; //ms
    // Output the results
    // std::cout << "Found " << primes.size() << " primes between " << start << " and " << end << ".\n";
    return 0;
}
