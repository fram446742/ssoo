#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *file; // Declaración de un puntero a FILE para manejar el archivo
    double number; // Variable para almacenar cada número leído del archivo
    long double sum = 0; // Variable para almacenar la suma de los números, inicializada en 0

    // Abre el archivo "numeros.txt" en modo lectura ("r")
    file = fopen("numeros.txt", "r");
    if (file == NULL) { // Verifica si el archivo se abrió correctamente
        fprintf(stderr, "Could not open file numeros.txt\n"); // Imprime un mensaje de error si no se pudo abrir el archivo
        return 1; // Termina el programa con un código de error
    }

    // Lee números del archivo hasta llegar al final del archivo (EOF)
    while (fscanf(file, "%lf", &number) != EOF) {
        sum += number; // Suma cada número leído a la variable sum
    }

    fclose(file); // Cierra el archivo

    // Imprime la suma total con una precisión de 50 decimales
    printf("The sum is: %Lf\n", sum);
    return 0; // Termina el programa exitosamente
}