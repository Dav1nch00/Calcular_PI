#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <limits.h>
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

long double calculate_pi_error(double pi_estimate) {
    const long double pi_reference =
        3.1415926535897932384626433832795028841971L;
    return fabsl((long double)pi_estimate - pi_reference);
}

int main() {
    long num_points;
    long requested_threads;

    printf("Ingrese el número de puntos a lanzar: ");
    if (scanf("%ld", &num_points) != 1 || num_points <= 0) {
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

    // Semilla global (no se usa rand() en el cálculo, pero se mantiene por consistencia)
    srand((unsigned)time(NULL));

    double t0 = omp_get_wtime();
    double pi_estimate = estimate_pi(num_points);
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