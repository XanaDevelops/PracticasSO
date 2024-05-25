/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include "verificacion.h"
#define NREGISTROS 256
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

    /*  if (numentradas != NUMPROCESOS)
      {
          return  exitError();}*/

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
    struct REGISTRO buff_reg[NREGISTROS];
    memset(buff_reg, '\0', sizeof(buff_reg));
    memset(buff_info, '\0', sizeof(buff_info));

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

        char prueba[100];

        strcpy(prueba, argv[2]);
        strcat(prueba, buff_entradas[i].nombre);
        strcat(prueba, "/prueba.dat");

        struct REGISTRO ultimo;
        int escriturasLeidas = 0;
        int leidosT = 0;
        int leidos = mi_read(informe, buff_reg, leidosT, sizeof(buff_reg));
        leidosT += leidos;

        while (leidos != 0)
        {
            int ulitmoreg = leidos / sizeof(struct REGISTRO);

            for (int j = 0; j < ulitmoreg; j++)
            {
                if (buff_reg[j].pid == buff_info[i].pid)
                {
                    if (escriturasLeidas == 0)
                    {
                        // Inicializar los registros significativos con los datos de esa escritura.
                        buff_info[i].MenorPosicion = buff_reg[j];
                        buff_info[i].PrimeraEscritura = buff_reg[j];
                        buff_info[i].UltimaEscritura = buff_reg[j];
                    }
                    else
                    {
                        // Comparar nº de escritura (para obtener primera y última) y actualizarlas si es preciso
                        if (buff_reg[j].nEscritura < buff_info[i].PrimeraEscritura.nEscritura)
                        {
                            buff_info[i].PrimeraEscritura = buff_reg[j];
                        }
                        if (buff_reg[j].nEscritura > buff_info[i].UltimaEscritura.nEscritura)
                        {
                            buff_info[i].UltimaEscritura = buff_reg[j];
                        }
                    }
                    // Incrementar contador escrituras validadas.
                    escriturasLeidas++;
                }
            } 

            ultimo = buff_reg[ulitmoreg - 1];

            memset(buff_reg, '\0', sizeof(buff_reg));
            leidos = mi_read(informe, buff_reg, leidosT, sizeof(buff_reg));
            leidosT += leidos;
        }
        // Obtener la escritura de la última posición.
        buff_info[i].MayorPosicion = ultimo;
        buff_info[i].nEscrituras = escriturasLeidas;
    }
    mi_write(informe, &buff_info, 0, sizeof(buff_info));

    // Desmontar disco
    if (bumount() == FALLO)
    {
        return FALLO;
    }
}
