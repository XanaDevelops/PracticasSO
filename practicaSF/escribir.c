#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ficheros.h"

void print_estado(struct STAT *estado);
void errorExit();

int main(int argc, char **argv)
{
    
    long int offsets[] = {9000, 209000, 30725000, 409605000, 480000000};
    char *buffer_original = argv[2];
    int buffer_size = strlen(buffer_original);
    memset(&buffer_original, '\0', buffer_size);

    // comprobamos argumentos de consola
    if (argc < 4)
    {
        fprintf(stderr, RED "ARGUMENTOS INSUFICIENTES--> Sintaxis: escribir <nombre_dispositivo> <$(cat fichero)> <diferentes_inodos>\n" RESET);
        return FALLO;
    }

     // montamos disco
    if (bmount(*(argv + 1)) == FALLO)
    {
        return FALLO;
    }

    // comprobamos numero bloques
    if (atoi(*(argv + 3)) == 0)
    {
        int inodoReservado = reservar_inodo('f', 6); // mirar si van bien

        if (inodoReservado == FALLO)
        {
            fprintf(stderr, RED "ERROR: reservar_inodo(): no se ha podido reservar inodo\n" RESET);
            errorExit();
        }
        for (int i = 0; i < (sizeof(offsets) / sizeof(offsets[0])); i++)
        {
            struct STAT estado;
            mi_stat_f(inodoReservado, &estado);
            print_estado(&estado);

            mi_write_f(inodoReservado, &buffer_original, offsets[i], buffer_size);

            mi_stat_f(inodoReservado, &estado);
            print_estado(&estado);

            memset(&buffer_original, '\0', buffer_size);
            mi_read_f(inodoReservado, &buffer_original, offsets[i], buffer_size);
            write(1, buffer_original, buffer_size);
        }
    }
    else if (atoi(*(argv + 3)) == 1)
    {
        for (int i = 0; i < (sizeof(offsets) / sizeof(offsets[0])); i++)
        {
            int inodoReservado = reservar_inodo('f', 6); // mirar si van bien

            if (inodoReservado == FALLO)
            {
                fprintf(stderr, RED "ERROR: leer_sf(): no se ha podido reservar inodo\n" RESET);
                return FALLO;
            }

            struct STAT estado;
            mi_stat_f(inodoReservado, &estado);
            print_estado(&estado);

            mi_write_f(inodoReservado, &buffer_original, offsets[i], buffer_size);

            mi_stat_f(inodoReservado, &estado);
            print_estado(&estado);

            memset(&buffer_original, '\0', buffer_size);
            mi_read_f(inodoReservado, &buffer_original, offsets[i], buffer_size);
            write(1, buffer_original, buffer_size);
        }
    }
    else
    {
        fprintf(stderr, RED "ARGUMENTOS INVALIDOS --> <diferentes_inodos> = 1 || 0 \n" RESET);
         errorExit();
    }

     // desmontamos disco
    if (bumount())
    {
        return FALLO;
    }

}

void errorExit()
{
    bumount();
    exit(FALLO);
}


void print_estado(struct STAT *estado)
{
    printf("\n\nESTATS\n");
    printf("Tipo: %c\n", estado->tipo);
    printf("Permisos: %c\n", estado->permisos);

    printf("Fecha y hora del último acceso a datos: %s", ctime(&(estado->atime)));
    printf("Fecha y hora de la última modificación de datos: %s", ctime(&(estado->mtime)));
    printf("Fecha y hora de la última modificación del inodo: %s", ctime(&(estado->ctime)));

    printf("Cantidad de enlaces de entradas en directorio: %u\n", estado->nlinks);
    printf("Tamaño en bytes lógicos (EOF): %u\n", estado->tamEnBytesLog);
    printf("Cantidad de bloques ocupados zona de datos: %u\n", estado->numBloquesOcupados);
}