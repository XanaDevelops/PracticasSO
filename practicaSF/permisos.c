#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ficheros_basico.h"

int main(int argc, char **argv)
{
    // Comprobamos argumentos de consola
    if (argc < 4)
    {
        fprintf(stderr, RED "ARGUMENTOS INSUFICIENTES: Uso: ./permitir {nombre_dispositivo} {ninodo} {permisos}\n" RESET);
        return FALLO;
    }

    // Montamos disco
    if (bmount(*(argv + 1)) == FALLO)
    {
        return FALLO;
    }
}


