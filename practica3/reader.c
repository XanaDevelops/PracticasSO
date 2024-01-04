/**ENTREGA 3
 * versió: 21/12/32
 * autors: Josep Ferriol Font, Daniel García Vázquez
 * i Biel Perelló Perelló
 */

#define _POSIX_C_SOURCE 200112L

#include <limits.h>
#include "my_lib.h"

/*Màxim dels elements que es mostraran*/
#define SHOW 10

int main(int argc, char *argv[])
{
    /*Reconstrucció pila dins memòria*/
    struct my_stack *pila;
     if (argv[1] == NULL) { 
        perror("reader(): Pila inexistente [argv[1] == NULL]");
        return -1;
    }
    pila = my_stack_read(argv[1]);

    /*Declaració dades a imprimir*/
    int element;
    int sum = 0;
    int min = INT_MAX;
    int max = 0;
    int num = my_stack_len(pila);
    printf("\nLlargaria: %d\n", num);

    /*Si la pila té més de 10 elements, els 
    càlculs només tindran en compte els 10 primers*/
    if (num > SHOW)
    {
        num = SHOW;
    }

    for (int i = 0; i < num; i++)
    {
        element = (int) my_stack_pop(pila);
        printf("%d- %d\n", i, element);

        /*Actualització dades*/
        sum +=element;
        if(element > max){
            max = element;
        }
        if(element < min){
            min = element;
        }
    }
    printf("RESUM:\n Suma: %d \n Mínim: %d \n Màxim: %d \n Mitja: %d\n", sum, min, max, ((int) (sum/num)));

    return 0;
}
