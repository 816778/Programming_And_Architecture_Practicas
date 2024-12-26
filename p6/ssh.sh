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
# Total processing time on GPU: 13139.8 ms
# Total processing time on GPU: 14615.2 ms
# Total processing time: 16121.2 ms


# paralel_option == ONE_DEVICE
# Total processing time on GPU: 25423 ms
# Total processing time: 25423.1 ms

# paralel_option == SIMPLE_BALANCED
# GPU 0 Load: 3237710080
# GPU 1 Load: 3237710080
# Total processing time on GPU: 13511.4 ms
# Total processing time on GPU: 15180.6 ms
# Total processing time: 17701.5 ms
