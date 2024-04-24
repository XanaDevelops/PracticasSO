/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "directorios.h"

#define DEBUG1 0
#define DEBUG3 1

void errorExit();

int main(int argc, char **argv)
{ 
    // comprobamos argumentos de consola
    if (argc < 3)
    {
        fprintf(stderr, RED "ARGUMENTOS INSUFICIENTES: Uso: ./mi_mkfs {nombre_disco} {nº bloques}\n" RESET);
        return FALLO;
    }

    // comprobamos numero bloques
    int nbloque = atoi(*(argv + 2));
#if DEBUG1
    fprintf(stderr, GRAY "nbloques: %d\n" RESET, nbloque);
#endif
    if (nbloque <= 0)
    {
        fprintf(stderr, RED "ARGUMENTO INVALIDO: nºBloques > 0\n" RESET);
        return FALLO;
    }

    // montamos disco
    if (bmount(*(argv + 1)) == FALLO)
    {
        return FALLO;
    }

    // escribimos disco
    unsigned char *buffer = malloc(BLOCKSIZE);
    if (!buffer)
    {
        perror(RED "ERROR");
        return FALLO;
    }

    memset(buffer, '\000', BLOCKSIZE);
    for (int i = 0; i < nbloque; i++)
    {
#if DEBUG1
        fprintf(stderr, GRAY "[main() -> inicializando bloque %d]\n" RESET, i);
#endif
        if (bwrite(i, buffer) == FALLO)
        {
            fprintf(stderr, RED "ERROR: main(): Error al inicializar bloque %d\n" RESET, i);
            errorExit();
        }
    }
    free(buffer);
#if DEBUG3
    fprintf(stderr, GRAY "[main() -> inicializando SB]\n" RESET);
#endif
    if (initSB(nbloque, nbloque / 4) == FALLO)
    {
        fprintf(stderr, RED "ERROR: main(): initSB()\n" RESET);
        errorExit();
    }
#if DEBUG3
    fprintf(stderr, GRAY "[main() -> inicializando MB]\n" RESET);
#endif
    if (initMB() == FALLO)
    {
        errorExit();
    }
#if DEBUG3
    fprintf(stderr, GRAY "[main() -> inicializando AI]\n" RESET);
#endif
    if (initAI() == FALLO)
    {
        errorExit();
    }
    // creamos directorio raiz
    if (reservar_inodo('d', 7) == FALLO)
    {
        fprintf(stderr, RED "ERROR: main(): no se ha podido crear inodo raiz\n");
        return FALLO;
    }

    // desmontamos disco
    if (bumount())
    {
        return FALLO;
    }

    return EXITO;
}

void errorExit()
{
    bumount();
    exit(FALLO);
}
