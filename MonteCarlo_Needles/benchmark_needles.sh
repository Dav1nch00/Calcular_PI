#!/usr/bin/env bash
set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
basic_source="$script_dir/needles_serial.c"
optimized_source="$script_dir/needles_optimizado.c"
points=${POINTS:-100000000}
repetitions=5
thread_counts=(1 2 4 8 12)

if ! command -v gcc >/dev/null 2>&1; then
    printf 'Error: se necesita gcc para compilar los programas.\n' >&2
    exit 1
fi

if [[ ! -f "$basic_source" || ! -f "$optimized_source" ]]; then
    printf 'Error: se requieren needles_serial.c y needles_optimizado.c en %s\n' \
        "$script_dir" >&2
    exit 1
fi

results_dir=$(mktemp -d "$script_dir/resultados_benchmark_$(date +%Y%m%d_%H%M%S)_XXXXXX")
build_dir=$(mktemp -d "${TMPDIR:-/tmp}/needles-benchmark.XXXXXX")
trap 'rm -rf -- "$build_dir"' EXIT

basic_bin="$build_dir/needles_basico"
optimized_bin="$build_dir/needles_optimizado"

printf 'Compilando Needles básico (-O0)...\n'
gcc -O0 "$basic_source" -o "$basic_bin" -lm

printf 'Compilando Needles optimizado (-O3, OpenMP)...\n'
gcc -O3 -fopenmp "$optimized_source" -o "$optimized_bin" -lm

basic_results="$results_dir/resultados_basico.txt"
printf 'Versión: básica | Optimización: -O0 | Agujas por ejecución: %s\n' "$points" \
    > "$basic_results"

for ((run = 1; run <= repetitions; run++)); do
    {
        printf '\n===== Ejecución %02d/%02d =====\n' "$run" "$repetitions"
        printf '%s\n' "$points" | "$basic_bin"
    } 2>&1 | tee -a "$basic_results"
done

for thread_count in "${thread_counts[@]}"; do
    optimized_results="$results_dir/resultados_optimizado_${thread_count}_hilos.txt"
    printf 'Versión: optimizada | Optimización: -O3 | Hilos: %s | Agujas por ejecución: %s\n' \
        "$thread_count" "$points" > "$optimized_results"

    for ((run = 1; run <= repetitions; run++)); do
        {
            printf '\n===== Ejecución %02d/%02d | Hilos: %s =====\n' \
                "$run" "$repetitions" "$thread_count"
            printf '%s\n%s\n' "$points" "$thread_count" | "$optimized_bin"
        } 2>&1 | tee -a "$optimized_results"
    done
done

printf '\nBenchmark finalizado. Resultados guardados en:\n%s\n' "$results_dir"
