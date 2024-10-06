#include <iostream>
#include <vector>
#include <random>
using namespace std;

/*
    g++ src/matrix.cpp -o executable/matrix
    time ./executable/matrix
*/

template <typename T>
class Matrix {
    unsigned int _n;
    T** m;

public:

    // Matrix constructor with size n.
    Matrix(unsigned int n) : _n(n) {
        m = new T*[n];
        for (unsigned int i = 0; i < n; i++) {
            m[i] = new T[n];
        }
    }

    // Matrix destructor.
    ~Matrix() {
        for (unsigned int i = 0; i < _n; i++) {
            delete[] m[i];
        }
        delete[] m;
    }

    // Fill the matrix with the values of a vector.
    void fill(const vector<T>& v) {
        if (_n * _n != v.size()) {
            throw runtime_error("Vector and matrix size mismatch. Vector has size " + to_string(v.size()) + " and matrix has size " + to_string(_n * _n) + ".");
        }
        unsigned int k = 0;
        for (unsigned int i = 0; i < _n; i++) {
            for (unsigned int j = 0; j < _n; j++) {
                m[i][j] = v[k++];
            }
        }
    }

    // Fill the matrix with random values between min_value and max_value.
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

    // Matrix multiplication operator overloading.
    friend Matrix operator*(const Matrix& a, const Matrix& b) {
        if (a._n != b._n) {
            throw runtime_error("Matrix size mismatch. Both matrices must have the same size.");
        }

        Matrix<T> c(a._n);
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

    // Matrix print function.
    void print() const {
        for (unsigned int i = 0; i < _n; i++) {
            for (unsigned int j = 0; j < _n; j++) {
                cout << m[i][j] << " ";
            }
            cout << endl;
        }
    }
};


// Multiply two double-type and randomly filled matrices.
void multiplicar_matrix(unsigned int size, double min_value, double max_value) {
    Matrix<double> mat1(size);
    Matrix<double> mat2(size);

    mat1.fill_random(min_value, max_value);
    mat2.fill_random(min_value, max_value);

    Matrix<double> result = mat1 * mat2;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Uso: " << argv[0] << " <size> <min_value> <max_value>" << endl;
        return 1;
    }

    unsigned int size = atoi(argv[1]);
    double min_value = atof(argv[2]);  
    double max_value = atof(argv[3]); 

    multiplicar_matrix(size, min_value, max_value);

    return 0;
}
