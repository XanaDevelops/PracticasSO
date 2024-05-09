/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "directorios.h"

void imprimir_stat(struct STAT *p_stat);

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

    // Llamada a mi_stat de la capa de directorios
    mi_stat(*(argv + 2), &p_stat);
    imprimir_stat(&p_stat);

    // desmontamos disco
    if (bumount() == FALLO)
    {
        return FALLO;
    }
}

// AUXILIAR
/**
 * imprime todos los parametros de struct STAT
 *
 */
void imprimir_stat(struct STAT *p_stat)
{
    fprintf(stdout, "tipo: %c\n", p_stat->tipo);
    fprintf(stdout, "permisos: %d\n", p_stat->permisos);
    fprintf(stdout, "atime: %s", ctime(&p_stat->atime));
    fprintf(stdout, "ctime: %s", ctime(&p_stat->ctime));
    fprintf(stdout, "mtime: %s", ctime(&p_stat->mtime));
    fprintf(stdout, "nlinks: %d\n", p_stat->nlinks);
    fprintf(stdout, "tamEnBytesLog: %d\n", p_stat->tamEnBytesLog);
    fprintf(stdout, "numBloquesOcupados: %d\n", p_stat->numBloquesOcupados);
}