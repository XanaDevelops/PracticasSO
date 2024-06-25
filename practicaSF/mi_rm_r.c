/*García Vázquez, Daniel*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "directorios.h"

#define DEBUGCP 1

/**
 * Copia un fichero, o directorio del origen a destino
*/
int main(int argc, char **argv)
{
    if (argc < 3){
        fprintf(stderr, RED "ERROR: mi_rm_r.c -> USO: ./mi_rm_r <disco> </ruta>\n" RESET);
        return FALLO;
    }

    char* disco = argv[1];
    char* ruta = argv[2];


    #if DEBUGEXTRA
    fprintf(stderr, GRAY "[mi_rm_r.c: ruta:%s]\n" RESET, ruta);
    #endif

    //comprobar copiar directorio a directorio, o fichero a fichero
    char tipo;
    if(*(ruta+strlen(ruta)-1) == '/'){
        tipo = 'd';
    }else{
        tipo = 'f';
    }
    
    if (strcmp(ruta, "/") == 0)
    {
        fprintf(stderr, RED "Error: mi_rm -> No se ha de poder borrar el directorio raíz\n" RESET);
        return FALLO;
    }

    if(bmount(disco)==FALLO){
        fprintf(stderr, RED "ERROR: mi_cp() -> no se ha podido montar %s\n" RESET, disco);
        return FALLO;
    }
    if(tipo == 'f'){
        if(mi_unlink(ruta)== FALLO){
            bumount();
            return FALLO;
        }
    }else{
        if(mi_unlink_r(ruta)== FALLO){
            bumount();
            return FALLO;
        }
    }
    

    return EXITO;
}