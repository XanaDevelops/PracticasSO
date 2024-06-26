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
#define N 1000000

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
// reader.c

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t pthreads[NUM_THREADS];

struct my_stack *stack;
char *filename;

int main(int argc, char *argv[])
{
    // Verificar si se ha pasado el nombre del fichero por consola
    if (argc != 2)
    {
        fprintf(stderr, ROJO_T "Sintaxis incorrecta. USO: ./stack_counters <stack_file>\n" RESET);
        exit(EXIT_FAILURE);
    }

    // Inicializar la pila
    filename = argv[1];
    stack_init();
    create_threads();
    stack_end();

    return 0;
}

/**
 * Funció: void stack_init()
 * --------------------------
 * Iniciatlitza la pila, creant-la o carregant-la en memòria
 *  
*/
void stack_init()
{
    // Verificar si el nombre del fichero se ha pasado por consola
    if (!filename)
    {
        fprintf(stderr, ROJO_T "Sintaxis incorrecta. USO: ./stack_counters <stack_file>\n" RESET);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Threads: %d, Iterations: %d\n" RESET, NUM_THREADS, N);

    // Verificar si la pila ya existe
    int file_exists = access(filename, F_OK);
    if (file_exists == -1)
    {
        // Si no existe, crearla e inicializarla con punteros a 0
        stack = my_stack_init(NUM_THREADS);

        fprintf(stdout, "stack->size: %lu\n" RESET, sizeof(int));
        fprintf(stdout, "initial stack length: %d\n" RESET, my_stack_len(stack));
        fprintf(stdout, "initial stack content:\n" RESET);

        // Inicializat pila
        for (int i = 0; i < NUM_THREADS; i++) {
            int *data = (int *)malloc(sizeof(int));
            if(data==NULL){
                perror("ERROR: malloc() al init pila");
            }
            *data = 0;
            my_stack_push(stack, data);
            fprintf(stdout, "%d\n", *data);
        }

        // Imprimir nuevo tamaño de la pila
        fprintf(stdout, "new stack length: %d\n", NUM_THREADS);
    }
    else
    {
        // Si ya existe, cargar la pila desde el fichero en la variable global
        stack = my_stack_read(filename);

        // Imprimir información sobre la pila original si existe
        fprintf(stdout, "original stack content:\n");
        struct my_stack_node *current_node = stack->top;
        while (current_node != NULL) {
            fprintf(stdout, "%d\n", *(int *)current_node->data);
            current_node = current_node->next;
        }

        // Imprimir la longitud de la pila original
        fprintf(stdout, "original stack length: %d\n", my_stack_len(stack));

        // Comprobar si la pila tiene menos de 10 elementos o ninguno
        int current_size = my_stack_len(stack);

        if (current_size < NUM_THREADS)
        {
            fprintf(stdout, "Adding %d elements\n", NUM_THREADS - current_size);
            // Agregar los restantes individualmente con punteros apuntando a cero
            for (int i = 0; i < NUM_THREADS - current_size; i++)
            {
                int *data = (int *)malloc(sizeof(int));
                if(data==NULL){
                    perror("ERROR: malloc() add missing data");
                }
                *data = 0;
                my_stack_push(stack, data);
                fprintf(stdout, "%d\n", *data);
            }
            fprintf(stdout, "new stack length: %d\n", my_stack_len(stack));
        }
    }
}

/**
 * Funció: void create_threads()
 * --------------------------
 * Crea els fils
 *  
*/
void create_threads()
{
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&pthreads[i], NULL, worker, NULL);
        fprintf(stdout, GRIS_T "[create_threads(): creado pthread %lu]\n" RESET, pthreads[i]);
    }
}

/**
 * Funció: void *worker()
 * --------------------------
 * Funció per als threads, accedeix amb un semàfor la pila
 *  
 * params: void *ptr
*/
void *worker(void *ptr)
{
    int *valor;
    int i;
    for (i = 0; i < N; i++)
    {
        //secció critica pop
        pthread_mutex_lock(&mutex);
        valor = my_stack_pop(stack);
        pthread_mutex_unlock(&mutex);
        
        //fprintf(stdout, GRIS_T "(%d) %lu Valor leido %i %lu\n" RESET, i, pthread_self(), *valor, valor);
        *valor += 1;
        //fprintf(stdout, GRIS_T "(%d) %lu Valor escrito %i %lu\n" RESET, i, pthread_self(), *valor, valor);

        //secció critica push
        pthread_mutex_lock(&mutex);
        my_stack_push(stack, valor);
        pthread_mutex_unlock(&mutex);
        
    }
    //fprintf(stdout, GRIS_T "SAYONARA %lu con i %d siendo max N=%i\n"RESET, pthread_self(), i, N);
    pthread_exit(NULL);
}

/**
 * Funció: void stack_end()
 * --------------------------
 * Espera als fils i imprimeix el resultat
 *  
*/
void stack_end()
{
    // Esperar a que todos los hilos terminen
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(pthreads[i], NULL);
    }

    fprintf(stdout, "stack content after threads iterations:\n");

    // Imprimir el contenido de la pila después de las iteraciones de los hilos
    struct my_stack_node *current_node = stack->top;
    while (current_node != NULL) {
        fprintf(stdout, "%d\n", *(int *)current_node->data);
        current_node = current_node->next;
    }

    // Imprimir la longitud de la pila
    fprintf(stdout, "stack length: %d\n", my_stack_len(stack));

    // Guardar la pila en un fichero
    my_stack_write(stack, filename);

    // Imprimir la información sobre los elementos escritos en el fichero
    fprintf(stdout, "Written elements from stack to file: %d\n", my_stack_len(stack));
    
    // Liberar espacio de la pila
    int bytes = my_stack_purge(stack);
    fprintf(stdout, "Released bytes: %d\n", bytes);

}
