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
    if (argc < 4){
        fprintf(stderr, RED "ERROR: mi_cp.c -> USO: ./mi_cp <disco> </origen[/]> </destino[/]>\n" RESET);
        return FALLO;
    }

    char* disco = argv[1];
    char* origen = argv[2];
    char* destino = argv[3];

    #if DEBUGCP ==  1 && DEBUGEXTRA
    fprintf(stderr, GRAY "[mi_cp.c: origen:%s, destino:%s]\n" RESET, origen, destino);
    #endif

    //comprobar copiar directorio a directorio, o fichero a fichero
    char tipoOrigen, tipoDestino;
    if(*(origen+strlen(origen)-1) == '/'){
        tipoOrigen = 'd';
    }else{
        tipoOrigen = 'f';
    }
    if(*(destino+strlen(destino)-1) == '/'){
        tipoDestino = 'd';
    }else{
        tipoDestino = 'f';
    }

    if((tipoOrigen == 'd') && (tipoDestino == 'f')){
        fprintf(stderr, RED "ERROR: mi_cp() -> no se puede copiar un directorio en un fichero\n" RESET);
        return FALLO;
    }

    if(bmount(disco)==FALLO){
        fprintf(stderr, RED "ERROR: mi_cp() -> no se ha podido montar %s\n" RESET, disco);
        return FALLO;
    }
    #if DEBUGEXTRA
    fprintf(stderr, GRAY "[mi_cp.c: %c -> %c]\n" RESET, tipoOrigen, tipoDestino);
    #endif
    if(mi_cp(origen, destino, tipoOrigen, tipoDestino)==FALLO){//son el mismo
        bumount();
        return FALLO; 
    }

    return EXITO;
}