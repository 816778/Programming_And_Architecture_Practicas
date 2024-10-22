# PACS_MASTER

## Laboratory 1
p1 contains a benchmark to compare the performance of three matrix multiplication implementations: two custom implementations and one using the Eigen library. The script execute.sh runs the experiments and records the execution times across different matrix sizes, storing the results in .txt files for further analysis.

We are benchmarking the following implementations:

1. **Custom Matrix Implementation 1**: 
   - Uses a single array to store matrix values.
2. **Custom Matrix Implementation 2**: 
   - Uses a double-pointer structure to store the matrix in a 2D array.
3. **Eigen Matrix Implementation**: 
   - Uses the highly optimized Eigen library for matrix multiplication.

The results include metrics like real time, user time, system time, and their respective standard deviations across multiple runs for various matrix sizes.

### Compilation and Execution

Run the `execute.sh` script as follows:

```bash
./execute.sh <min_value> <max_value> <size_inicial> <size_final> <incremento>
```

## Laboratory 2

p2 contains multiple exercises focused on matrix multiplication performance and system call analysis. The exercises involve custom matrix implementations, comparisons with the Eigen library, and profiling using tools such as gettimeofday(), strace, and perf. The results highlight differences in efficiency, performance metrics, and system behavior across different implementations.


2. **Exercise 2: Matrix Multiplication Timing**: 
   - Compares execution times between a custom and Eigen-based matrix multiplication implementation, using gettimeofday() for both initialization and multiplication.
   ```bash
   ./execute_2.sh <min_value> <max_value> <size_inicial> <size_final> <incremento>
   ```
3. **Exercise 3: System Call Analysis with strace**: 
   - Analyzes system calls for both implementations using strace -c to compare system call frequency and execution time.
   ```bash
   ./execute_3.sh <min_value> <max_value> <N> <num_repeticiones>
   ```
4. **Exercise 4: Performance Profiling with perf**: 
   - Profiles both implementations using perf stat -r 10, collecting metrics such as CPU cycles, instructions, and context switches to assess efficiency.
   ```bash
   ./execute_4.sh <min_value> <max_value> <N> <num_repeticiones>
   ```

---

## Laboratory 3

p3 contains exercises focused on parallel programming concepts, including load balancing, parallel reductions, and performance analysis. The exercises explore multi-threaded implementations, performance optimization techniques, and profiling using tools like perf and gettimeofday().
1. **Exercise 1: Parallel π Approximation with Taylor Series**: 

2. **Exercise 2: Matrix Multiplication Timing**: 

3. **Exercise 3: Matrix Multiplication Timing**: 

---