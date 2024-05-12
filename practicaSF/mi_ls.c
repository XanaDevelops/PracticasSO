/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include <stdio.h>
#include <stdlib.h>

#include "directorios.h"

#define TAMFILA 100
#define NFILAS 1000
#define TAMBUFFER (TAMFILA * NFILAS) // suponemos un máx de 1000 entradas, aunque debería ser SB.totInodos

int main(int argc, char **argv)
{
    // comprobar entrada
    if (!(argc == 3 || argc == 4))
    {
        fprintf(stderr, RED "ERROR: sintaxis correcta: ./mi_ls -l <disco> </ruta>\n" RESET);
        return FALLO;
    }

    int extendido = 0;
    if (argc == 4)
    {
        if (strcmp(argv[1], "-l") == 0)
        {
            extendido = 1;
        }
        else
        {
            fprintf(stderr, RED "ERROR: sintaxis correcta: ./mi_ls -l <disco> </ruta>\n" RESET);
            return FALLO;
        }
    }

    char buff[NFILAS][TAMFILA];
    // asumiendo leer solo una entrada
    char bufdir[TAMNOMBRE * BLOCKSIZE / sizeof(struct entrada)];
    memset(bufdir, '\0', sizeof(bufdir));

    // llamar a mi_dir()
    int nEntradas;
    if (extendido)
    {
        if (bmount(argv[2]) == FALLO)
        {
            fprintf(stderr, RED "ERROR main() -> fallo al bmount argv[2]\n" RESET);
            return FALLO;
        }
        nEntradas = mi_dir(argv[3], bufdir, 'd', 1); // placehoder 'd' y 1
    }
    else
    {
        if (bmount(argv[1]) == FALLO)
        {
            fprintf(stderr, RED "ERROR main() -> fallo al bmount argv[1]\n" RESET);
            return FALLO;
        }
        nEntradas = mi_dir(argv[2], bufdir, 'd', 0);
    }

    printf("ls: %s\n", bufdir);
    char *iter = strtok(bufdir, "|");

    while (iter != NULL)
    {
        printf("entr: %s\n", iter);
        iter = strtok(NULL, "|");
    }

    return EXITO;
}