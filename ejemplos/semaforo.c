#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_THREADS 5

sem_t semaforo; // Declaración del semáforo
int recurso_compartido = 0; // Variable compartida

void* acceder_recurso(void* thread_id) {
    long tid = (long)thread_id;

    // Intentar adquirir el semáforo
    sem_wait(&semaforo);
    printf("Hilo %ld ha adquirido el semáforo.\n", tid);

    // Sección crítica
    recurso_compartido++;
    printf("Hilo %ld ha incrementado el recurso compartido a %d.\n", tid, recurso_compartido);

    // Simular trabajo
    sleep(1);

    // Liberar el semáforo
    sem_post(&semaforo);
    printf("Hilo %ld ha liberado el semáforo.\n", tid);

    return NULL;
}

int main() {
    pthread_t hilos[NUM_THREADS];

    // Inicializar el semáforo con un valor inicial de 2
    sem_init(&semaforo, 0, 2);

    // Crear hilos
    for (long i = 0; i < NUM_THREADS; i++) {
        pthread_create(&hilos[i], NULL, acceder_recurso, (void*)i);
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(hilos[i], NULL);
    }

    // Destruir el semáforo
    sem_destroy(&semaforo);

    printf("Acceso final al recurso compartido: %d\n", recurso_compartido);
    return 0;
}
