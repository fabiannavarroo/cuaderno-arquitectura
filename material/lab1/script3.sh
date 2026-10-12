#!/bin/bash
#SBATCH --job-name=speedup
#SBATCH --partition=avignon
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4      # Número de cores/hilos a reservar (p. ej. 8 cores)
#SBATCH --time=00:05:00
#SBATCH --output=./output/speedup_%j.out
#SBATCH --error=./output/speedup_%j.err

# 1. Compilar el programa C especificando librerías matemáticas y de hilos POSIX
gcc -O3 benchmark.c -o benchmark -lpthread -lm

# 2. Definición de parámetros del experimento
TAMANO_MATRIZ=2000              # N (Matriz NxN)
ITERACIONES=100                 # Iteraciones de carga computacional
FRACCION_SEC=0.10               # Fracción secuencial (10% secuencial, 90% paralelizable)
NUM_HILOS=4 		        # Número de hilos de la ejecución 

echo "=================================================="
echo "EJECUCIÓN DE EVALUACIÓN DE LEY DE AMDAHL"
echo "=================================================="
echo "Nodo asignado     : $SLURMD_NODENAME"
echo "Cores reservados  : $SLURM_CPUS_PER_TASK"
echo "=================================================="
echo "N,Iteraciones,Fraccion_Secuencial,Hilos,Tiempo_Sec,Tiempo_Par,Tiempo_Total"

# 3. Ejecución del binario pasándole la variable con los cores asignados por Slurm
./benchmark $TAMANO_MATRIZ $ITERACIONES $FRACCION_SEC $NUM_HILOS
