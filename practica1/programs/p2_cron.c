#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int main(int argc, char *argv[])
{
    __pid_t pid = fork();

    if (pid < 0) // Error en fork()
    {
        perror("Error creating child process\n");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) // Hijo
    {
        if (execv("/home/franc/ssoo/p2_extra_cron", argv) < 0)
        {
            perror("Error in child process\n");
            exit(EXIT_FAILURE);
        }
    }
    else // Padre
    {
        wait(NULL);
        sleep(3);
        printf("Hola mundo! Soy el proceso padre!\n");
    }

    // Print timestamp
    printf("Timestamp: %ld\n", time(NULL));

    exit(EXIT_SUCCESS);
}