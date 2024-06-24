/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include "verificacion.h"

#define NREGISTROS 256
#define TAMBUFREG sizeof(struct REGISTRO) * REGMAX
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
    if (argc != 3)
    {
        fprintf(stderr, RED "ARGUMENTOS invalidos--> Sintaxis:  verificacion <nombre_dispositivo> <directorio_simulación>\n" RESET);
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
        fprintf(stderr, RED "ERROR: verificacion.c -> numentradas != NUMPROCESOS, %d != %d\n" RESET, numentradas, NUMPROCESOS);
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

    int offset_info = 0;
    struct REGISTRO *buff_reg = calloc(REGMAX, sizeof(struct REGISTRO));
    // memset(buff_reg, '\0', TAMBUFREG); //calloc = malloc + memset a 0
    if (buff_reg == NULL)
    {
        fprintf(stderr, RED "ERROR: verificacion.c -> no se ha podido reservar ram para buff_reg\n" RESET);
        return exitError();
    }

    for (int i = 0; i < numentradas; i++)
    {
        struct INFORMACION info;

        char *inici = strchr(buff_entradas[i].nombre, 'D');

        if (inici)
        {
            info.pid = atoi(inici + 1);
        }
        else
        {
            // Manejar el caso en que no se encuentre el caracter '_'
            info.pid = FALLO;
            free(buff_reg);
            return exitError();
        }

        char prueba[100];
        memset(prueba, '\0', sizeof(prueba));

        strcpy(prueba, argv[2]);
        strcat(prueba, buff_entradas[i].nombre);
        strcat(prueba, "/prueba.dat");

        struct REGISTRO ultimo;
        int escriturasLeidas = 0;
        int leidosT = 0;
        int leidos = mi_read(prueba, buff_reg, leidosT, TAMBUFREG);
        leidosT += leidos;

        while (leidos > 0)
        {
            int ultimoreg = leidos / sizeof(struct REGISTRO);

            for (int j = 0; j < ultimoreg; j++)
            {
#if DEBUG13
// fprintf(stderr, GRAY "[verificacion() -> comprobando %d respecto %d]\n" RESET, buff_reg[j].pid, info.pid);
#endif
                if (buff_reg[j].pid == info.pid)
                {
                    if (escriturasLeidas == 0)
                    {
                        // Inicializar los registros significativos con los datos de esa escritura.
                        info.MenorPosicion = buff_reg[j];
                        info.PrimeraEscritura = buff_reg[j];
                        info.UltimaEscritura = buff_reg[j];
                    }
                    else
                    {
                        // Comparar nº de escritura (para obtener primera y última) y actualizarlas si es preciso
                        if (buff_reg[j].nEscritura < info.PrimeraEscritura.nEscritura)
                        {
                            info.PrimeraEscritura = buff_reg[j];
                        }
                        if (buff_reg[j].nEscritura > info.UltimaEscritura.nEscritura)
                        {
                            info.UltimaEscritura = buff_reg[j];
                        }
                    }
                    // Incrementar contador escrituras validadas.
                    escriturasLeidas++;
                    ultimo = buff_reg[j];
                }
            }

            memset(buff_reg, '\0', TAMBUFREG);
            leidos = mi_read(informe, buff_reg, leidosT, TAMBUFREG);
            leidosT += leidos;
        }

        fprintf(stdout, GRAY "[%d) %d escrituras validadas en %s]\n" RESET, i, escriturasLeidas, prueba);

        // Obtener la escritura de la última posición.
        info.MayorPosicion = ultimo;
        info.nEscrituras = escriturasLeidas;

        char info_escribir[500]; // Incrementamos el tamaño del buffer para evitar desbordamiento
        char buffer[100];        // Buffer temporal para conversiones
        char fecha_formateada[50];
        struct tm *tiempo_descompuesto;

        // Limpiamos el buffer
        memset(info_escribir, '\0', sizeof(info_escribir));
        memset(buffer, '\0', sizeof(buffer));
        memset(fecha_formateada, '\0', sizeof(fecha_formateada));

        // todo tabulado \t
        //  Concatenamos PID
        strcat(info_escribir, "\tPid: ");
        sprintf(buffer, "%d\n", info.pid);
        strcat(info_escribir, buffer);

        // Concatenamos Nº Escrituras
        strcat(info_escribir, "\tNº Escrituras: ");
        sprintf(buffer, "%u\n", info.nEscrituras);
        strcat(info_escribir, buffer);

        // Concatenamos Primera Escritura
        struct timeval tiempo = info.PrimeraEscritura.fecha;
        tiempo_descompuesto = localtime(&tiempo.tv_sec);
        strcat(info_escribir, "\tPrimeraEscritura: ");
        strftime(fecha_formateada, sizeof(fecha_formateada), "%a %d-%m-%Y %H:%M:%S", tiempo_descompuesto);
        sprintf(buffer, "%d - %d - %s.%06ld\n", info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, fecha_formateada, info.PrimeraEscritura.fecha.tv_usec);
        strcat(info_escribir, buffer);

        // Concatenamos Última Escritura
        tiempo = info.UltimaEscritura.fecha;
        tiempo_descompuesto = localtime(&tiempo.tv_sec);
        strcat(info_escribir, "\tUltimaEscritura: ");
        //faltaba copiar esta mald... bendita linea :D
        strftime(fecha_formateada, sizeof(fecha_formateada), "%a %d-%m-%Y %H:%M:%S", tiempo_descompuesto);
        //no se si con los cambios en las secciones criticas realmente se ejecutaba rapido, pero mostraba siempre el tiempo inicial
        sprintf(buffer, "%d - %d - %s.%06ld\n", info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, fecha_formateada, info.UltimaEscritura.fecha.tv_usec);
        strcat(info_escribir, buffer);

        // Concatenamos Menor Posición
        tiempo = info.MenorPosicion.fecha;
        tiempo_descompuesto = localtime(&tiempo.tv_sec);
        strcat(info_escribir, "\tMenorPosicion: ");
        strftime(fecha_formateada, sizeof(fecha_formateada), "%a %d-%m-%Y %H:%M:%S", tiempo_descompuesto);

        sprintf(buffer, "%d - %d - %s.%06ld\n", info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro, fecha_formateada, info.MenorPosicion.fecha.tv_usec);
        strcat(info_escribir, buffer);

        // Concatenamos Mayor Posición
        tiempo = info.MayorPosicion.fecha;
        tiempo_descompuesto = localtime(&tiempo.tv_sec);
        strcat(info_escribir, "\tMayorPosicion: ");
        strftime(fecha_formateada, sizeof(fecha_formateada), "%a %d-%m-%Y %H:%M:%S", tiempo_descompuesto);

        sprintf(buffer, "%d - %d - %s.%06ld\n\n", info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, fecha_formateada, info.MayorPosicion.fecha.tv_usec);
        strcat(info_escribir, buffer);

        offset_info += mi_write(informe, &info_escribir, offset_info, (strlen(info_escribir) * sizeof(info_escribir[0])));
    }

    free(buff_reg);

    // Desmontar disco
    if (bumount() == FALLO)
    {
        return FALLO;
    }

    return EXITO;
}
