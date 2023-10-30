/* lib.h librería con las funciones equivalentes a las
de <string.h> y las funciones y estructuras para el
manejo de una pila */

#include <stdio.h>     /* para printf en depurarión */
#include <string.h>    /* para funciones de strings  */
#include <stdlib.h>    /* Funciones malloc(), free(), y valor NULL */
#include <fcntl.h>     /* Modos de apertura de función open()*/
#include <sys/stat.h>  /* Permisos función open() */
#include <sys/types.h> /* Definiciones de tipos de datos como size_t*/
#include <unistd.h>    /* Funciones read(), write(), close()*/
#include <errno.h>     /* COntrol de errores (errno) */

//declaraciones funciones libreria string
size_t my_strlen(const char *str);
int my_strcmp(const char *str1, const char *str2);
char *my_strcpy(char *dest, const char *src);
char *my_strncpy(char *dest, const char *src, size_t n);
char *my_strcat(char *dest, const char *src);
char *my_strchr(const char *s, int c);
int my_stack_write(struct my_stack *stack, char *filename);

// char *my_strncat(char *dest, const char *src, size_t n);

//structuras para gestor de pila
struct my_stack_node {      // nodo de la pila (elemento)
    void *data;
    struct my_stack_node *next;
};

struct my_stack {   // pila
    int size;       // tamaño de data, nos lo pasarán por parámetro
    struct my_stack_node *top;  // apunta al nodo de la parte superior
};  

//declaraciones funciones gestor de pila
struct my_stack *my_stack_init(int size);
int my_stack_push(struct my_stack *stack, void *data);
void *my_stack_pop(struct my_stack *stack);
int my_stack_len(struct my_stack *stack);
int my_stack_purge(struct my_stack *stack); 
struct my_stack *my_stack_read(char *filename);


size_t my_strlen(const char *str) {
    size_t len = 0;

    for(int i = 0; str[i]; i++) {
        len++;
    }

    return len;    
}

int my_strcmp(const char *str1, const char *str2) {
    int i = 0;

    while(str1[i] & str2[i]) {
        char c1,c2;
        c1 = str1[i];
        c2 = str2[i];

        if(c1 != c2) {
            return c1-c2;
        }

        i++;
    }

    return 0;
}

char *my_strcpy(char *dest, const char *src) {
    int i = 0;

    while(src[i]) {
        dest[i] = src[i];
        i++;
    }

    dest[i] = '\0';

    return dest;
}

char *my_strncpy(char *dest, const char *src, size_t n){
    int i = 0;

    while((src[i]) && (n > 0)) {
        dest[i] = src[i];
        i++;
        n--;
    }

    while(n>0) {
        dest[i] = '\0';
        i++;
        n--;
    }

    return dest;   
}

char *my_strcat(char *dest, const char *src) {
    int i = 0;

    while(dest[i] != '\0'){
        i++;
    }

    for(int j = 0; src[j]; j++) {
        dest[i] = src[j]; 
        i++;
    }

    dest[i] = '\0';

    return dest;
}

char *my_strchr(const char *str, int c) {
    while(*str != (char)c && *str) {
        str++;
    }

    if(*str == (char)c) {
        return (char*)str;
    } else {
        return NULL;
    }
}

struct my_stack *my_stack_init(int size) {
    //guardam memòria per la pila
    struct my_stack *stack;
    stack = malloc(sizeof(struct my_stack));

    //declaram les dades de la pila
    stack->size = size;
    stack->top = NULL;

    return stack;
}

int my_stack_push(struct my_stack *stack, void *data) {
    //comprovar si la pila està inicialitzada
    if (stack == NULL || stack->size <= 0) {
        printf("Error: Pila no inicializada o tamaño no válido.\n");
        return -1;
    }

    //guardam memòria per el node a introduïr
    struct my_stack_node *node;
    node = malloc(sizeof(struct my_stack_node));

    //comprovar si el node és null
    if (node == NULL) {
        printf("Error: No se pudo insertar el elemento en la pila.\n");
        return -1;
    }

    //guardam memòria per les dades del node
    node->data = malloc(stack->size);
    if (node->data == NULL) {
        printf("Error: No se pudo asignar memoria para el elemento en la pila.\n");
        return -1;
    }

    //gestió de la pila (top) i el nou node a introduïr
    node->data = data;
    node->next = stack->top;
    stack->top = node;

    return 0;
}

void *my_stack_pop(struct my_stack *stack) {
    //comprovar si la pila no està inicialitzada
    if (stack == NULL) {
        printf("Error: Pila no inicializada.\n");
        return NULL;
    }

    //comprovar si la pila és buida
    if(stack->top == NULL) {
        return NULL;
    }

    //extreu el node superior de la pila
    struct my_stack_node *top = stack->top;
    void *data = top->data;

    stack->top = top->next;

    //alliberar la memòria del node
    free(top);

    return data;
}
    
