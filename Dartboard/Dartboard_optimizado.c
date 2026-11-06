#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>

// Estimar π con OpenMP. Cada hilo usa su propio generador rand_r (thread-safe)
// y acumula localmente; la reducción combina al final sin false sharing.
double estimate_pi(long num_points) {
    long inside_circle = 0;

    #pragma omp parallel reduction(+:inside_circle)
    {
        // Semilla única por hilo: base + id (evita correlación entre hilos)
        unsigned int seed = (unsigned int)(time(NULL)) ^ (unsigned int)(omp_get_thread_num() * 2654435761u);

        // Acumulador local del hilo (en registro/pila, sin false sharing)
        long local_inside = 0;

        #pragma omp for schedule(static)
        for (long i = 0; i < num_points; i++) {
            // rand_r es thread-safe y evita el lock interno de rand()
            double x = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;
            double y = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;

            if ((x * x + y * y) <= 1.0) {
                local_inside++;
            }
        }

        inside_circle += local_inside;
    }

    return 4.0 * (double)inside_circle / (double)num_points;
}

int main() {
    long num_points;

    printf("Ingrese el número de puntos a lanzar: ");
    if (scanf("%ld", &num_points) != 1 || num_points <= 0) {
        printf("Entrada inválida. Debe ser un número entero positivo.\n");
        return 1;
    }

    // Semilla global (no se usa rand() en el cálculo, pero se mantiene por consistencia)
    srand((unsigned)time(NULL));

    double t0 = omp_get_wtime();
    double pi_estimate = estimate_pi(num_points);
    double t1 = omp_get_wtime();

    printf("Estimación de π usando %ld puntos: %.10f\n", num_points, pi_estimate);
    printf("Tiempo de cómputo: %.4f s\n", t1 - t0);

    return 0;
}