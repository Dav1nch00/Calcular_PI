#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <omp.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Algoritmo de Agujas de Buffon paralelo con OpenMP.
// Cada hilo usa su propio generador rand_r (thread-safe, sin lock interno)
// y acumula hits localmente; al final se hace una reducción sin false sharing.
double estimate_pi(long num_needles) {
    long hits = 0;

    #pragma omp parallel reduction(+:hits)
    {
        // Semilla única por hilo para evitar correlación entre secuencias
        unsigned int seed = (unsigned int)(time(NULL)) ^
                            (unsigned int)(omp_get_thread_num() * 2654435761u);

        // Acumulador local del hilo (pila/registro, sin false sharing)
        long local_hits = 0;

        #pragma omp for schedule(static)
        for (long i = 0; i < num_needles; i++) {
            double x = (double)rand_r(&seed) / RAND_MAX * 0.5;
            double theta = (double)rand_r(&seed) / RAND_MAX * (M_PI / 2.0);

            if (x <= 0.5 * sin(theta)) {
                local_hits++;
            }
        }

        hits += local_hits;
    }

    if (hits == 0) return 0.0;

    // pi ≈ (2 * L * N) / (d * hits), con L = d = 1
    return 2.0 * (double)num_needles / (double)hits;
}

long double calculate_pi_error(double pi_estimate) {
    const long double pi_reference =
        3.1415926535897932384626433832795028841971L;
    return fabsl((long double)pi_estimate - pi_reference);
}

int main() {
    long num_needles;
    long requested_threads;

    printf("Ingrese el número de agujas a lanzar: ");
    if (scanf("%ld", &num_needles) != 1 || num_needles <= 0) {
        printf("Entrada inválida. Debe ser un número entero positivo.\n");
        return 1;
    }

    printf("Ingrese el número de hilos: ");
    if (scanf("%ld", &requested_threads) != 1 ||
        requested_threads <= 0 || requested_threads > INT_MAX) {
        printf("Entrada inválida. Debe ser un número entero positivo.\n");
        return 1;
    }
    omp_set_num_threads((int)requested_threads);

    // Semilla global (se mantiene por consistencia; no se usa rand() en el cálculo)
    srand((unsigned)time(NULL));

    double t0 = omp_get_wtime();
    double pi_estimate = estimate_pi(num_needles);
    double t1 = omp_get_wtime();

    long double absolute_error = calculate_pi_error(pi_estimate);
    const long double pi_reference =
        3.1415926535897932384626433832795028841971L;
    long double relative_error = absolute_error / pi_reference * 100.0L;

    printf("Valor encontrado de PI: %.15f\n", pi_estimate);
    printf("Tiempo de ejecución: %.6f segundos\n", t1 - t0);
    printf("Diferencia absoluta respecto a PI de referencia: %.4Lf\n",
           absolute_error);
    printf("Error relativo: %.4Lf%%\n", relative_error);

    return 0;
}
