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

# Tarea principal
all: $(BUILD_DIR) $(EXEC)

# Crear el directorio build si no existe
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compilar el ejecutable
$(EXEC): $(SRCS)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(SRC_DIR)/smallpt_thread_pool.cpp -o $(EXEC)

# Limpiar archivos generados
clean:
	rm -rf $(BUILD_DIR)
