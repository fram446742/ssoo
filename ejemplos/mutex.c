#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 5

pthread_mutex_t mutex; // Declaración del mutex
int shared_counter = 0; // Variable compartida

void* increment_counter(void* thread_id) {
    long tid = (long)thread_id;

    // Bloquear el mutex antes de acceder a la sección crítica
    pthread_mutex_lock(&mutex);

    // Sección crítica
    printf("Thread %ld: incrementing counter\n", tid);
    shared_counter++;
    printf("Thread %ld: shared_counter = %d\n", tid, shared_counter);

    // Desbloquear el mutex después de salir de la sección crítica
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    pthread_mutex_init(&mutex, NULL); // Inicializar el mutex

    // Crear hilos
    for (long i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, increment_counter, (void*)i);
    }

    // Esperar a que los hilos terminen
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Destruir el mutex
    pthread_mutex_destroy(&mutex);

    printf("Final shared_counter = %d\n", shared_counter);
    return 0;
}
