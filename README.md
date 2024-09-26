# PACS_MASTER

## Práctica 1
p1 contains a benchmark to compare the performance of three matrix multiplication implementations: two custom implementations and one using the Eigen library. The script execute.sh runs the experiments and records the execution times across different matrix sizes, storing the results in .txt files for further analysis.

We are benchmarking the following implementations:

1. **Custom Matrix Implementation 1**: 
   - Uses a single array to store matrix values.
2. **Custom Matrix Implementation 2**: 
   - Uses a double-pointer structure to store the matrix in a 2D array.
3. **Eigen Matrix Implementation**: 
   - Uses the highly optimized Eigen library for matrix multiplication.

The results include metrics like real time, user time, system time, and their respective standard deviations across multiple runs for various matrix sizes.

---

### Compilation and Execution

Run the `execute.sh` script as follows:

```bash
./execute.sh <min_value> <max_value> <size_inicial> <size_final> <incremento>