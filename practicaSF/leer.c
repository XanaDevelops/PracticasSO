/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "directorios.h"

void errorExit();
//#define BLOCKLEER  (4 * BLOCKSIZE)
#define BLOCKLEER  1500

#define DEBUG5 1

int main(int argc, char **argv)
{
   // signal(SIGABRT, errorExit);
    char buff[BLOCKLEER];
    memset(buff, '\0', sizeof(buff));

    // comprobamos argumentos de consola
    if (argc < 3)
    {
        fprintf(stderr, RED "ARGUMENTOS INSUFICIENTES--> Sintaxis: leer <nombre_dispositivo> <ninodo> \n" RESET);
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
        fprintf(stderr, RED "ERROR: leer.c: No se ha podido leer SB\n" RESET);
        errorExit();
    }

    int numInodo = atoi(*(argv + 2));

    if (numInodo < 0)
    {
        fprintf(stderr, RED "ARGUMENTO INVALIDO: nºINODO > 0\n" RESET);
        errorExit();
    }
    else if (numInodo > SB.totInodos)
    {
        fprintf(stderr, RED "ARGUMENTO INVALIDO: nºINODO < nºtotal Inodos\n" RESET);
        errorExit();
    }

    struct STAT estado;
    mi_stat_f(numInodo, &estado);

    unsigned int final_f = estado.tamEnBytesLog;
    unsigned int cont = 0;
    unsigned int cont_bytes = 0;

    for (int i = 0; i <= final_f; i += BLOCKLEER)
    {
     
        cont_bytes = mi_read_f(numInodo, buff, i, BLOCKLEER); 
        /* fprintf(stderr, GRAY "----------------: \n");
        fprintf(stderr, GRAY "%d-%d-%d-%d-: \n", cont,cont_bytes,final_f,i);*/

        if (cont_bytes == FALLO)
        {
            break;
        }

        cont += cont_bytes;
        fwrite(buff, 1, cont_bytes, stdout);
        fflush(stdout);
        memset(buff, '\0', sizeof(buff));
    }

    // desmontamos disco
    if (bumount() == FALLO)
    {
        return FALLO;
    }
#if DEBUG5
    fprintf(stderr, GRAY "\ntotal_leidos: %d\n", cont);
    fprintf(stderr, GRAY "tamEnBytesLog: %d\n", estado.tamEnBytesLog);
#endif

    return EXITO;
}

void errorExit()
{
    bumount();
    exit(FALLO);
}