#include <iostream>
#include "../eigen-3.4.0/Eigen/Dense"  
#include <random>
#include <cstdlib>
#include <sys/time.h> 

using namespace std;
using namespace Eigen;

// Función para generar matrices aleatorias 
MatrixXd generate_random_matrix(unsigned int size, double min_value, double max_value) {
    MatrixXd mat(size, size);
    
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(min_value, max_value);  
    
    for (int i = 0; i < mat.size(); ++i) {
        mat(i) = dis(gen);
    }

    return mat;
}

// Función para medir el tiempo usando gettimeofday()
double get_time_in_seconds() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// Función para multiplicar matrices con medición de tiempos
void multiplicar_matrix(unsigned int size, double min_value, double max_value, bool verbose=false) {
    // Medir el tiempo de declaración, asignación y generación de matrices aleatorias
    double start_init_time = get_time_in_seconds();

    MatrixXd mat1 = generate_random_matrix(size, min_value, max_value);
    MatrixXd mat2 = generate_random_matrix(size, min_value, max_value);

    double end_init_time = get_time_in_seconds();
    cout << (end_init_time - start_init_time) << "\n";

    // Medir el tiempo de la multiplicación de matrices
    double start_mult_time = get_time_in_seconds();

    MatrixXd result = mat1 * mat2;

    double end_mult_time = get_time_in_seconds();
    cout << (end_mult_time - start_mult_time) << "\n";

    if (verbose) {
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
    double min_value = atof(argv[2]);  
    double max_value = atof(argv[3]);  

    multiplicar_matrix(size, min_value, max_value);

    return 0;
}
