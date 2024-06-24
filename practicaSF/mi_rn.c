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
        PRINT_ERR("ERROR mi_rm -> Uso correcto .mi_rm <disco> <ruta/antiguo> <nuevo>");
        return FALLO;
    }

    char *disco = argv[1];
    char *ruta_antigua = argv[2];
    char *nombre_nuevo = argv[3];
    
    if(bmount(disco)==FALLO){
        PRINT_ERR("mi_rm -> no se ha podido montar disco %s", disco);
        return FALLO;
    }

    //comprbar tipo
    char tipo;
    int len_ruta = strlen(ruta_antigua);
    if(ruta_antigua[len_ruta-1]=='/'){
        tipo = 'd';
    }else{
        tipo = 'f';
    }

    return mi_rn(ruta_antigua, nombre_nuevo, tipo);
}