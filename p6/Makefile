# Archivos ejecutables
EXECUTABLES = platformsAndDeviced first_experiment

# Archivos fuente
SRC_platformsAndDeviced = platformsAndDeviced.cpp
SRC_first_experiment = first_experiment.cpp

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
all: $(EXECUTABLES)

# Compilación de platformsAndDeviced
platformsAndDeviced: $(SRC_platformsAndDeviced)
	$(CC) $(CFLAGS) $(SRC_platformsAndDeviced) -o platformsAndDeviced $(LDFLAGS)

# Compilación de first_experiment
first_experiment: $(SRC_first_experiment)
	$(CC) $(CFLAGS) $(SRC_first_experiment) -o first_experiment $(LDFLAGS)

# Limpieza
clean:
	rm -f $(EXECUTABLES)
