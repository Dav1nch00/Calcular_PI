#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Algoritmo de Agujas de Buffon (Monte Carlo Needles) para estimar PI.
// Agujas de largo L = 1 sobre líneas paralelas separadas por distancia d = 1.
// Una aguja cruza una línea si: x <= (L/2) * sin(theta)
// donde x es la distancia del centro al borde más cercano [0, d/2]
// y theta es el ángulo con las líneas [0, pi/2].
// Fórmula: pi ≈ (2 * L * N) / (d * hits)  =>  con L=d=1: pi ≈ 2 * N / hits
double estimate_pi(long num_needles) {
    long hits = 0;

    for (long i = 0; i < num_needles; i++) {
        // Distancia del centro de la aguja a la línea más cercana: [0, 0.5]
        double x = (double)rand() / RAND_MAX * 0.5;
        // Ángulo de la aguja respecto a las líneas: [0, pi/2]
        double theta = (double)rand() / RAND_MAX * (M_PI / 2.0);

        // ¿La aguja cruza una línea?
        if (x <= 0.5 * sin(theta)) {
            hits++;
        }
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

    printf("Ingrese el número de agujas a lanzar: ");
    if (scanf("%ld", &num_needles) != 1 || num_needles <= 0) {
        printf("Entrada inválida. Debe ser un número entero positivo.\n");
        return 1;
    }

    // Inicializar la semilla para números aleatorios
    srand((unsigned)time(NULL));

    clock_t start_time = clock();
    if (start_time == (clock_t)-1) {
        fprintf(stderr, "No se pudo iniciar la medición del tiempo.\n");
        return 1;
    }

    double pi_estimate = estimate_pi(num_needles);

    clock_t end_time = clock();
    if (end_time == (clock_t)-1) {
        fprintf(stderr, "No se pudo finalizar la medición del tiempo.\n");
        return 1;
    }

    long double absolute_error = calculate_pi_error(pi_estimate);
    const long double pi_reference =
        3.1415926535897932384626433832795028841971L;
    long double relative_error = absolute_error / pi_reference * 100.0L;

    printf("Valor encontrado de PI: %.15f\n", pi_estimate);
    printf("Tiempo de ejecución: %.6f segundos\n",
           (double)(end_time - start_time) / CLOCKS_PER_SEC);
    printf("Diferencia absoluta respecto a PI de referencia: %.4Lf\n",
           absolute_error);
    printf("Error relativo: %.4Lf%%\n", relative_error);

    return 0;
}
