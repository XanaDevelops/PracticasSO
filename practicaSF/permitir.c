#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ficheros.h"

int main(int argc, char **argv)
{
    // Comprobamos argumentos de consola
    if (argc < 4)
    {
        fprintf(stderr, RED "ARGUMENTOS INSUFICIENTES: Uso: permitir <nombre_dispositivo> <ninodo> <permisos>\n" RESET);
        return FALLO;
    }

    // Montamos disco
    if (bmount(*(argv + 1)) == FALLO)
    {
        return FALLO;
    }
    
    // convertimos valores pasados por parámetro a enteros
    int ninodo = atoi(*(argv + 2));
    int permisos = atoi(*(argv + 3));

    // llamada a chmod
    mi_chmod_f(ninodo, permisos);

    // desmontamos disco
    if (bumount() == FALLO)
    {
        return FALLO;
    }
}


