# Reto 2 — Cálculo de π con Paralelismo OpenMP

**Curso:** HPC — Universidad  
**Repositorio:** https://github.com/Dav1nch00/Calcular_PI  
**Carpeta de trabajo:** Raíz del proyecto (`reto 2/`)  
**Entorno de ejecución:** WSL (Ubuntu), gcc 15.2.0, OpenMP (libgomp1), 12 núcleos CPU

---

## 1. Análisis de los algoritmos

Se implementaron y analizaron dos algoritmos Monte Carlo para estimar π:

### 1.1 Dartboard (Lanzamiento de dardos)

Se generan puntos `(x, y)` uniformes dentro del cuadrado `[-1, 1] × [-1, 1]`
y se cuenta cuántos caen dentro del círculo unitario `x² + y² ≤ 1`.

Fórmula:
```
π ≈ 4 × (puntos_dentro_del_círculo / total_de_puntos)
```

- Operaciones por iteración: 2 números aleatorios, 2 multiplicaciones, 1 suma, 1 comparación.
- Muy ligero computacionalmente; limitado principalmente por la generación de números aleatorios (`rand()` / `rand_r()`).

### 1.2 Monte Carlo Needles (Agujas de Buffon)

Se lanzan agujas de largo `L = 1` sobre un plano con líneas paralelas separadas por distancia `d = 1`.
Para cada aguja se genera:
- `x`: distancia del centro de la aguja a la línea más cercana, en `[0, 0.5]`.
- `θ`: ángulo de la aguja respecto a las líneas, en `[0, π/2]`.

La aguja cruza una línea si `x ≤ 0.5 × sin(θ)`.

Fórmula:
```
π ≈ (2 · L · N) / (d · hits)   →   con L = d = 1:   π ≈ 2 · N / hits
```

- Operaciones por iteración: 2 números aleatorios, 1 seno (`sin()`), 1 multiplicación, 1 comparación.
- Más costoso por iteración que Dartboard debido a la evaluación de `sin()`.

---

## 2. Versiones seriales implementadas

| Archivo | Algoritmo | Compilación | Medición de tiempo |
|---------|-----------|-------------|--------------------|
| `Dartboard/Dartboard.c` | Dartboard serial | `gcc -O0` | `clock()` |
| `MonteCarlo_Needles/needles_serial.c` | Needles serial | `gcc -O0` | `clock()` |

Ambas versiones:
- Leen `N` por stdin.
- Inicializan `srand(time(NULL))`.
- Imprimen valor estimado de π, tiempo de ejecución, error absoluto y error relativo respecto a π de referencia.

---

## 3. Paralelización con OpenMP y optimizaciones CPU / Memoria

### 3.1 Estrategia de paralelización (ambos algoritmos)

Ambos algoritmos son **embarazosamente paralelos**: cada iteración es independiente, no hay dependencias entre lanzamientos.

Se aplicó en las versiones optimizadas (`*_optimizado.c`):

| Técnica | Descripción | Beneficio |
|---------|-------------|-----------|
| `#pragma omp parallel` | Crea el equipo de hilos | Aprovecha múltiples núcleos |
| `#pragma omp for schedule(static)` | Distribuye iteraciones equitativamente entre hilos | Carga balanceada (iteraciones uniformes) |
| `reduction(+:variable)` | Combina acumuladores locales al final sin sección crítica | Evita race conditions y sobrecarga de locks |
| Acumulador local por hilo (`local_inside` / `local_hits`) | Variable en pila/registro por hilo | Evita **false sharing** entre hilos |
| `rand_r(&seed)` con semilla distinta por hilo | Generador thread-safe, sin lock interno de `rand()` | Elimina contención en el generador serial |
| Semilla por hilo: `time(NULL) ^ (tid × 2654435761u)` | Semillas decorrelacionadas | Evita secuencias aleatorias idénticas entre hilos |
| `omp_get_wtime()` | Medición de tiempo real (wall-clock) | Más preciso que `clock()` (CPU-time) |
| `-O3` | Optimización del compilador (vectorización, inline) | Mejor uso de la CPU |

### 3.2 Optimizaciones a nivel de CPU

- **Vectorización e inlining:** compilación con `-O3` permite al compilador vectorizar operaciones del bucle interno.
- **Reducción de trabajo dentro de la región paralela:** constantes pre-calculadas fuera del `#pragma omp for`.
- **Medición precisa:** `omp_get_wtime()` en lugar de `clock()`, que mide CPU-time acumulado y no refleja el paralelismo real de pared de reloj.

### 3.3 Optimizaciones a nivel de memoria

- **Sin false sharing:** cada hilo acumula en su propia variable local (`local_hits`), evitando que múltiples hilos escriban en la misma línea de caché.
- **Sin allocaciones dentro de la región paralela:** toda la memoria se usa en pila/registro.
- **Localidad de caché:** `schedule(static)` asigna rangos contiguos de iteraciones a cada hilo, mejorando la localidad.
- **Generador thread-safe por hilo:** `rand_r` evita el lock global de `rand()`, reduciendo contención de memoria en el generador serial.

---

## 4. Benchmarks y resultados

Entorno: WSL Ubuntu, 12 núcleos, 50 millones de iteraciones por ejecución.

### 4.1 Monte Carlo Needles (50M agujas, 10 repeticiones)

