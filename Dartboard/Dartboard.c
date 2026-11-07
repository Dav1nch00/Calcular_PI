#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Función para estimar π usando el método Dartboard
double estimate_pi(long num_points) {
    long inside_circle = 0;

    for (long i = 0; i < num_points; i++) {
        // Generar coordenadas aleatorias entre -1 y 1
        double x = (double)rand() / RAND_MAX * 2.0 - 1.0;
        double y = (double)rand() / RAND_MAX * 2.0 - 1.0;

        // Verificar si el punto está dentro del círculo de radio 1
        if ((x * x + y * y) <= 1.0) {
            inside_circle++;
        }
    }

    // Fórmula: π ≈ 4 * (puntos dentro / total de puntos)
    return 4.0 * (double)inside_circle / (double)num_points;
}

long double calculate_pi_error(double pi_estimate) {
    const long double pi_reference =
        3.1415926535897932384626433832795028841971L;
    return fabsl((long double)pi_estimate - pi_reference);
}

int main() {
    long num_points;

    printf("Ingrese el número de puntos a lanzar: ");
    if (scanf("%ld", &num_points) != 1 || num_points <= 0) {
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

    double pi_estimate = estimate_pi(num_points);

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