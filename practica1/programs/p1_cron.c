#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main()
{
    __pid_t pid = fork();

    if (pid != 0)
    {
        for (int i = 0; i < 100; i++)
        {
            printf("Hello number: %d\n", i);
            // sleep(1);
        }
    }
    else if (pid == 0)
    {
        long a;
        for (long i = 0; i <= 1000000; i += 2)
        {
            // printf("The value is: %ld\n", i);
            a += i;
            // sleep(1);
        }
        sleep(3);
        printf("The value is: %ld\n", a);
    }
    else
    {
        printf("Error creating child process\n");
    }

    // Print timestamp
    printf("Timestamp: %ld\n", time(NULL));
}