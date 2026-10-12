#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

// Estructura para pasar argumentos a los hilos
typedef struct {
    int id_hilo;
    size_t indice_inicio;
    size_t indice_fin;
    double *matriz;
    int iteraciones_computo;
} datos_hilo_t;

// Función que simula una carga computacional intensiva
void aplicar_carga(double *valor, int iteraciones) {
    double temporal = *valor;
    for (int i = 0; i < iteraciones; i++) {
        temporal = sin(temporal) * cos(temporal) + 1.0;
    }
    *valor = temporal;
}

// Función ejecutada por cada hilo (Fase Paralela)
void* tarea_paralela(void* argumento) {
    datos_hilo_t *datos = (datos_hilo_t*)argumento;
    for (size_t i = datos->indice_inicio; i < datos->indice_fin; i++) {
        aplicar_carga(&(datos->matriz[i]), datos->iteraciones_computo);
    }
    return NULL;
}

// Función para calcular la diferencia de tiempo en segundos
double obtener_tiempo(struct timespec inicio, struct timespec fin) {
    return (fin.tv_sec - inicio.tv_sec) + (fin.tv_nsec - inicio.tv_nsec) / 1e9;
}

int main(int numero_argumentos, char *argumentos[]) {
    if (numero_argumentos != 5) {
        fprintf(stderr, "Uso: %s <Tamano_Matriz_N> <Iteraciones_Computo> <Fraccion_Secuencial_0_a_1> <Num_Hilos>\n", argumentos[0]);
        fprintf(stderr, "Ejemplo: %s 1000 500 0.1 4\n", argumentos[0]);
        return 1;
    }

    // 1. Leer parámetros
    size_t N = (size_t)atol(argumentos[1]);
    int iteraciones_computo = atoi(argumentos[2]);
    double fraccion_secuencial = atof(argumentos[3]);
    int numero_hilos = atoi(argumentos[4]);

    if (fraccion_secuencial < 0.0 || fraccion_secuencial > 1.0) {
        fprintf(stderr, "Error: La fracción secuencial debe estar entre 0.0 y 1.0\n");
        return 1;
    }
    if (numero_hilos < 1) numero_hilos = 1;

    size_t elementos_totales = N * N;
    size_t elementos_secuenciales = (size_t)(elementos_totales * fraccion_secuencial);
    size_t elementos_paralelos = elementos_totales - elementos_secuenciales;

    // 2. Inicializar matriz (Conjunto de datos)
    double *matriz = (double*)malloc(elementos_totales * sizeof(double));
    if (!matriz) {
        fprintf(stderr, "Error al asignar memoria para la matriz.\n");
        return 1;
    }
    for (size_t i = 0; i < elementos_totales; i++) {
        matriz[i] = (double)(i % 100) / 100.0;
    }

    struct timespec inicio_total, fin_total, inicio_sec, fin_sec, inicio_par, fin_par;

    // --- INICIO DE LA SIMULACIÓN ---
    clock_gettime(CLOCK_MONOTONIC, &inicio_total);

    // FASE 1: Trabajo Secuencial
    clock_gettime(CLOCK_MONOTONIC, &inicio_sec);
    for (size_t i = 0; i < elementos_secuenciales; i++) {
        aplicar_carga(&matriz[i], iteraciones_computo);
    }
    clock_gettime(CLOCK_MONOTONIC, &fin_sec);

    // FASE 2: Trabajo Paralelo
    clock_gettime(CLOCK_MONOTONIC, &inicio_par);
    pthread_t *hilos = (pthread_t*)malloc(numero_hilos * sizeof(pthread_t));
    datos_hilo_t *datos_hilos = (datos_hilo_t*)malloc(numero_hilos * sizeof(datos_hilo_t));

    size_t tamano_bloque = elementos_paralelos / numero_hilos;
    size_t resto_bloque = elementos_paralelos % numero_hilos;
    size_t indice_actual = elementos_secuenciales;

    for (int i = 0; i < numero_hilos; i++) {
        datos_hilos[i].id_hilo = i;
        datos_hilos[i].indice_inicio = indice_actual;
        
        size_t elementos_para_este_hilo = tamano_bloque + (i < resto_bloque ? 1 : 0);
        datos_hilos[i].indice_fin = indice_actual + elementos_para_este_hilo;
        datos_hilos[i].matriz = matriz;
        datos_hilos[i].iteraciones_computo = iteraciones_computo;
        
        indice_actual = datos_hilos[i].indice_fin;

        pthread_create(&hilos[i], NULL, tarea_paralela, (void*)&datos_hilos[i]);
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < numero_hilos; i++) {
        pthread_join(hilos[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &fin_par);

    clock_gettime(CLOCK_MONOTONIC, &fin_total);
    // --- FIN DE LA SIMULACIÓN ---

    // 3. Imprimir resultados en formato CSV para fácil tabulación
    double t_sec = obtener_tiempo(inicio_sec, fin_sec);
    double t_par = obtener_tiempo(inicio_par, fin_par);
    double t_total = obtener_tiempo(inicio_total, fin_total);

    // Formato: N, Iteraciones, Fraccion_Secuencial, Hilos, Tiempo_Sec, Tiempo_Par, Tiempo_Total
    printf("%zu,%d,%.2f,%d,%.6f,%.6f,%.6f\n", 
           N, iteraciones_computo, fraccion_secuencial, numero_hilos, t_sec, t_par, t_total);

    free(hilos);
    free(datos_hilos);
    free(matriz);
    return 0;
}