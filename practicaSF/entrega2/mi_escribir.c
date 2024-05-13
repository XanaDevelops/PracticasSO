/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "directorios.h"
int exitError();

int exitError()
{
    // Desmontar el disco antes de salir
    if (bumount() == FALLO)
    {
        return FALLO;
    }

    return FALLO;
}

int main(int argc, char **argv)
{
    // comprobamos argumentos de consola
    if (argc < 5)
    {
        fprintf(stderr, RED "ARGUMENTOS INSUFICIENTES--> Sintaxis: mi_escribir  <disco> </ruta_fichero> <texto> <offset>\n" RESET);
        return FALLO;
    }

    // montamos disco
    if (bmount(*(argv + 1)) == FALLO)
    {
        return FALLO;
    }

    char *camino = argv[2];
    int longitud_texto = 0;

    int n_palabras = 0;
    while (n_palabras < argc - 4)
    {
        longitud_texto += strlen(argv[n_palabras + 3]);
        n_palabras++;
    }
    int n_espacios = n_palabras - 1;
    longitud_texto += n_espacios;

    char buffer_original[longitud_texto];
    strcpy(buffer_original, argv[3]);
    for (int i = 1; i < n_palabras; i++)
    {
        strcat(buffer_original, " ");
        strcat(buffer_original, argv[i + 3]);
    }

    int offset = atoi(*(argv + argc - 1));
    printf("longitud_texto %d \n", longitud_texto);
    
    int bytes;

    if (*(camino + longitud_texto - 1) != '/')
    {
        // Llamada a mi_write de la capa de directorios
        bytes = mi_write(camino, buffer_original, offset, longitud_texto);
    }
    else
    {
        fprintf(stderr, RED "RUTA INVÁLIDA: La ruta para mi_write leer fichero no debe terminar con '/'.\n" RESET);
        printf("Bytes Escritos: %d\n", 0);
        return exitError();
    }
    if(bytes<=FALLO){
        return exitError();
    }
    printf("Bytes Escritos: %d\n", bytes);

    // desmontamos disco
    if (bumount() == FALLO)
    {
        return FALLO;
    }

    return EXITO;
}