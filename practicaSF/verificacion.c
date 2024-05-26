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
#if DEBUG13
    fprintf(stderr, "[dir_sim: %s]\n" RESET, argv[2]);
#endif

    int numentradas = s_stat.tamEnBytesLog / sizeof(struct entrada);

    if (numentradas != NUMPROCESOS)
    {
        return exitError();
    }

#if DEBUG13
    fprintf(stderr, "[numentradas: %d NUMPROCESOS: %d]\n" RESET, numentradas, NUMPROCESOS);
#endif

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

    struct INFORMACION buff_info;
    int offset_info = 0;
    struct REGISTRO buff_reg[NREGISTROS];
    memset(buff_reg, '\0', sizeof(buff_reg));

    for (int i = 0; i < numentradas; i++)
    {
        char *inici = strchr(buff_entradas[i].nombre, 'D');

        if (inici)
        {
            buff_info.pid = atoi(inici + 1);
        }
        else
        {
            // Manejar el caso en que no se encuentre el caracter '_'
            buff_info.pid = FALLO;
            return exitError();
        }

        char prueba[100];

        strcpy(prueba, argv[2]);
        strcat(prueba, buff_entradas[i].nombre);
        strcat(prueba, "/prueba.dat");

        struct REGISTRO ultimo;
        int escriturasLeidas = 0;
        int leidosT = 0;
        int leidos = mi_read(prueba, buff_reg, leidosT, sizeof(buff_reg));
        leidosT += leidos;

        while (leidos != 0)
        {
            int ultimoreg = leidos / sizeof(struct REGISTRO);
           // int ultimoreg = sizeof(buff_reg) / sizeof(struct REGISTRO);

            for (int j = 0; j < ultimoreg; j++)
            {
                if (buff_reg[j].pid == buff_info.pid)
                {
                    if (escriturasLeidas == 0)
                    {
                        // Inicializar los registros significativos con los datos de esa escritura.
                        buff_info.MenorPosicion = buff_reg[j];
                        buff_info.PrimeraEscritura = buff_reg[j];
                        buff_info.UltimaEscritura = buff_reg[j];
                    }
                    else
                    {
                        // Comparar nº de escritura (para obtener primera y última) y actualizarlas si es preciso
                        if (buff_reg[j].nEscritura < buff_info.PrimeraEscritura.nEscritura)
                        {
                            buff_info.PrimeraEscritura = buff_reg[j];
                        }
                        if (buff_reg[j].nEscritura > buff_info.UltimaEscritura.nEscritura)
                        {
                            buff_info.UltimaEscritura = buff_reg[j];
                        }
                    }
                    // Incrementar contador escrituras validadas.
                    escriturasLeidas++;
                    ultimo = buff_reg[j];
                }
            }

            memset(buff_reg, '\0', sizeof(buff_reg));
            leidos = mi_read(informe, buff_reg, leidosT, sizeof(buff_reg));
            leidosT += leidos;
        }
#if DEBUG13
        fprintf(stderr, GRAY "[%d) %d escrituras validadas en %s]\n" RESET, i, escriturasLeidas, prueba);
#endif
        // Obtener la escritura de la última posición.
        buff_info.MayorPosicion = ultimo;
        buff_info.nEscrituras = escriturasLeidas;

        char info_escribir[500]; // Incrementamos el tamaño del buffer para evitar desbordamiento
        char buffer[100];        // Buffer temporal para conversiones

        // Limpiamos el buffer
        memset(info_escribir, 0, sizeof(info_escribir));

        // Concatenamos PID
        strcat(info_escribir, "Pid: ");
        sprintf(buffer, "%d\n", buff_info.pid);
        strcat(info_escribir, buffer);

        // Concatenamos Nº Escrituras
        strcat(info_escribir, "Nº Escrituras: ");
        sprintf(buffer, "%u\n", buff_info.nEscrituras);
        strcat(info_escribir, buffer);

        // Concatenamos Primera Escritura
        strcat(info_escribir, "PrimeraEscritura: ");
        sprintf(buffer, "%d\n", buff_info.PrimeraEscritura.nEscritura);
        strcat(info_escribir, buffer);

        // Concatenamos Última Escritura
        strcat(info_escribir, "UltimaEscritura: ");
        sprintf(buffer, "%d\n", buff_info.UltimaEscritura.nEscritura);
        strcat(info_escribir, buffer);

        // Concatenamos Menor Posición
        strcat(info_escribir, "Nº MenorPosicion: ");
        sprintf(buffer, "%d\n", buff_info.MenorPosicion.nRegistro);
        strcat(info_escribir, buffer);

        // Concatenamos Mayor Posición
        strcat(info_escribir, "Nº MayorPosicion: ");
        sprintf(buffer, "%d\n", buff_info.MayorPosicion.nRegistro);
        strcat(info_escribir, buffer);

        strcat(info_escribir, "\n");

        offset_info += mi_write(informe, &info_escribir, offset_info, sizeof(info_escribir));
    }

    // Desmontar disco
    if (bumount() == FALLO)
    {
        return FALLO;
    }
}
