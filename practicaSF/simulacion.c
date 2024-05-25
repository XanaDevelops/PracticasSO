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

    fprintf(stdout, NEGRITA "SIMULACION: %d PROCESOS, %d ESCRITURAS\n" RESET, NUMPROCESOS, NUMESCRITURAS);

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
    
    if(mi_creat(nombreCarpeta, 0b111)==FALLO){
        return FALLO;
    }

    fprintf(stdout, "dirsim: %s\n", nombreCarpeta);

    srand(time(NULL) + getpid());
    struct REGISTRO escribir;
    for (int i = 0; i < NUMPROCESOS; i++)
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
            
            if(mi_creat(rutaHijo, 0b111)==FALLO){
                fprintf(stderr, RED "ERROR: hijo:%d no ha podido crear carpeta %s\n" RESET, getpid(), rutaHijo);
                bumount();
                exit(FALLO);
            }

            #if DEBUG12 || 1
                fprintf(stderr, GRAY "[hijo:%d -> carpeta creada %s]\n" RESET, getpid(), rutaHijo);
            #endif

            strcat(rutaHijo, "prueba.dat");
            if(mi_creat(rutaHijo, 0b111)==FALLO){
                fprintf(stderr, RED "ERROR: hijo:%d no ha podido crear archivo %s\n" RESET, getpid(), rutaHijo);
                bumount();
                exit(FALLO);
            }

            for (int j = 1; j <= NUMESCRITURAS; j++)
            {
                memset(&escribir, '\0', sizeof(struct REGISTRO));
                escribir.pid = getpid();
                escribir.nRegistro = rand() % REGMAX;
                escribir.nEscritura = j;
                escribir.fecha = time(NULL);

                #if DEBUG12
                fprintf(stderr, GRAY "[hijo:%d -> escribiendo %d]\n" RESET, escribir.pid, escribir.nEscritura);
                #endif

                mi_write(rutaHijo, &escribir, escribir.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO));
                usleep(50);

            }
            exit(bumount());
        }else{
            #if DEBUG12
            fprintf(stderr, GRAY "[main() -> creado hijo %d]\n" RESET, pid);
            #endif
            usleep(150);
        }
        

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
