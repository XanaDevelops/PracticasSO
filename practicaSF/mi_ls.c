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
    char tipo;
    char *camino, *disco;
    if (argc == 4)
    {
        if (strcmp(argv[1], "-l") == 0)
        {
            extendido = 1;
            camino = argv[3];
            disco = argv[2];
        }
        else
        {
            fprintf(stderr, RED "ERROR: sintaxis correcta: ./mi_ls -l <disco> </ruta>\n" RESET);
            return FALLO;
        }
    }
    else
    {
        camino = argv[2];
        disco = argv[1];
    }

    if (camino[strlen(camino) - 1] != '/')
    {
        tipo = 'f';
    }
    else
    {
        tipo = 'd';
    }

#if DEBUG8
    fprintf(stderr, GRAY "[main() -> disco:%s, camino:%s, tipo:%c, modo:%d]\n" RESET, disco, camino, tipo, extendido);
#endif
    char buff[NFILAS][TAMFILA];
    // asumiendo leer solo una entrada
    char bufdir[TAMBUFFER];
    memset(bufdir, '\0', sizeof(bufdir));

    // llamar a mi_dir()
    int nEntradas;

    if (bmount(disco) == FALLO)
    {
        fprintf(stderr, RED "ERROR main() -> fallo al bmount argv[2]\n" RESET);
        return FALLO;
    }

    nEntradas = mi_dir(camino, bufdir, tipo, extendido); // placehoder 'd' y 1
    if(nEntradas == FALLO){
        bumount();
        return FALLO;
    }
    // printf("bufdir: %s\n", bufdir);
    char *iter = strtok(bufdir, "|");
    if (extendido)
    {
        if (tipo == 'd')
        {
            fprintf(stdout, "Total: %d\n", nEntradas);
        }
        fprintf(stdout, "Tipo	Modo	mTime			Tamaño			Nombre\n");
        fprintf(stdout, "----------------------------------------------------------------------\n");
    }
    while (iter != NULL)
    {
        printf("%s\n", iter);
        iter = strtok(NULL, "|");
    }

    return EXITO;
}