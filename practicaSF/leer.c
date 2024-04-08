#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ficheros.h"

void errorExit();
//int bytes_transf = (4 * BLOCKSIZE);
int bytes_transf = 1500;

#define DEBUG5 1

int main(int argc, char **argv)
{
    char buff[bytes_transf];
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

    int final_f = estado.tamEnBytesLog;
    int cont = 0;
     int cont_bytes = 0;

    // MODIFICA LLEGEIX MES BLOQUES QUE EL KI TOCA
    for (int i = 0; i < final_f; i += bytes_transf)
    {
        cont_bytes = mi_read_f(numInodo, buff, i, bytes_transf);
        if(cont_bytes == FALLO){
            break;
        }
        cont += cont_bytes;
        fwrite(buff, 1, cont_bytes, stdout);
    }

    // desmontamos disco
    if (bumount())
    {
        return FALLO;
    }

#if DEBUG5
    fprintf(stderr, GRAY "total_leidos: %d\n", cont);
    fprintf(stderr, GRAY "tamEnBytesLog: %d\n", estado.tamEnBytesLog);
#endif

    return EXITO;
}

void errorExit()
{
    bumount();
    exit(FALLO);
}