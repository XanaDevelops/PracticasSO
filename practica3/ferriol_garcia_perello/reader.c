/**ENTREGA 3
 * versió: 21/12/32
 * autors: Josep Ferriol Font, Daniel García Vázquez
 * i Biel Perelló Perelló
 */

#define _POSIX_C_SOURCE 200112L

#include <limits.h>
#include "my_lib.h"
#include <stdio.h>

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

/*Màxim dels elements que es mostraran*/
#define SHOW 10

int main(int argc, char *argv[])
{
    /*Reconstrucció pila dins memòria*/
    struct my_stack *pila;
    if (argv[1] == NULL)
    {
        fprintf(stderr, ROJO_T "Sintaxi incorrecta. US: ./reader <stack_file>\n" RESET);
        return EXIT_FAILURE;
    }
    pila = my_stack_read(argv[1]);

    /*Declaració dades a imprimir*/
    int *element;
    int sum = 0;
    int min = INT_MAX;
    int max = 0;
    int num = my_stack_len(pila);
    if (num <= 0)
    {
        if (num == 0)
        {
            fprintf(stderr, ROJO_T "reader(): Pila buida [llargaria = 0]\n" RESET);
        }
        return EXIT_FAILURE;
    }
    printf("Llargaria: %d\n", num);

    /*Si la pila té més de 10 elements, els
    càlculs només tindran en compte els 10 primers*/
    if (num > SHOW)
    {
        num = SHOW;
    }

    for (int i = 0; i < num; i++)
    {
        element = my_stack_pop(pila);
        printf("%d- %d\n", i, *element);

        /*Actualització dades*/
        sum += *element;
        if (*element > max)
        {
            max = *element;
        }
        if (*element < min)
        {
            min = *element;
        }
    }
    printf("RESUM:\n Suma: %d \n Mínim: %d \n Màxim: %d \n Mitja: %d\n", sum, min, max, ((int)(sum / num)));

    return 0;
}
