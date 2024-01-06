/**ENTREGA 3
 * versió: 21/12/32
 * autors: Josep Ferriol Font, Daniel García Vázquez
 * i Biel Perelló Perelló
 */

#define _POSIX_C_SOURCE 200112L

#include <pthread.h>
#include "my_lib.h"
#include <stdio.h>

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

void stack_init(const char *filename);
void create_threads();
void *worker(void *ptr);
void stack_end();
//reader.c

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t pthreads[NUM_THREADS];

struct my_stack *stack;

int main(int argc, char *argv[]){
    // Verificar si se ha pasado el nombre del fichero por consola
    if (argc != 2) {
        fprintf(stderr, "Sintaxis incorrecta. Uso: ./programa <nombre_fichero>\n");
        exit(EXIT_FAILURE);
    }

    // Inicializar la pila
    stack_init(argv[1]);
    create_threads();
    

    return 0;
}

void stack_init(const char *filename){
    // Verificar si el nombre del fichero se ha pasado por consola
    if (!filename){
        fprintf(stderr, "Sintaxis incorrecta. Uso: ./programa <nombre_fichero>\n");
        exit(EXIT_FAILURE);
    }

    // Verificar si la pila ya existe
    int file_exists = access(filename, F_OK);
    if (file_exists == -1){
        // Si no existe, crearla e inicializarla con punteros a 0
        stack = my_stack_init(NUM_THREADS);

        for (int i = 0; i < NUM_THREADS; i++)
        {
            int *data = (int *)malloc(sizeof(int));
            *data = 0;
            my_stack_push(stack, data);
        }

        // Guardar la pila en el fichero
        my_stack_write(stack, filename);

        // Liberar memoria
        int bytes = my_stack_purge(stack);
        fprintf(stdout, "Released Bytes: %d \n", bytes);
    }
    else {
        // Si ya existe, cargar la pila desde el fichero en la variable global
        stack = my_stack_read(filename);

        // Comprobar si la pila tiene menos de 10 elementos o ninguno
        int current_size = my_stack_len(stack);

        if (current_size < NUM_THREADS){
            // Agregar los restantes individualmente con punteros apuntando a cero
            for (int i = 0; i < NUM_THREADS - current_size; i++)
            {
                int *data = (int *)malloc(sizeof(int));
                *data = 0;
                my_stack_push(stack, data);
            }
        }
    }
}

void create_threads(){
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&pthreads[i], NULL, worker, NULL);
        fprintf(stderr, GRIS_T "[create_threads(): reado pthread %lu]\n" RESET, pthreads[i]);
    }
    
}

void *worker(void *ptr){
    int *valor;
    for (int i = 0; i < N; i++)
    {
        pthread_mutex_lock(&mutex);
        valor = my_stack_pop(stack);
        fprintf(stderr, GRIS_T "Valor leido %i\n" RESET, *valor);
        *valor+=1;
        fprintf(stderr, GRIS_T "Valor escrito %i\n" RESET, *valor);
        my_stack_push(stack, valor);
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}

void stack_end() {
    // Esperar a que todos los hilos terminen
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(pthreads[i], NULL);
    }

    // Guardar la pila en un fichero
    my_stack_write(stack, "output_stack.txt");

    // Liberar espacio de la pila
    my_stack_purge(stack);

    // Finalizar los hilos
    pthread_exit(NULL);
}