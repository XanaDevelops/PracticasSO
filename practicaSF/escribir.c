#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ficheros.h"

int main(int argc, char **argv)
{
    int *offsets = {9000, 209000, 30725000, 409605000, 480000000};
    char buffer_original;
    int buffer_size;

    // comprobamos argumentos de consola
    if (argc < 4)
    {
        fprintf(stderr, RED "ARGUMENTOS INSUFICIENTES--> Sintaxis: escribir <nombre_dispositivo> <$(cat fichero)> <diferentes_inodos>\n" RESET);
        return FALLO;
    }

    // comprobamos numero bloques
    if (atoi(*(argv + 3)) == 0)
    {
    }
    else if (atoi(*(argv + 3)) == 1)
    {
    }
    else
    {
        fprintf(stderr, RED "ARGUMENTOS INVALIDOS --> <diferentes_inodos> = 1 || 0 \n" RESET);
        return FALLO;
    };

    int inodoReservado = reservar_inodo('f', 6); // mirar si van bien
    
    if (inodoReservado == FALLO)
    {
        fprintf(stderr, RED "ERROR: leer_sf(): no se ha podido reservar inodo\n" RESET);
        return FALLO;
    }
    
    struct STAT estado;
    mi_stat_f(inodoReservado, &estado);
    
    mi_write_f(inodoReservado, buffer_original, 0, buffer_size);

    mi_stat_f(inodoReservado, &estado);

    memset(buffer_original, '\0', buffer_size);
    mi_read_f(inodoReservado, buffer_original, 0, buffer_size);
    write(1, buffer_original, buffer_size);
}