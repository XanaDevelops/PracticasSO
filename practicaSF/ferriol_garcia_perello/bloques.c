/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include "bloques.h"
#include "semaforo_mutex_posix.h"

static int descriptor = 0;
static int debug_print = 0; //imprime bloque leer/escribir DEBUG 

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
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);

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
    if (close(descriptor) == 0)
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
    if(debug_print){ //BORRAR
        fprintf(stderr, GRAY "[DEBUG bwrite(): nbloque:%d, desp:%lX]\n" RESET, nbloque, desplazamiento);
    }
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
}
/**
 * Lee nbloque en un *buf
 * return: numBytes leidos, o FALLO
*/
int bread(unsigned int nbloque, void *buf)
{
     // Calcular el desplazamiento del dispositivo virtual
    off_t desplazamiento = nbloque * BLOCKSIZE;
    if(debug_print){ //BORRAR
        fprintf(stderr, GRAY "[DEBUG bread(): nbloque:%d, desp:%lX]\n" RESET, nbloque, desplazamiento);
    }
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
}

//************************************FUNCIONES SEMAFORO**********************
void mi_waitSem() {
    if (!inside_sc) 
    { 
        // inside_sc==0, no se ha hecho ya un wait
        waitSem(mutex);
    }
   inside_sc++;
}


void mi_signalSem() {
    inside_sc--;
    
    if (!inside_sc) 
    {
        signalSem(mutex);
    }

}


void enablepd(){
    debug_print=1;
    fprintf(stderr, GRAY "estado debug_print: %d\n" RESET, debug_print);
}
void disablepd(){
    debug_print=0;
        fprintf(stderr, GRAY "estado debug_print: %d\n" RESET, debug_print);

}