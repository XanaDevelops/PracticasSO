#include "simulacion.h"
#include <stddef.h>

#include <string.h>
#include <stddef.h>
#include <stdio.h>

#define FALLO -1
#define EXITO 0

int main(int argc, char **argv)
{
    // Montar disco
    if (bmount(*(argv + 1)) == FALLO)
    {
        return FALLO;
    }

    srand(time(NULL) + getpid());
    struct REGISTRO escribir;
    for (int i = 1; i < 6; i++)
    {
        for (int j = 1; j < 11; j++)
        {
            escribir.pid = i;
            escribir.nRegistro = rand() % REGMAX;
            escribir.nEscritura = j;
            escribir.fecha = time(NULL);

            char prueba[100];

            strcpy(prueba, "/p1/p_");
            strcat(prueba, (char[]){(char)(i + '0'), '\0'});
            strcat(prueba, "/prueba.dat");

            mi_write(prueba, &escribir, escribir.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO));
        }
    }
    // Desmontar disco
    if (bumount() == FALLO)
    {
        return FALLO;
    }
}
