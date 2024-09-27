#include <iostream>
#include <vector>
#include <random>
#include <cstdlib>
#include <sys/time.h>  
using namespace std;

/*
    g++ src/matrix.cpp -o executable/matrix
    time ./executable/matrix
*/

template <typename T>
class Matrix {
    unsigned int _n;
    T* m;

public:
    Matrix(unsigned int n) : _n(n), m(new T[n*n]) {}
    ~Matrix() { delete[] m; }

    void fill_random(T min_value = 1.0, T max_value = 100.0) {
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<T> dis(min_value, max_value);
        for (unsigned int i = 0; i < _n * _n; i++) {
            m[i] = dis(gen);
        }
    }

    friend Matrix operator*(const Matrix& a, const Matrix& b) {
        if (a._n != b._n) throw runtime_error("Matrix size mismatch.");
        Matrix<T> c(a._n);
        for (unsigned int i = 0; i < a._n; i++) {
            for (unsigned int j = 0; j < a._n; j++) {
                c.m[i*c._n+j] = 0;
                for (unsigned int k = 0; k < a._n; k++) {
                    c.m[i*c._n+j] += a.m[i*c._n+k] * b.m[k*c._n+j];
                }
            }
        }
        return c;
    }
};

// Función para medir el tiempo usando gettimeofday()
double get_time_in_seconds() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Uso: " << argv[0] << " <size> <min_value> <max_value>" << endl;
        return 1;
    }

    unsigned int size = atoi(argv[1]);
    double min_value = atof(argv[2]);
    double max_value = atof(argv[3]);

    // Medir el tiempo de declaración, asignación y inicialización
    double start_init_time = get_time_in_seconds();

    Matrix<double> mat1(size);
    Matrix<double> mat2(size);
    mat1.fill_random(min_value, max_value);
    mat2.fill_random(min_value, max_value);

    double end_init_time = get_time_in_seconds();
    cout << (end_init_time - start_init_time) << "\n";

    // Medir el tiempo de la multiplicación de matrices
    double start_mult_time = get_time_in_seconds();

    Matrix<double> result = mat1 * mat2;

    double end_mult_time = get_time_in_seconds();
    cout << (end_mult_time - start_mult_time) << "\n";

    return 0;
}
