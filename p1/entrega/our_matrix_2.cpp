#include <iostream>
#include <vector>
#include <random>
using namespace std;

template <typename T>
class Matrix {
    unsigned int _n;
    T** m;

public:
    Matrix(unsigned int n) : _n(n) {
        m = new T*[n];
        for (unsigned int i = 0; i < n; i++) {
            m[i] = new T[n];
        }
    }


    ~Matrix() {
        for (unsigned int i = 0; i < _n; i++) {
            delete[] m[i];
        }
        delete[] m;
    }

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

    void print() const {
        for (unsigned int i = 0; i < _n; i++) {
            for (unsigned int j = 0; j < _n; j++) {
                cout << m[i][j] << " ";
            }
            cout << endl;
        }
    }
};

// Función de prueba para multiplicar matrices de tipo double
void test_matrix() {
    unsigned int size = 2;
    Matrix<double> mat1(size);
    Matrix<double> mat2(size);

    vector<double> values1 = {1.1, 2.0, 3.0, 4.0};
    vector<double> values2 = {5.0, 6.0, 7.0, 8.0};

    mat1.fill(values1);
    mat2.fill(values2);

    Matrix<double> result = mat1 * mat2;

    cout << "Matrix 1:" << endl;
    mat1.print();
    cout << "Matrix 2:" << endl;
    mat2.print();
    cout << "Resultado de la multiplicación de matrices:" << endl;
    result.print();
}

// Función para multiplicar matrices aleatorias de tipo double
void multiplicar_matrix(unsigned int size, double min_value, double max_value) {
    Matrix<double> mat1(size);
    Matrix<double> mat2(size);

    mat1.fill_random(min_value, max_value);
    mat2.fill_random(min_value, max_value);

    Matrix<double> result = mat1 * mat2;
    // result.print();
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
