#!/bin/bash

# ssh -X a816778@central.unizar.es
# ssh -X a816778@berlin.unizar.es

# scp -r /home/erika/Descargas/Master/Programming_and_Architecture_of_Computing_Systems/PACS_MASTER/p6/images a816778@central.unizar.es:/home/a816778/master/Programming_And_Architecture_Practicas/p6
scp /home/erika/Descargas/Master/Programming_and_Architecture_of_Computing_Systems/PACS_MASTER/p6/kernel.cl a816778@central.unizar.es:/home/a816778/master/Programming_And_Architecture_Practicas/p6/

scp /home/erika/Descargas/Master/Programming_and_Architecture_of_Computing_Systems/PACS_MASTER/p6/platformsAndDeviced.cpp a816778@central.unizar.es:/home/a816778/master/Programming_And_Architecture_Practicas/p6/


# CORRECCIONES DE P5

## HECHAS
# Makefile 
# Test en otras imágenes

# Memoria: Decir plataforma

# ------------------------------------------------
## SIN HACER
# Kernel should have been moved to local memory
# Esperar eventos completos para medir tiempo
# bandwidth


# 5000 IMAGES
# paralel_option == SIMPLE
# Total processing time on GPU: 9034.44 ms
# Total processing time on GPU: 9817.33 ms


# paralel_option == ONE_DEVICE
# Total processing time on GPU: 20119.3 ms