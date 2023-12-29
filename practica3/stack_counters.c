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

void stack_init();
void create_threads();
void *worker(void *ptr);
void stack_end();
//reader.c

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t pthreads[NUM_THREADS];

int main(int argc, char *argv[]){
    return 0;
}

void create_threads(){

}