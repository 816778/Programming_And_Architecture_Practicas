# Nombre del compilador
CXX = g++

# Opciones de compilación
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

# Directorios
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

# Archivos fuente y el ejecutable
SRCS = $(SRC_DIR)/smallpt_thread_pool.cpp $(wildcard $(INC_DIR)/*.hpp)
EXEC = $(BUILD_DIR)/smallpt_thread_pool
EXEC_PRIMES = $(BUILD_DIR)/find_primes

# Tarea principal
all: $(BUILD_DIR) $(EXEC) $(EXEC_PRIMES)

# Crear el directorio build si no existe
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compilar el ejecutable
$(EXEC): $(SRCS)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(SRC_DIR)/smallpt_thread_pool.cpp -o $(EXEC)

$(EXEC_PRIMES): $(SRC_DIR)/find_primes.cpp $(wildcard $(INC_DIR)/*.hpp)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(SRC_DIR)/find_primes.cpp -o $(EXEC_PRIMES)

# Limpiar archivos generados
clean:
	rm -rf $(BUILD_DIR)
