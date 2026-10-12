#!/bin/bash
#SBATCH --job-name=info_nodo
#SBATCH --partition=avignon
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=00:05:00
#SBATCH --output=./output/info_nodo_%j.out
#SBATCH --error=./output/info_nodo_%j.err

echo "=================================================="
echo "INFORMACIÓN DEL NODO ASIGNADO"
echo "=================================================="
echo "Nodo de ejecución : $(hostname)"
echo "ID del Trabajo     : $SLURM_JOB_ID"
echo "=================================================="
echo ""

echo "--- ESPECIFICACIONES DE CPU ---"
lscpu | grep -E "Model name|Architecture|CPU\(s\):|Thread\(s\) per core:|Core\(s\) per socket:|Socket\(s\):"
echo ""

echo "--- MEMORIA RAM DEL NODO ---"
free -h | grep -E "total|Mem:|Swap:"
echo ""
echo "=================================================="
