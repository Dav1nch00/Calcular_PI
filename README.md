# Calcular PI — Reto 2 HPC

Implementación de dos algoritmos Monte Carlo para estimar π con paralelismo OpenMP:

1. **Dartboard** (lanzamiento de dardos) — ya existente.
2. **Monte Carlo Needles** (Agujas de Buffon) — nuevo.

## Estructura

```
Dartboard/          Dartboard serial y OpenMP
MonteCarlo_Needles/ Needles serial y OpenMP (nuevo)
reporte.md          Análisis, benchmarks y conclusiones
```

## Requisitos

- gcc con OpenMP (`libgomp1`)
- WSL / Linux (los scripts `.sh` están pensados para bash)

## Compilación y ejecución

Desde WSL:

```bash
cd "/mnt/c/Users/erest/Desktop/universidad/HPC/reto 2/MonteCarlo_Needles"

gcc -O0 needles_serial.c -o needles_serial -lm
gcc -O3 -fopenmp needles_optimizado.c -o needles_omp -lm

echo 50000000 | ./needles_serial
printf '50000000\n8\n' | ./needles_omp
```

## Benchmarks

```bash
POINTS=50000000 bash MonteCarlo_Needles/benchmark_needles.sh
POINTS=50000000 bash Dartboard/benchmark.sh
```

## Repositorio remoto

`https://github.com/Dav1nch00/Calcular_PI` (rama `main`)
