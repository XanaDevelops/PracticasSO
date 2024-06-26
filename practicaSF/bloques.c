/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include "bloques.h"
#include "semaforo_mutex_posix.h"

#define USARMMAP 1

static int descriptor = 0;

#if USARMMAP
static int tamSFM;   // tamaño mem compartida
static void *prtSFM; // puntero mem compartida
#endif
// Variable global para el semáforo
static sem_t *mutex;
// Evitar multiples waits
static unsigned int inside_sc = 0;

/**
 *  Montar el dispositivo virtual
 */
int bmount(const char *camino)
{
    // Abrir el fichero como lectura y escritura, creándolo si no existe
    // cambios por sim.c
    if (descriptor > 0)
    {
        close(descriptor);
    }
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    #if USARMMAP == 1
    prtSFM = do_mmap(descriptor);
    #endif
    // Comprobar si ha habido un error al abrir el fichero
    if (descriptor == -1)
    {
        // Gestión de error
        fprintf(stderr, RED "ERROR: bmount(): No se ha podido abrir o crear el fichero\n" RESET);
        // Devolver FALLO
        return FALLO;
    }

    // el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
    if (!mutex)
    {
        mutex = initSem();
        if (mutex == SEM_FAILED)
        {
            return -1;
        }
    }

    // Devolver el descriptor del fichero
    return descriptor;
}

/**
 *  Desmontar el dispositivo virtual
 */
int bumount()
{
    #if USARMMAP == 1
    msync(prtSFM, tamSFM, MS_SYNC);

    munmap(prtSFM, tamSFM);
    #endif
    descriptor = close(descriptor);
    if (descriptor == 0)
    {
        deleteSem();
        return EXITO;
    }
    else
    {
        // Gestón del error
        fprintf(stderr, RED "ERROR: bumount(): No se ha podido cerrar el fichero\n" RESET);
        return FALLO;
    }
}
/**
 * Escribe *buf en nBloque
 * return: numBytes escritos o FALLO
 */
int bwrite(unsigned int nbloque, const void *buf)
{
    
    // Calcular el desplazamiento del dispositivo virtual
    off_t desplazamiento = nbloque * BLOCKSIZE;
    #if USARMMAP == 1
    int s;
    // controlar limites
    if (desplazamiento + BLOCKSIZE <= tamSFM)
    {
        s = BLOCKSIZE;
    }
    else
    {
        s = tamSFM - desplazamiento;
    }
    if (s > 0)
    {
        memcpy(prtSFM + desplazamiento, buf, s);
    }
    else
    {
        // Gestión de error
        fprintf(stderr, RED "ERROR bwrite: s es <= 0\n" RESET);
        return FALLO;
    }

    // Devolver el número de bytes escrito en el fichero
    return s;
    #else
     // Mover el puntero al desplazamiento calculado
    off_t puntero = lseek(descriptor, desplazamiento, SEEK_SET);

    if (puntero == -1)
    {
        // Gestión de error
        perror(RED "Error");
        printf(RESET);
        return FALLO;
    }

    // Escribir el contenido del buffer en el bloque especificado
    size_t numBytes = write(descriptor, buf, BLOCKSIZE);

    if (numBytes == -1)
    {
        // Gestión de error
        perror(RED "Error");
        printf(RESET);
        return FALLO;
    }

    // Devolver el número de bytes escrito en el fichero
    return numBytes;
    #endif
}
/**
 * Lee nbloque en un *buf
 * return: numBytes leidos, o FALLO
 */
int bread(unsigned int nbloque, void *buf)
{
   
    // Calcular el desplazamiento del dispositivo virtual
    off_t desplazamiento = nbloque * BLOCKSIZE;

    #if USARMMAP == 1
    int s;
    // controlar limites
    if (desplazamiento + BLOCKSIZE <= tamSFM)
    {
        s = BLOCKSIZE;
    }
    else
    {
        s = tamSFM - desplazamiento;
    }
    if (s > 0)
    {
        memcpy(buf, prtSFM + desplazamiento, s);
    }
    else
    {
        // Gestión de error
        fprintf(stderr, RED "ERROR bread() -> s es <= 0\n" RESET);
        return FALLO;
    }
#if DEBUG1
    fprintf(stderr, GRAY "[bread()-> numBytes=%ld]\n" RESET, numBytes);
#endif
    // Devolver el número de bytes que se ha podido leer
    return s;
    #else
     // Mover el puntero al desplazamiento calculado
    off_t puntero = lseek(descriptor, desplazamiento, SEEK_SET);

      if (puntero == -1)
    {
        // Gestión de error
        perror(RED "Error");
         //printf(RESET);
        return FALLO;
    }

    size_t numBytes = read(descriptor, buf, BLOCKSIZE);

     if (numBytes == -1)
    {
        // Gestión de error
        perror(RED "Error");
         //printf(RESET);
        return FALLO;
    }
#if DEBUG1
    fprintf(stderr, GRAY "[bread()-> numBytes=%ld]\n" RESET, numBytes);
#endif
   // Devolver el número de bytes que se ha podido leer
    return numBytes;
    #endif
}

//************************************FUNCIONES SEMAFORO**********************
void mi_waitSem()
{
    if (!inside_sc)
    {
        // inside_sc==0, no se ha hecho ya un wait
        waitSem(mutex);
    }
    inside_sc++;
}

void mi_signalSem()
{
    inside_sc--;

    if (!inside_sc)
    {
        signalSem(mutex);
    }
}

#if USARMMAP
void *do_mmap(int fd)
{
    struct stat st;
    void *ptr;
    fstat(fd, &st);
    tamSFM = st.st_size;
    if ((ptr = mmap(NULL, tamSFM, PROT_WRITE, MAP_SHARED, fd, 0)) == (void *)-1)
    {
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
    }

    return ptr;
}
#endif