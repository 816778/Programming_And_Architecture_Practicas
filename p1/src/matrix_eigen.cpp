#include <iostream>
#include "../eigen-3.4.0/Eigen/Dense"  
#include <random>
#include <cstdlib>

using namespace std;
using namespace Eigen;

// Función para generar matrices aleatorias
MatrixXi generate_random_matrix(unsigned int size, int min_value, int max_value) {
    MatrixXi mat(size, size);
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(min_value, max_value);
    
    for (int i = 0; i < mat.size(); ++i) {
        mat(i) = dis(gen);
    }

    return mat;
}

// Función para multiplicar matrices
void multiplicar_matrix(unsigned int size, int min_value, int max_value, bool verbose=false) {
    MatrixXi mat1 = generate_random_matrix(size, min_value, max_value);
    MatrixXi mat2 = generate_random_matrix(size, min_value, max_value);
    MatrixXi result = mat1 * mat2;

    if (verbose){
        cout << "Matrix 1:\n" << mat1 << endl;
        cout << "Matrix 2:\n" << mat2 << endl;
        cout << "Resultado de la multiplicación de matrices:\n" << result << endl;
    }
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
