/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "directorios.h"

int main(int argc, char **argv)
{
    // Comprobar argumentos de consola
    if (argc < 4)
    {
        fprintf(stderr, RED "ARGUMENTOS INSUFICIENTES: Uso: mi_chmod <disco> <permisos> </ruta>\n" RESET);
        return FALLO;
    }

    // Montar disco
    if (bmount(*(argv + 1)) == FALLO)
    {
        return FALLO;
    }
    
    int permisos = atoi(*(argv + 2));

    // Validar que los permisos estén en el rango válido (0-7)
    if(permisos < 0 || permisos > 7)
    {
        fprintf(stderr, RED "PERMISOS INVÁLIDOS: Los permisos deben estar en el rango de 0 a 7.\n" RESET);
        // Desmontar el disco antes de salir
        if (bumount() == FALLO)
        {
            return FALLO;
        }

        return FALLO;
    }

    // Llamada a mi_chmod de la capa de directorios
    mi_chmod(*(argv + 3), (unsigned char) permisos);

    // Desmontar disco
    if (bumount() == FALLO)
    {
        return FALLO;
    }
}