/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include "simulacion.h"

#include <stddef.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define FALLO -1
#define EXITO 0

#define USAR_MYSLEEP 0

#if USAR_MYSLEEP == 1
#define MULT 1
#else
#define MULT 1000
#endif
// con usleep
#define ESPERA_PADRE 150 * MULT
#define ESPERA_HIJO 50 * MULT

void reaper();
void my_sleep();

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

    char nombreCarpeta[128]; // tamaño '/simul_aaaammddhhmmss/'
    memset(nombreCarpeta, '\0', sizeof(nombreCarpeta));

    sprintf(nombreCarpeta, "/simul_%d%02d%02d%02d%02d%02d/",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    if (mi_creat(nombreCarpeta, 0b110) == FALLO)
    {
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
            // crear directorio hijo
            char rutaHijo[256];
            memset(rutaHijo, '\0', sizeof(rutaHijo));

            sprintf(rutaHijo, "%sproceso_PID%d/", nombreCarpeta, getpid());

            if (mi_creat(rutaHijo, 0b110) == FALLO)
            {
                fprintf(stderr, RED "ERROR: hijo:%d no ha podido crear carpeta %s\n" RESET, getpid(), rutaHijo);
                bumount();
                exit(FALLO);
            }

#if DEBUG12
            fprintf(stderr, GRAY "[hijo:%d -> carpeta creada %s]\n" RESET, getpid(), rutaHijo);
#endif

            strcat(rutaHijo, "prueba.dat");
            if (mi_creat(rutaHijo, 0b110) == FALLO)
            {
                fprintf(stderr, RED "ERROR: hijo:%d no ha podido crear archivo %s\n" RESET, getpid(), rutaHijo);
                bumount();
                exit(FALLO);
            }

            srand(time(NULL) + getpid());
            int escrituras = 0;
            for (int j = 1; j <= NUMESCRITURAS; j++)
            {
                memset(&escribir, '\0', sizeof(struct REGISTRO));
                escribir.pid = getpid();
                escribir.nRegistro = rand() % REGMAX;
                escribir.nEscritura = j;
                // escribir.fecha = time(NULL);
                gettimeofday(&escribir.fecha, NULL);
#if DEBUG12
                fprintf(stderr, GRAY "[hijo:%d -> escribiendo %d]\n" RESET, escribir.pid, escribir.nEscritura);
#endif

                if (mi_write(rutaHijo, &escribir, escribir.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO)) == FALLO)
                {
                    fprintf(stderr, RED "ERROR hijo:%d -> no se ha podido escribir registro:%d con nEscr: %d\n" RESET,
                            getpid(), escribir.nRegistro, escribir.nEscritura);
                    bumount();
                    exit(FALLO);
                }
                escrituras++;
#if USAR_MYSLEEP == 0
                usleep(ESPERA_HIJO);
#else
                my_sleep(ESPERA_HIJO);
#endif

                #if DEBUG13
                if(j==NUMESCRITURAS){
                    char aux[128];
                    strftime(aux, sizeof(aux), "%H:%M:%S", localtime(&escribir.fecha.tv_sec));
                    fprintf(stderr, GRAY "ultima escritura en :%s.%06ld\n" RESET, aux, escribir.fecha.tv_usec);
                }
                #endif
            }
            fprintf(stdout, GRAY "[Proceso %d: Completadas %d escrituras en %s]\n" RESET, i, escrituras, rutaHijo);

            exit(bumount());
        }
        else
        {
#if DEBUG12
            fprintf(stderr, GRAY "[main() -> creado hijo %d]\n" RESET, pid);
#endif
#if USAR_MYSLEEP == 0
            usleep(ESPERA_PADRE);
#else
            my_sleep(ESPERA_PADRE);
#endif
        }
    }

    while (acabados < NUMPROCESOS)
    {
        pause();
    }

    // Desmontar disco
    if (bumount() == FALLO)
    {
        return FALLO;
    }

    return EXITO;
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

void my_sleep(unsigned msec)
{ // recibe tiempo en milisegundos
    struct timespec req, rem;
    int err;
    req.tv_sec = msec / 1000;              // conversión a segundos
    req.tv_nsec = (msec % 1000) * 1000000; // conversión a nanosegundos
    while ((req.tv_sec != 0) || (req.tv_nsec != 0))
    {
        if (nanosleep(&req, &rem) == 0)
            // rem almacena el tiempo restante si una llamada al sistema
            // ha sido interrumpida por una señal
            break;
        err = errno;
        // Interrupted; continue
        if (err == EINTR)
        {
            req.tv_sec = rem.tv_sec;
            req.tv_nsec = rem.tv_nsec;
        }
    }
}
