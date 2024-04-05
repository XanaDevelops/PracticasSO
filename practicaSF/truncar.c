#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ficheros_basico.h"
#include "ficheros.h"

int main(int argc, char **argv)
{
    // Comprobamos argumentos de consola
    if (argc < 4)
    {
        fprintf(stderr, RED "ARGUMENTOS INSUFICIENTES: Uso: truncar <nombre_dispositivo> <ninodo> <nbytes>\n" RESET);
        return FALLO;
    }

    // Montamos disco
    if (bmount(*(argv + 1)) == FALLO)
    {
        return FALLO;
    }

    // convertimos valores pasados por parámetro a enteros
    int ninodo = atoi(*(argv + 2));
    int nbytes = atoi(*(argv + 3));

    if(nbytes == 0) 
    {
        liberar_inodo(ninodo);
    } 
    else 
    {
        mi_truncar_f(ninodo, nbytes);
    }

    // desmontamos disco
    if (bumount() == FALLO)
    {
        return FALLO;
    }
}