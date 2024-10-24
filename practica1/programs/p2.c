#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    __pid_t pid = fork();

    if (pid < 0) // Error en fork()
    {
        printf("Error creating child process\n");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) // Hijo
    {
        if (execv("./p2_extra", argv) < 0)
        {
            printf("Error in child process\n");
            exit(EXIT_FAILURE);
        }
    }
    else // Padre
    {
        wait(NULL);
        sleep(3);
        printf("Hola mundo! Soy el proceso padre!\n");
    }

    exit(EXIT_SUCCESS);
}