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
int my_stack_write(struct my_stack *stack, char *filename);
    
