#!/bin/bash

# ssh -X a816778@central.unizar.es
# ssh -X a816778@berlin.unizar.es

# scp -r /home/erika/Descargas/Master/Programming_and_Architecture_of_Computing_Systems/PACS_MASTER/p6/images a816778@central.unizar.es:/home/a816778/master/Programming_And_Architecture_Practicas/p6
# scp -r  a816778@central.unizar.es:/home/a816778/master/Programming_And_Architecture_Practicas/p6/results/images /home/erika/Descargas/Master/Programming_and_Architecture_of_Computing_Systems/PACS_MASTER/p6/results
scp /home/erika/Descargas/Master/Programming_and_Architecture_of_Computing_Systems/PACS_MASTER/p6/kernel.cl a816778@central.unizar.es:/home/a816778/master/Programming_And_Architecture_Practicas/p6/

scp /home/erika/Descargas/Master/Programming_and_Architecture_of_Computing_Systems/PACS_MASTER/p6/platformsAndDeviced.cpp a816778@central.unizar.es:/home/a816778/master/Programming_And_Architecture_Practicas/p6/


# CORRECCIONES DE P5

## HECHAS
# Makefile 
# Test en otras imágenes
# Kernel should have been moved to local memory

# Memoria: Decir plataforma

# ------------------------------------------------
## SIN HACER
# Esperar eventos completos para medir tiempo
# bandwidth


# CL_DEVICE_MAX_COMPUTE_UNITS: 72
# CL_DEVICE_MAX_CLOCK_FREQUENCY: 1695 MHz
# CL_DEVICE_GLOBAL_MEM_SIZE: 22617.81 MB
# CL_DEVICE_LOCAL_MEM_SIZE: 48.00 KB
# CL_DEVICE_MAX_WORK_GROUP_SIZE: 1024
# CL_DEVICE_GLOBAL_MEM_CACHE_SIZE: 2016.00 KB
# CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE: 128 Bytes


# 5000 IMAGES
# paralel_option == SIMPLE
# GPU 0 Load: 3282165760
# GPU 1 Load: 3193254400
################ GPU 0 ##################
# Total transfer time to GPU_0: 1647.82 ms
# Total kernel execution time GPU_0: 89.6366 ms
# Total transfer time from GPU_0: 2463.4 ms
# Total processing time on GPU_0: 13465.6 ms
# Total transfer time to GPU_0: 1689.59 ms
# Total kernel execution time GPU_0: 90.8204 ms
# Total transfer time from GPU_0: 2541.96 ms
# GPU_0 - Write: 39.0894%
# GPU_0 - Kernel: 2.10117%
# GPU_0 - Read: 58.8094%
# GPU_0 Average Write Time: 0.676648 ms per image
# GPU_0 Average Kernel Time: 0.0363718 ms per image
# GPU_0 Average Read Time: 1.018 ms per image
# Total processing time on GPU_0: 13616.3 ms
################ GPU 1 ##################
# Total transfer time to GPU_1: 1953.58 ms
# Total kernel execution time GPU_1: 86.6592 ms
# Total transfer time from GPU_1: 3624.77 ms
# GPU_1 - Write: 34.485%
# GPU_1 - Kernel: 1.52973%
# GPU_1 - Read: 63.9852%
# GPU_1 Average Write Time: 0.782371 ms per image
# GPU_1 Average Kernel Time: 0.0347053 ms per image
# GPU_1 Average Read Time: 1.45165 ms per image
# Total processing time on GPU_1: 15543.5 ms

# GPU 0 Load: 3.28217e+09
# GPU 1 Load: 3.19325e+09
# GPU_0 processes 2497 images.
# GPU_1 processes 2497 images.
# Workload imbalance: 2.70892%
# Total processing time: 16881.1 ms


# paralel_option == ONE_DEVICE
# Total transfer time to GPU_10: 3440.22 ms
# Total kernel execution time GPU_10: 179.935 ms
# Total transfer time from GPU_10: 5258.22 ms
# GPU_10 - Write: 38.7483%
# GPU_10 - Kernel: 2.02666%
# GPU_10 - Read: 59.225%
# GPU_10 Average Write Time: 0.688871 ms per image
# GPU_10 Average Kernel Time: 0.0360302 ms per image
# GPU_10 Average Read Time: 1.05291 ms per image
# Total processing time on GPU_10: 26516.8 ms
# Total processing time: 26517.1 ms

# paralel_option == SIMPLE_BALANCED
# GPU 0 Load: 3237710080
# GPU 1 Load: 3237710080
# Total processing time on GPU: 13781.3 ms
# Total processing time on GPU: 15477.4 ms
# Total processing time: 17959.6 m
