#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    printf("Hola mundo! Soy el proceso hijo!\n");
    printf("Voy a imprimir 10000 veces un mensaje por pantalla\n");

    sleep(1);

    for (long i = 1; i <= 10000; i++)
    {
        printf("Mensaje %ld\n", i);
        sleep(1);
    }

    exit(EXIT_SUCCESS);
}
