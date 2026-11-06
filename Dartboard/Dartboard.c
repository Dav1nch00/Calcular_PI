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

int main() {
    long num_points;

    printf("Ingrese el número de puntos a lanzar: ");
    if (scanf("%ld", &num_points) != 1 || num_points <= 0) {
        printf("Entrada inválida. Debe ser un número entero positivo.\n");
        return 1;
    }

    // Inicializar la semilla para números aleatorios
    srand((unsigned)time(NULL));

    double pi_estimate = estimate_pi(num_points);
    printf("Estimación de π usando %ld puntos: %.10f\n", num_points, pi_estimate);

    return 0;
}