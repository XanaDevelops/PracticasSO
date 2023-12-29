/**ENTREGA 3
 * versió: 21/12/32
 * autors: Josep Ferriol Font, Daniel García Vázquez
 * i Biel Perelló Perelló
 */

#define _POSIX_C_SOURCE 200112L

#include <pthread.h>
#include "my_lib.h"

#define NUM_THREADS 10
#define N 10000000

#define RESET "\033[0m"
#define NEGRO_T "\x1b[30m"
#define NEGRO_F "\x1b[40m"
#define GRIS_T "\x1b[90m" // #define GRIS_T "\x1b[94m"
#define ROJO_T "\x1b[31m"
#define VERDE_T "\x1b[32m"
#define AMARILLO_T "\x1b[33m"
#define AZUL_T "\x1b[34m"
#define MAGENTA_T "\x1b[35m"
#define CYAN_T "\x1b[36m"
#define BLANCO_T "\x1b[97m"
#define NEGRITA "\x1b[1m"

void stack_init();
void create_threads();
void *worker(void *ptr);
void stack_end();
//reader.c

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t pthreads[NUM_THREADS];

struct my_stack *stack;

int main(int argc, char *argv[]){

    create_threads();

    return 0;
}

void create_threads(){
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(pthreads[i], NULL, worker, NULL);
        fprintf(stderr, GRIS_T "[create_threads(): reado pthread %lu]" RESET, pthreads[i]);
    }
    
}

void *worker(void *ptr){
    int valor;
    for (int i = 0; i < N; i++)
    {
        pthread_mutex_lock(&mutex);
        valor = my_stack_pop(stack);
        valor++;
        my_stack_push(stack, valor);
        pthread_mutex_unlock(&mutex);
    }
    
}