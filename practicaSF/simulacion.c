#include "simulacion.h"

#include <stddef.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define FALLO -1
#define EXITO 0

void reaper();

int acabados;

int main(int argc, char **argv)
{
    // comprobar formato
    if (argc != 2)
    {
        fprintf(stderr, RED "ERROR: uso -> ./simulacion <disco>\n" RESET);
        return FALLO;
    }

    // Montar disco
    if (bmount(*(argv + 1)) == FALLO)
    {
        return FALLO;
    }

    // asociar señal
    signal(SIGCHLD, reaper);

    acabados = 0;

    struct tm *tm;
    time_t timeAct = time(NULL);
    tm = localtime(&timeAct);

    char nombreCarpeta[128]; //tamaño '/simul_aaaammddhhmmss/'
    memset(nombreCarpeta, '\0', sizeof(nombreCarpeta));

    sprintf(nombreCarpeta, "/simul_%d%02d%02d%02d%02d%02d/",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    if(mi_creat(nombreCarpeta, 6)==FALLO){
        return FALLO;
    }

    srand(time(NULL) + getpid());
    struct REGISTRO escribir;
    for (int i = 1; i < NUMPROCESOS; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        { // hijo
            if (bmount(*(argv + 1)) == FALLO)
            {
                fprintf(stderr, RED "simulacion() -> ERROR montar hijo\n" RESET);
                exit(FALLO);
            }
            //crear directorio hijo
            char rutaHijo[256];
            memset(rutaHijo, '\0', sizeof(rutaHijo));

            sprintf(rutaHijo, "%sproceso_PID%d/", nombreCarpeta, getpid());
            if(mi_creat(rutaHijo, 7)==FALLO){
                fprintf(stderr, RED "ERROR: hijo:%d no ha podido crear carpeta %s\n" RESET, getpid(), rutaHijo);
                bumount();
                exit(FALLO);
            }

            strcat(rutaHijo, "prueba.dat");
            if(mi_creat(rutaHijo, 7)==FALLO){
                fprintf(stderr, RED "ERROR: hijo:%d no ha podido crear archivo %s\n" RESET, getpid(), rutaHijo);
                bumount();
                exit(FALLO);
            }

            for (int j = 1; j < NUMESCRITURAS; j++)
            {
                memset(&escribir, '\0', sizeof(struct REGISTRO));
                escribir.pid = i;
                escribir.nRegistro = rand() % REGMAX;
                escribir.nEscritura = j;
                escribir.fecha = time(NULL);

                mi_write(rutaHijo, &escribir, escribir.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO));
                usleep(50);

            }
            exit(bumount());
        }
        usleep(150);

    }
    // Desmontar disco
    if (bumount() == FALLO)
    {
        return FALLO;
    }
}

void reaper()
{
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        acabados++;
    }
}
