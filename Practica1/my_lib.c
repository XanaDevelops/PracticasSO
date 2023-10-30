/**ENTREGA 1
 * versió: 30/10/32
 * autors: Josep Ferriol Font, Daniel García Vázquez i Biel Perelló Perelló
 */

/**lib.c librería con las funciones equivalentes a las de <string.h> y
 * las funciones y estructuras para el manejo de una pila
 */
#include "my_lib.h"

/*REPTE 1*/
/**
 * Funció: my_strlen
 * -------------------
 * Calcula la llargaria de la cadena de caràcters
 *
 * param: str --> punter de la cadena de caràcters ha calcular llargaria
 * return: llargaria fora element centinela de la cadena str
 */
size_t my_strlen(const char *str)
{
    size_t len = 0;
    /*Mentre element d'índex len de str sigui diferent del caràcter centinela increment len */
    while (str[len])
    {
        len++;
    }

    return len;
}

/**
 * Funció: my_strcmp
 * -------------------
 * Compara mitjançant el codi ASCII dues cadenes
 *
 * param: str1 --> primer punter de una de les cadenes a comparar
 *        str2 --> segon punter de una de les cadenes a comparar
 * return: la resta dels dos primers caràcters no iguals comparats
 */
int my_strcmp(const char *str1, const char *str2)
{
    int i = 0;
    /*Mentres cap dels dels dos acabi i els caràcters siguin iguals  */
    while (*(str1 + i) != 0 && *(str2 + i) != 0 && *(str1 + i) == *(str2 + i))
    {
        i++;
    }
    /*retorna la resta ASCII del dos primers caràcters no iguals*/
    return *(str1 + i) - *(str2 + i);
}

/**
 * Funció: my_strcpy
 * -------------------
 * Copia el contigut d'una cadena de caràcters a una altra.
 * param: dest --> Cadena de caràcters
 *  */
char *my_strcpy(char *dest, const char *src)
{
    int i = 0;

    while (src[i])
    {
        dest[i] = src[i];
        i++;
    }

    dest[i] = '\0';

    return dest;
}

char *my_strncpy(char *dest, const char *src, size_t n)
{
    int i = 0;

    while ((src[i]) && (n > 0))
    {
        dest[i] = src[i];
        i++;
        n--;
    }

    while (n > 0)
    {
        dest[i] = '\0';
        i++;
        n--;
    }

    return dest;
}

char *my_strcat(char *dest, const char *src)
{
    int i = 0;

    while (dest[i] != '\0')
    {
        i++;
    }

    for (int j = 0; src[j]; j++)
    {
        dest[i] = src[j];
        i++;
    }

    dest[i] = '\0';

    return dest;
}

char *my_strchr(const char *str, int c)
{
    while (*str != (char)c && *str)
    {
        str++;
    }

    if (*str == (char)c)
    {
        return (char *)str;
    }
    else
    {
        return NULL;
    }
}

/*Repte 2*/
struct my_stack *my_stack_init(int size)
{
    // guardam memòria per la pila
    struct my_stack *stack;
    stack = malloc(sizeof(struct my_stack));

    // declaram les dades de la pila
    stack->size = size;
    stack->top = NULL;

    return stack;
}

int my_stack_push(struct my_stack *stack, void *data)
{
    // comprovar si la pila està inicialitzada
    if (stack == NULL || stack->size <= 0)
    {
        printf("Error: Pila no inicializada o tamaño no válido.\n");
        return -1;
    }

    // guardam memòria per el node a introduïr
    struct my_stack_node *node;
    node = malloc(sizeof(struct my_stack_node));

    // comprovar si el node és null
    if (node == NULL)
    {
        printf("Error: No se pudo insertar el elemento en la pila.\n");
        return -1;
    }

    // guardam memòria per les dades del node
    node->data = malloc(stack->size);
    if (node->data == NULL)
    {
        printf("Error: No se pudo asignar memoria para el elemento en la pila.\n");
        return -1;
    }

    // gestió de la pila (top) i el nou node a introduïr
    node->data = data;
    node->next = stack->top;
    stack->top = node;

    return 0;
}

void *my_stack_pop(struct my_stack *stack)
{
    // comprovar si la pila no està inicialitzada
    if (stack == NULL)
    {
        printf("Error: Pila no inicializada.\n");
        return NULL;
    }

    // comprovar si la pila és buida
    if (stack->top == NULL)
    {
        return NULL;
    }

    // extreu el node superior de la pila
    struct my_stack_node *top = stack->top;
    void *data = top->data;

    stack->top = top->next;

    // alliberar la memòria del node
    free(top);

    return data;
}
