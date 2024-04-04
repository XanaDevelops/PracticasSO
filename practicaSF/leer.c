#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ficheros.h"

void errorExit();
int bytes_transf = (4 * BLOCKSIZE);

int main(int argc, char **argv){
    char buff[bytes_transf];
    memset(buff,'\0',sizeof(buff));
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
    }else if (numInodo > SB.totInodos){
        fprintf(stderr, RED "ARGUMENTO INVALIDO: nºINODO < nºtotal Inodos\n" RESET);
        errorExit();
    }

    struct STAT estado;
    mi_stat_f(numInodo, &estado);

    int final_f = estado.tamEnBytesLog;
    int cont = 0;

    for(int i = 0; i < final_f; i += bytes_transf){
       cont += mi_read_f(numInodo, buff, i,  bytes_transf);
       write(1,buff,bytes_transf);
    }

     // desmontamos disco
    if (bumount())
    {
        return FALLO;
    }
   //printf("total_leidos: %d\n", cont);
    return EXITO;
}

void errorExit()
{
    bumount();
    exit(FALLO);
}