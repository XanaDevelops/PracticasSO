/*García Vázquez, Daniel*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "directorios.h"


/**
 * renombra un fichero, o directorio
*/
int main(int argc, char **argv)
{
    if(argc<3){
        PRINT_ERR("ERROR mi_mv -> Uso correcto .mi_rm <disco> <ruta/origen> <ruta/destino>");
        return FALLO;
    }

    char *disco = argv[1];
    char *origen = argv[2];
    char *destino = argv[3];

    //comprbar tipo

    int len_ruta = strlen(destino);
    if(destino[len_ruta-1]!='/'){
        PRINT_ERR("no se puede mover a un directorio");
        return FALLO;
    }
    
    if(bmount(disco)==FALLO){
        PRINT_ERR("mi_mv -> no se ha podido montar disco %s", disco);
        return FALLO;
    }

    return mi_mv(origen, destino);
}