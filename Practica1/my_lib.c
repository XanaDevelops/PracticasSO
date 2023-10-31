/**ENTREGA 1
 * versió: 30/10/32
 * autors: Josep Ferriol Font, Daniel García Vázquez
 * i Biel Perelló Perelló
 */

/**lib.c librería con las funciones equivalentes a las de
 * <string.h> y  * las funciones y estructuras para el manejo
 * de una pila
 */
#include "my_lib.h"

/*REPTE 1*/
#define FPERMS 0666
int recursive_write(struct my_stack_node *node, int *file, int *size);
/**
 * Funció: my_strlen
 * -------------------
 * Calcula la llargaria de la cadena de caràcters
 *
 * param: str --> punter de la cadena de caràcters ha calcular
 * llargaria
 * return: llargaria fora element centinela de la cadena str
 */
size_t my_strlen(const char *str)
{
    size_t len = 0;
    /*Mentre element d'índex len de str sigui diferent del caràcter
    centinela increment len */
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
 * param: dest --> Cadena de caràcters on s'han de copiar els
 * elements
 *        src --> Cadena de caràcters de la qual s'han de copiar els
 *  elements
 * return: el punter a l'inici de l'array dest on s'ha copiat src
 * */
char *my_strcpy(char *dest, const char *src)
{
    int i = 0;
    /*Fins no troba element centinela assignar els valors de src a dst*/
    while (src[i])
    {
        dest[i] = src[i];
        i++;
    }
    /*Finalment posa el caràcter centinela i retorna el punter*/
    dest[i] = '\0';

    return dest;
}

/**
 * Funció: my_strncpy
 * -------------------
 * Copia els n primers elements d'una cadena de caràcters a una altra.
 * param: dest --> Cadena de caràcters on s'han de copiar els
 *                  elements
 *        src --> Cadena de caràcters de la qual s'han de copiar els
 *                  elements
 *         n --> nombre d'elements que s'ha de copiar
 * return: el punter a l'inici de l'array dest on s'ha copiat src
 * */
char *my_strncpy(char *dest, const char *src, size_t n)
{
    int i = 0;
    /*Fins no troba element centinela o arriba els n elements
    copiats, assignar els valors de src a dst*/
    while ((src[i]) && (n > 0))
    {
        dest[i] = src[i];
        i++;
        n--;
    }
    /*Sino s'ha arribat a n oblir amb 0*/
    while (n > 0)
    {
        dest[i] = '\0';
        i++;
        n--;
    }

    return dest;
}

/**
 * Funció: my_strcat
 * -------------------
 * Concatedenació de dues cadenes de caràcters, afegeix src a dest
 * param: dest --> punter on s'han d'afegir els nous elements
 *        src --> cadena dels caràcters que s'han d'afegir
 * return: el punter a l'inici de l'array dest on hi ha els elements de dst+src
 * */
char *my_strcat(char *dest, const char *src)
{
    int i = 0;
    /*Cercam el darrer element de dst*/
    while (dest[i] != '\0')
    {
        i++;
    }
    /*Sobreescrivim el centinela de dest i copiam src darrera*/
    for (int j = 0; src[j]; j++)
    {
        dest[i] = src[j];
        i++;
    }
    /*Afegim l'element nul o centinela*/
    dest[i] = '\0';

    return dest;
}

/**
 * Funció: my_strchr
 * -------------------
 * Cerca de la primera aparició del caràcter pasat per paràmetre
 * param: str --> punter on s'ha de cercar el caràcter
 *        c --> paràmetre a cercar
 * return: si l'element hi és retorna la seva posició, sino null
 * */
char *my_strchr(const char *str, int c)
{
    /*Fa una cerca del caràcter*/
    while (*str != (char)c && *str)
    {
        str++;
    }

    /*Si la trobat retorna la seva posició, sino retorna null*/
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
/**
 * Funció: my_stack_init
 * -------------------
 * Reserva un espai de memòria per la pila
 *
 * param: int size --> tamany de les dades
 * return: punter a la pila inicialitzada
 */
struct my_stack *my_stack_init(int size)
{
    // guardam memòria per la pila
    struct my_stack *stack;
    stack = malloc(sizeof(struct my_stack));

    // comprovam error
    if (stack == NULL)
    {
        perror("Error malloc my_stack_init()\n");
        return NULL;
    }

    // declaram les dades de la pila
    stack->size = size;
    stack->top = NULL;

    // retornam punter a la pila
    return stack;
}

/**
 * Funció: my_stack_push
 * -------------------
 * Inserta un nou node als elements de la pila
 *
 * param: struct my_stack *stack --> punter a la pila
 *        void *data --> punter a les dades del node a introduir
 * return: retorna 0 si s'ha produït correctament el push, 0 en cas contrari
 */
int my_stack_push(struct my_stack *stack, void *data)
{
    // comprovar si la pila està inicialitzada
    if (stack == NULL || stack->size <= 0)
    {
        perror("Error: Pila no inicialitzada o mida no vàlida.\n");
        return -1;
    }

    // guardam memòria per el node a introduir
    struct my_stack_node *node;
    node = malloc(sizeof(struct my_stack_node));

    // comprovar si el node és null
    if (node == NULL)
    {
        perror("Error: No s'ha pogut inserir l'element a la pila.\n");
        return -1;
    }

    // guardam memòria per les dades del node
    node->data = malloc(stack->size);
    if (node->data == NULL)
    {
        perror("Error: No se pudo asignar memoria para el elemento en la pila.\n");
        return -1;
    }

    // gestió de la pila (top) i el nou node a introduïr
    node->data = data;
    node->next = stack->top;
    stack->top = node;

    // el push s'ha produït correctament
    return 0;
}

/**
 * Funció: my_stack_pop
 * -------------------
 * Elimina el node superior dels elements de la pila
 *
 * param: struct my_stack *stack --> punter a la pila
 * return: punter a les dades del node/element eliminat
 */
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

    // retorna el punter a les dades del node eliminat
    return data;
}

/**
 * Funció: my_stack_len
 * -------------------
 * Recompta amb un recorregut el nombre de nodes
 *
 * param: struct my_stack *stack --> punter a la pila
 * return: el nombre de nodes de la pila
 */
int my_stack_len(struct my_stack *stack)
{
    int len = 0;
    /*Primer comprova que Stack hagi estat inacializat*/
    if (stack == NULL)
    {
        return -1;
    }
    /*Ens situam en el node top i mentres no sigui
    num iteram damunt la pila i incrementam len*/
    struct my_stack_node *node = stack->top;
    while (node != NULL)
    {
        /*Actualizam amb el pròxim node*/
        node = node->next;
        len++;
    }

    return len;
}

int my_stack_purge(struct my_stack *stack)
{
    int bytes = 0;
    if (stack == NULL)
    {
        return -1;
    }
    struct my_stack_node *node;
    while (stack->top != NULL)
    {
        node = stack->top->next;
        bytes += sizeof(node);
        free(stack->top);
        stack->top = node;
    }
    bytes += sizeof(stack);
    free(stack);

    return bytes;
}

struct my_stack *my_stack_read(char *filename)
{
    /*Obrim el fitxer i en cas d'error retornam NULL*/
    int fitxer = open(filename, O_RDONLY) if (fitxer == -1)
    {
        perror("ERROR: Fitxer no es pot obrir");
        return NULL;
    }
    /*Miram la mida del fitxer per inincialtzar pila*/
    int mida_fit;
    int r = read(fitxer, &mida_fit, sizeof(int));
    if (r == -1)
    {
        perror("ERROR: No es pot llegir del fitxer my_stack_read\n");
        return NULL;
    }
    /*Inicialitzam la pila i el punter on es guardaran les dades*/
    struct my_stack *stack = my_stack_init(mida_fit);
    void *data;

    do
    {
        /*Reservam memòria per la dada que anam a llegir*/
        data = malloc(mida_fit);
        if (dato == NULL)
        {
            perror("ERROR: malloc my_stack_read\n");
            return NULL;
        }
        /*Llegim i si la lectura es correcta feim un push*/
        r = read(fitxer, data, mida_fit);
        if (r > 0)
        {
            my_stack_push(stack, data);
        }
    } while (r > 0);

    free(data);

    if (close(fitxer) == -1)
    {
        perror("ERROR: tancant el fitxer my_stack_read\n");
        return NULL;
    }

    return stack;
}

int my_stack_write(struct my_stack *stack, char *filename)
{

    if (stack == NULL)
    {
        return -1;
    }
    int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, FPERMS);
    if (file == -1)
    {
        perror("ERROR: open file my_stack_write\n");
        return -1;
    }
    int r = write(file, &(stack->size), sizeof(int) - 1);
    r += recursive_write(stack->top, &file, &(stack->size));
    return 0; // PLACEHOLDER
}

int recursive_write(struct my_stack_node *node, int *file, int *size)
{
    int r = 0;
    if (node->next != NULL)
    {
        r = recursive_write(node->next, file, size - 1);
    }
    if (r == -1)
    {
        return -1;
    }
    return write(*file, node->data, *size - 1) + r;
}