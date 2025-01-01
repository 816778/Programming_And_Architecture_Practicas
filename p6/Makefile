# Nombre del ejecutable
EXECUTABLE = platformsAndDeviced

# Archivos fuente
SRC = platformsAndDeviced.cpp

# Compilador
CC = g++

# Flags del compilador
CFLAGS = -std=c++11 -O2 -Iinclude

# Rutas de bibliotecas
LIB_PATHS = -L/opt/intel/oneapi/compiler/2023.2.0/linux/lib \
            -L/opt/intel/oneapi/compiler/2023.2.0/linux/compiler/lib/intel64

# Rutas en tiempo de ejecución
RPATH_FLAGS = -Wl,-rpath=/opt/intel/oneapi/compiler/2023.2.0/linux/lib \
              -Wl,-rpath=/opt/intel/oneapi/compiler/2023.2.0/linux/compiler/lib/intel64

# Flags de enlace
LDFLAGS = $(LIB_PATHS) $(RPATH_FLAGS) -lOpenCL -lm -lpthread -lX11 -ljpeg


# Regla principal
all: $(EXECUTABLE)

$(EXECUTABLE): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(EXECUTABLE) $(LDFLAGS)

# Limpieza
clean:
	rm -f $(EXECUTABLE)
