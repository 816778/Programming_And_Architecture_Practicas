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

    void fill_random(T min_value = 1, T max_value = 100) {
        random_device rd; // Semilla
        mt19937 gen(rd()); // Generador Mersenne Twister
        uniform_int_distribution<T> dis(min_value, max_value); 

        for (unsigned int i = 0; i < _n; i++) {
            for (unsigned int j = 0; j < _n; j++) {
                m[i][j] = dis(gen); // Asignar un valor aleatorio a cada posición
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


void test_matrix(){
    unsigned int size = 2;
    Matrix<int> mat1(size);
    Matrix<int> mat2(size);

    vector<int> values1 = {1, 2, 3, 4};
    vector<int> values2 = {5, 6, 7, 8};

    mat1.fill(values1);
    mat2.fill(values2);

    Matrix<int> result = mat1 * mat2;

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

    // result.print();
}


int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Uso: " << argv[0] << " <size> <min_value> <max_value>" << endl;
        return 1;
    }

    unsigned int size = atoi(argv[1]);
    int min_value = atoi(argv[2]);
    int max_value = atoi(argv[3]);

    multiplicar_matrix(size, min_value, max_value);
    return 0;
}