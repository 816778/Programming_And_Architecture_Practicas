/*
    g++ matrix.cpp -o matrix
    time ./matrix

    g++ matrix.cpp -o matrix_2
    time ./2_matrix

    g++ -O2 -Wall -I p1/eigen-3.4.0/ matrix.cpp -o matrix_eigen
    time ./eigen_matrix
*/

#include <iostream>
#include <vector>
#include <random>
#include <cstdlib>
using namespace std;

template <typename T>
class Matrix {
	unsigned int _n;
	T* m;

public:

	Matrix(unsigned int n) : _n(n), m(new T[n*n]) {}

    Matrix(const vector<T>& v) {
        _n = sqrt(v.size());
        if (_n * _n != v.size()) {
            throw runtime_error("Vector size must be a perfect square.");
        }
        m = new T[_n*_n];
        fill(v);
    }

    ~Matrix() {
        delete[] m;
    }


    void fill_random(T min_value = 1, T max_value = 100) {
        random_device rd;  // Seed
        mt19937 gen(rd()); // Generador Mersenne Twister
        uniform_int_distribution<T> dis(min_value, max_value); // Distribución uniforme

        for (unsigned int i = 0; i < _n * _n; i++) {
            m[i] = dis(gen); // Asignar un valor aleatorio
        }
    }

	void fill(const vector<T>& v) {
        if (_n * _n != v.size()) {
            throw runtime_error("Vector and matrix size mismatch. Vector has size " + to_string(v.size()) + " and matrix has size " + to_string(_n * _n) + ".");
        }
        unsigned int k = 0;
        for (unsigned int i = 0; i < _n*_n; i++) m[i] = v[k++];
    }

	friend Matrix operator*(const Matrix& a, const Matrix& b) {
        if (a._n != b._n) {
            throw runtime_error("Matrix size mismatch. Both matrices must have the same size.");
        }

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

    void print() const {
        for (unsigned int i = 0; i < _n*_n; i++) {
            cout << m[i] << " ";
            if (i%_n == _n-1) cout << endl;
        }
    }

};


void test_matrix(){
    unsigned int size = 2;
    Matrix<int> mat1(size);
    Matrix<int> mat2(size);

    vector<int> values1 = {1, 2, 3, 4};
    vector<int> values2 = {5, 6, 7, 8};

    // Fill matrices with values
    mat1.fill(values1);
    mat2.fill(values2);

    // Perform matrix multiplication
    Matrix<int> result = mat1 * mat2;

    // Print the result
    cout << "Matrix 1:" << endl;
    mat1.print();
    cout << "Matrix 2:" << endl;
    mat2.print();
    cout << "Result of Matrix Multiplication:" << endl;
    result.print();
}

void multiplicar_matrix(unsigned int size, int min_value, int max_value){
    Matrix<int> mat1(size);
    Matrix<int> mat2(size);

    mat1.fill_random(min_value, max_value);
    mat2.fill_random(min_value, max_value);

    Matrix<int> result = mat1 * mat2;

    //result.print();
}


int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Uso: " << argv[0] << " <size> <min_value> <max_value>" << endl;
        return 1;
    }
    // Convertir los argumentos de cadena a enteros
    unsigned int size = atoi(argv[1]);
    int min_value = atoi(argv[2]);
    int max_value = atoi(argv[3]);

    multiplicar_matrix(size, min_value, max_value);

    return 0;
}