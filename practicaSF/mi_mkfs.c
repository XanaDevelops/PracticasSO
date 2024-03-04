#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ficheros_basico.h"

#define DEBUG1 1

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

    memset(buffer, '\0', BLOCKSIZE);
    for (int i = 0; i < nbloque; i++)
    {
#if DEBUG1
        fprintf(stderr, GRAY "[main() -> inicializando bloque %d]\n" RESET, i);
#endif
        bwrite(i, buffer);
    }
    free(buffer);

    if (initSB(nbloque, nbloque / 4) == FALLO)
    {
        fprintf(stderr, RED "main(): ERROR: initSB()\n" RESET);
        errorExit();
    }
    initMB();
    initAI();

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
