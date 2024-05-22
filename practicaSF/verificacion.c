/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include "verificacion.h"
// Mejora 2,

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
    if (bmount(argv[1]) == FALLO)
    {
        return FALLO;
    }

    // Calcular el nº de entradas del directorio de simulación a partir del stat de su inodo.
    struct STAT s_stat;
    if (mi_stat(argv[2], &s_stat) == FALLO)
    {
        return exitError();
    }
    int numentradas = s_stat.tamEnBytesLog / sizeof(struct entrada);

    if (numentradas != NUMPROCESOS)
    {
        return exitError();
    }

    // Crear el fichero "informe.txt" dentro del directorio de simulación.
    char informe[100];
    strcpy(informe, argv[2]);
    strcat(informe, "informe.txt");

    if (mi_creat(informe, 6) == FALLO)
    {
        return exitError();
    }

    // Leer los directorios correspondientes a los procesos.
    struct entrada buff_entradas[numentradas];
    if (mi_read(argv[2], buff_entradas, 0, sizeof(buff_entradas)) == FALLO)
    {
        return exitError();
    }

    struct INFORMACION buff_info[numentradas];
    for (int i = 0; i < numentradas; i++)
    {
        char *inici = strchr(buff_entradas[i].nombre, '_');
        if (inici)
        {
            buff_info[i].pid = atoi(inici + 1);
        }
        else
        {
            // Manejar el caso en que no se encuentre el caracter '_'
            buff_info[i].pid = FALLO;
            return exitError();
        }
    }
}
