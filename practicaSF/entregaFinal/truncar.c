/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ficheros.h" //fer include del més nou, per al nivell6, "ficheros.h"

int imprimir_estado(struct STAT *estado);

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

    struct STAT estado;
    mi_stat_f(ninodo, &estado);

    printf("\nDATOS INODO: %d\n", ninodo);
    imprimir_estado(&estado);

    

    // desmontamos disco
    if (bumount() == FALLO)
    {
        return FALLO;
    }
    return EXITO;
}

int imprimir_estado(struct STAT *estado)
{
    fprintf(stdout, "tipo: %c\n", estado->tipo);
    fprintf(stdout, "permisos: %d\n", estado->permisos);
    fprintf(stdout, "atime: %s", ctime(&estado->atime));
    fprintf(stdout, "ctime: %s", ctime(&estado->ctime));
    fprintf(stdout, "mtime: %s", ctime(&estado->mtime));
    fprintf(stdout, "nlinks: %d\n", estado->nlinks);
    fprintf(stdout, "tamEnBytesLog: %d\n", estado->tamEnBytesLog);
    fprintf(stdout, "numBloquesOcupados: %d\n", estado->numBloquesOcupados);

    return EXITO;
}