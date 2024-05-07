/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "directorios.h"

//#define BLOCKLEER  (4 * BLOCKSIZE)
#define BLOCKCAT  1500
#define DEBUG9 1

int main(int argc, char **argv)
{
   // signal(SIGABRT, errorExit);
    char buff[BLOCKCAT];
    memset(buff, '\0', sizeof(buff));

    // comprobamos argumentos de consola
    if (argc < 3)
    {
        fprintf(stderr, RED "ARGUMENTOS INSUFICIENTES--> Sintaxis: mi_cat <disco> </ruta_fichero> \n" RESET);
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
        fprintf(stderr, RED "ERROR: mi_cat.c: No se ha podido leer SB\n" RESET);
        // Desmontar el disco antes de salir
        if (bumount() == FALLO)
        {
            return FALLO;
        }

        return FALLO;
    }

    // Verificar si la ruta termina con '/'
    char *ruta = *(argv + 2);
    int longitud_ruta = strlen(ruta);

    if (*(ruta + longitud_ruta - 1) == '/')
    {
        fprintf(stderr, RED "ERROR: mi_cat.c: La ruta especificada no se corresponde a un fichero\n" RESET);
        // Desmontar el disco antes de salir
        if (bumount() == FALLO)
        {
            return FALLO;
        }

        return FALLO;
    }

    struct STAT fichero;
    mi_stat(ruta, &fichero);

    unsigned int final_f = fichero.tamEnBytesLog;
    unsigned int bytesLeidos = 0;
    unsigned int cont_bytes = 0;

    for (int offset = 0; offset <= final_f; offset += BLOCKCAT)
    {
        cont_bytes = mi_read(ruta, buff, offset, BLOCKCAT); 
        /* fprintf(stderr, GRAY "----------------: \n");
        fprintf(stderr, GRAY "%d-%d-%d-%d-: \n", cont,cont_bytes,final_f,i);*/

        if (cont_bytes == FALLO)
        {
            break;
        }

        bytesLeidos += cont_bytes;
        fwrite(buff, 1, cont_bytes, stdout);
    }

    // desmontamos disco
    if (bumount() == FALLO)
    {
        return FALLO;
    }

#if DEBUG9
    fprintf(stderr, GRAY "total_leidos: %d\n", bytesLeidos);
    fprintf(stderr, GRAY "tamEnBytesLog: %d\n", fichero.tamEnBytesLog);
#endif

    return EXITO;
}