| Configuración | Tiempo medio (s) | Speedup vs serial | Eficiencia |
|---------------|------------------|-------------------|------------|
| Serial (-O0) | ~3.69 | 1.00× | 100% |
| 1 hilo (-O3) | ~2.88 | 1.28× | 128% |
| 2 hilos | ~1.49 | 2.48× | 124% |
| 4 hilos | ~0.63 | 5.86× | 146% |
| 8 hilos | ~0.27 | 13.67× | 171% |
| 12 hilos | ~0.22 | 16.77× | 140% |

### 4.2 Dartboard (50M puntos)

| Configuración | Tiempo (s) | Speedup vs serial | Eficiencia |
|---------------|------------|-------------------|------------|
| Serial (-O0) | ~0.82 | 1.00× | 100% |
| 1 hilo (-O3) | ~0.50 | 1.64× | 164% |
| 2 hilos | ~0.25 | 3.28× | 164% |
| 4 hilos | ~0.13 | 6.31× | 158% |
| 8 hilos | ~0.07 | 11.71× | 146% |
| 12 hilos | ~0.08 | 10.25× | 85% |

> Nota: los speedups superiores a 1× en "1 hilo (-O3)" se deben a que la versión serial se compila con `-O0` y la optimizada con `-O3`; el compilador aplica vectorización y optimizaciones que reducen el tiempo incluso con un solo hilo.

### 4.3 Scripts de benchmark

- `Dartboard/benchmark.sh` — ya existía, compara serial vs OpenMP con 1, 2, 4, 8, 12 hilos.
- `MonteCarlo_Needles/benchmark_needles.sh` — nuevo, análogo al de Dartboard.

Los resultados detallados se guardan en carpetas `resultados_benchmark_*` generadas automáticamente por cada script.

---

## 5. Análisis de causas del desempeño

### 5.1 ¿Por qué Needles es más lento que Dartboard?

Needles ejecuta `sin()` en cada iteración, una función trascendental costosa. Dartboard solo usa aritmética básica. Por ello, para el mismo `N`, Needles tarda aproximadamente 4–5× más que Dartboard.

### 5.2 Escalabilidad y saturación

- Ambos algoritmos escalan casi linealmente hasta **8 hilos**.
- Con **12 hilos** el speedup se estanca o incluso degrada ligeramente (Dartboard pasa de 0.07s a 0.08s). Causas:
  - Overhead de creación/sincronización de hilos.
  - Contención en la reducción final (`reduction`).
  - Saturación del ancho de banda de memoria y de los puertos de ejecución de la CPU.
  - El equipo tiene 12 núcleos; usar exactamente 12 hilos no garantiza mejoras si el trabajo por hilo es muy pequeño o hay ruido del sistema.

### 5.3 Cuello de botella

El profiling con `gprof` sobre la versión serial de Needles muestra que el **99.19% del tiempo se concentra en `estimate_pi`**, es decir, en el bucle principal de generación de aleatorios y evaluación de la condición. No hay funciones secundarias significativas; la optimización debe centrarse en ese bucle (que ya está paralelo con OpenMP).

### 5.4 Calidad de la estimación de π

Ambos algoritmos convergen al valor de π con error relativo menor al 0.03% para 50 millones de iteraciones. Needles tiene una varianza ligeramente mayor por la naturaleza del problema de Buffon, pero ambos son aceptables para fines educativos de HPC.

---

## 6. Conclusiones

1. **Ambos algoritmos son paralelizables con OpenMP de forma casi trivial** gracias a su independencia entre iteraciones.
2. **OpenMP + `-O3` brinda speedups importantes** (hasta ~16× en Needles y ~12× en Dartboard con 8–12 hilos).
3. **Las optimizaciones de memoria (acumuladores locales, `rand_r`, sin false sharing) son esenciales** para evitar contención y escalar correctamente.
4. **Needles es computacionalmente más costoso** que Dartboard por el uso de `sin()`, pero ambos escalan de forma similar.
5. **El punto dulce está en 8 hilos** en este equipo de 12 núcleos; más allá se observa saturación y overhead.

---

## 7. Estructura del repositorio

```
reto 2/
├── Dartboard/
│   ├── Dartboard.c                 (serial)
│   ├── Dartboard_optimizado.c      (OpenMP)
│   ├── benchmark.sh
│   └── resultados_benchmark_*/     (salida de benchmarks)
├── MonteCarlo_Needles/
│   ├── needles_serial.c            (serial, NUEVO)
│   ├── needles_optimizado.c        (OpenMP, NUEVO)
│   ├── benchmark_needles.sh        (script, NUEVO)
│   └── resultados_benchmark_*/     (salida de benchmarks)
├── reporte.md                      (este documento, NUEVO)
└── README.md                       (instrucciones, NUEVO)
```

---

## 8. Cómo compilar y ejecutar

Desde WSL (Ubuntu) o Linux:

```bash
# Navegar a la carpeta del algoritmo deseado (desde la raíz del proyecto)
cd MonteCarlo_Needles

# Compilar serial
gcc -O0 needles_serial.c -o needles_serial -lm

# Compilar OpenMP optimizado
gcc -O3 -fopenmp needles_optimizado.c -o needles_omp -lm

# Ejecutar serial (ingresar N por teclado)
echo 50000000 | ./needles_serial

# Ejecutar OpenMP (ingresar N y número de hilos)
printf '50000000\n8\n' | ./needles_omp

# Ejecutar benchmark automático
POINTS=50000000 bash benchmark_needles.sh
```

Igual para `Dartboard/` ingresando con `cd Dartboard` y utilizando `Dartboard.c`, `Dartboard_optimizado.c` y `benchmark.sh`.
