/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "directorios.h"

int main(int argc, char **argv)
{
    // Comprobamos argumentos de consola
    if (argc < 3)
    {
        fprintf(stderr, RED "ARGUMENTOS INSUFICIENTES: Uso: mi_stat <disco> </ruta>\n" RESET);
        return FALLO;
    }

    // Montamos disco
    if (bmount(*(argv + 1)) == FALLO)
    {
        return FALLO;
    }
    
    struct STAT p_stat;

    // Llamada a mi_stat
    mi_stat(*(argv + 2), &p_stat);

    // desmontamos disco
    if (bumount() == FALLO)
    {
        return FALLO;
    }
}