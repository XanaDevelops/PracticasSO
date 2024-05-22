/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include "verificion.h"

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
    if (argc < 3)
    {
        fprintf(stderr, RED "ARGUMENTOS INSUFICIENTES--> Sintaxis:  verificacion <nombre_dispositivo> <directorio_simulación>\n" RESET);
        return FALLO;
    }

    // montamos disco
    if (bmount(*(argv + 1)) == FALLO)
    {
        return FALLO;
    }

    // Calcular el nº de entradas del directorio de simulación a partir del stat de su inodo.
    int numentradas = 100; //************************************************************************

    if (numentradas != NUMPROCESOS)
    {
        return exitError();
    }

    //Crear el fichero "informe.txt" dentro del directorio de simulación.
    mi_creat("/informe.txt", 6);

    return exitError();
}
