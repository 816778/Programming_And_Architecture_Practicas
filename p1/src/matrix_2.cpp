#include <iostream>
#include <vector>
#include <random>
#include <cstdlib>
#include <thread>
#include <future>
#include <cmath>

using namespace std;

// matriz con arreglo bidimensional
template <typename T>
class Matrix2D {
    unsigned int _n;
    T** m;

public:
    Matrix2D(unsigned int n) : _n(n) {
        m = new T*[n];
        for (unsigned int i = 0; i < n; i++) {
            m[i] = new T[n];
        }
    }

    ~Matrix2D() {
        for (unsigned int i = 0; i < _n; i++) {
            delete[] m[i];
        }
        delete[] m;
    }

    void fill_random(T min_value = 1.0, T max_value = 100.0) {
        random_device rd; 
        mt19937 gen(rd());
        uniform_real_distribution<T> dis(min_value, max_value); 

        for (unsigned int i = 0; i < _n; i++) {
            for (unsigned int j = 0; j < _n; j++) {
                m[i][j] = dis(gen); 
            }
        }
    }

    friend Matrix2D operator*(const Matrix2D& a, const Matrix2D& b) {
        if (a._n != b._n) {
            throw runtime_error("Matrix size mismatch.");
        }

        Matrix2D c(a._n);
        for (unsigned int i = 0; i < a._n; i++) {
            for (unsigned int j = 0; j < a._n; j++) {
                c.m[i][j] = 0;
                for (unsigned int k = 0; k < a._n; k++) {
                    c.m[i][j] += a.m[i][k] * b.m[k][j];
                }
            }
        }
        return c;
    }

    void print() const {
        for (unsigned int i = 0; i < _n; i++) {
            for (unsigned int j = 0; j < _n; j++) {
                cout << m[i][j] << " ";
            }
            cout << endl;
        }
    }
};

// matriz con arreglo unidimensional
template <typename T>
class Matrix1D {
    unsigned int _n;
    T* m;

public:
    Matrix1D(unsigned int n) : _n(n), m(new T[n * n]) {}

    ~Matrix1D() {
        delete[] m;
    }

    void fill_random(T min_value = 1.0, T max_value = 100.0) {
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<T> dis(min_value, max_value);

        for (unsigned int i = 0; i < _n * _n; i++) {
            m[i] = dis(gen);
        }
    }

    friend Matrix1D operator*(const Matrix1D& a, const Matrix1D& b) {
        if (a._n != b._n) {
            throw runtime_error("Matrix size mismatch.");
        }

        Matrix1D c(a._n);
        vector<future<void>> futures;

        for (unsigned int i = 0; i < a._n; i++) {
            futures.emplace_back(async(launch::async, [&, i] {
                for (unsigned int j = 0; j < a._n; j++) {
                    c.m[i * c._n + j] = 0;
                    for (unsigned int k = 0; k < a._n; k++) {
                        c.m[i * c._n + j] += a.m[i * a._n + k] * b.m[k * b._n + j];
                    }
                }
            }));
        }

        for (auto& f : futures) {
            f.get(); // Esperar a que todos los hilos terminen
        }

        return c;
    }

    void print() const {
        for (unsigned int i = 0; i < _n * _n; i++) {
            cout << m[i] << " ";
            if (i % _n == _n - 1) cout << endl;
        }
    }
};

// Función para multiplicar matrices aleatorias de tipo double
void multiplicar_matrix(unsigned int size, double min_value, double max_value, int tipo) {
    if (tipo == 1) { // Matrices 2D
        Matrix2D<double> mat1(size);
        Matrix2D<double> mat2(size);
        mat1.fill_random(min_value, max_value);
        mat2.fill_random(min_value, max_value);
        Matrix2D<double> result = mat1 * mat2;
        result.print();
    } else if (tipo == 2) { // Matrices 1D
        Matrix1D<double> mat1(size);
        Matrix1D<double> mat2(size);
        mat1.fill_random(min_value, max_value);
        mat2.fill_random(min_value, max_value);
        Matrix1D<double> result = mat1 * mat2;
        result.print();
    } else {
        cerr << "Tipo de matriz no válido." << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cerr << "Uso: " << argv[0] << " <size> <min_value> <max_value> <tipo>" << endl;
        return 1;
    }

    unsigned int size = atoi(argv[1]);
    double min_value = atof(argv[2]);  
    double max_value = atof(argv[3]);
    int tipo = atoi(argv[4]); // 1 para 2D, 2 para 1D

    multiplicar_matrix(size, min_value, max_value, tipo);

    return 0;
}
