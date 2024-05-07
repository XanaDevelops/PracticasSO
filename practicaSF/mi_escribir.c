/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "directorios.h"
int  exitError();

int exitError(){
// Desmontar el disco antes de salir
        if (bumount() == FALLO)
        {
            return FALLO;
        }

        return FALLO;

}

int main(int argc, char **argv)
{
    // comprobamos argumentos de consola
    if (argc < 5)
    {
        fprintf(stderr, RED "ARGUMENTOS INSUFICIENTES--> Sintaxis: mi_escribir  <disco> </ruta_fichero> <texto> <offset>\n" RESET);
        return FALLO;
    }

    // montamos disco
    if (bmount(*(argv + 1)) == FALLO)
    {
        return FALLO;
    }

    char *camino = argv[2];
    char *buffer_original = argv[3];
    int offset = atoi(*(argv + 4));
    int buffer_size = strlen(buffer_original);
    int bytes;

    if (*(camino + buffer_size - 1) != '/')
    {
        // Llamada a mi_write de la capa de directorios
        bytes = mi_write(camino, buffer_original, offset, buffer_size);
    }
    else
    {
        fprintf(stderr, RED "RUTA INVÁLIDA: La ruta para mi_write leer fichero no debe terminar con '/'.\n" RESET);
        printf("Bytes Leidos: %d\n", 0);
        return exitError();
    }

    printf("Bytes Leidos: %d\n", bytes);

    // desmontamos disco
    if (bumount() == FALLO)
    {
        return FALLO;
    }

    return EXITO;
}