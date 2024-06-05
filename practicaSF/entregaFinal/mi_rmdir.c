/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "directorios.h"
int exitError();

int exitError()
{
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
    if (argc < 3)
    {
        fprintf(stderr, RED "ARGUMENTOS INSUFICIENTES--> Sintaxis: mi_rmdir <disco> </ruta>\n" RESET);
        return FALLO;
    }

    // montamos disco
    if (bmount(*(argv + 1)) == FALLO)
    {
        return FALLO;
    }

    char *camino = *(argv + 2);
    int longitud_ruta = strlen(camino);
    if (strcmp(camino, "/") == 0)
    {
        fprintf(stderr, RED "Error: mi_rmdir -> No se ha de poder borrar el directorio raíz\n" RESET);
        return exitError();
    }

    if ((*(camino + longitud_ruta - 1) != '/'))
    {
        fprintf(stderr, RED "ERROR: mi_rmdir(): Las rutas especificadas no se corresponden a un directorio\n" RESET);
        return exitError();
    }
    mi_unlink(camino);

    // Desmontar el disco antes de salir
    if (bumount() == FALLO)
    {
        return FALLO;
    }
    return EXITO;
}