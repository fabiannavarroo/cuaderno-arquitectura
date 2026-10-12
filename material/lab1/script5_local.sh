#!/bin/bash

# 1. Compilar el programa C especificando librerías matemáticas y de hilos POSIX
gcc -O3 benchmark.c -o benchmark -lpthread -lm

# 2. Definición de parámetros del experimento
export TAMANO_MATRIZ=2000              # N (Matriz NxN)
export ITERACIONES=100                # Iteraciones de carga computacional
export FRACCION_SEC=0.10              # Fracción secuencial (10% secuencial, 90% paralelizable)
export NUM_HILOS=8 		        # Número de hilos de la ejecución 

echo "=================================================="
echo "EJECUCIÓN DE EVALUACIÓN DE LEY DE AMDAHL"
echo "=================================================="
echo "N,Iteraciones,Fraccion_Secuencial,Hilos,Tiempo_Sec,Tiempo_Par,Tiempo_Total"

# 3. Ejecución del binario pasándole la variable con los cores asignados por Slurm
 salloc -p avignon -N 1 --time=00:05:00 perf stat -a -e power/energy-pkg/ ./benchmark $TAMANO_MATRIZ $ITERACIONES $FRACCION_SEC $NUM_HILOS
