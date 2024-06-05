/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "directorios.h"

int main(int argc, char **argv)
{
    // comprobamos argumentos de consola
    if (argc < 4)
    {
        fprintf(stderr, RED "ARGUMENTOS INSUFICIENTES--> Sintaxis: mi_link <disco> </ruta_fichero_original> </ruta_enlace>\n" RESET);
        return FALLO;
    }

    // montamos disco
    if (bmount(*(argv + 1)) == FALLO)
    {
        return FALLO;
    }

    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_link.c: No se ha podido leer SB\n" RESET);
        // Desmontar el disco antes de salir
        if (bumount() == FALLO)
        {
            return FALLO;
        }

        return FALLO;
    }

    // Verificar si la ruta termina con '/'
    char *ruta_original = *(argv + 2);
    int longitud_ruta_org = strlen(ruta_original);
    char *ruta_enlace = *(argv + 3);
    int longitud_ruta_link = strlen(ruta_enlace);

    if ((*(ruta_original + longitud_ruta_org - 1) == '/') || (*(ruta_enlace + longitud_ruta_link - 1) == '/'))
    {
        fprintf(stderr, RED "ERROR: mi_link.c: Las rutas especificadas no se corresponden a un fichero\n" RESET);
        // Desmontar el disco antes de salir
        if (bumount() == FALLO)
        {
            return FALLO;
        }

        return FALLO;
    }

    mi_link(ruta_original, ruta_enlace);    

    // desmontamos disco
    if (bumount() == FALLO)
    {
        return FALLO;
    }

    return EXITO;
}