/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include <stdio.h>
#include <stdlib.h>

#include "directorios.h"

#define TAMFILA 100
#define NFILAS 1000
#define TAMBUFFER (TAMFILA*NFILAS) //suponemos un máx de 1000 entradas, aunque debería ser SB.totInodos

int main(int argc, char **argv){
    //comprobar entrada
    if(!(argc==3 || argc==4)){
        fprintf(stderr, RED "ERROR: sintaxis correcta: ./mi_ls -l <disco> </ruta>\n" RESET);
        return FALLO;
    }

    int extendido = 0;
    if(argc==4){
        if(strcmp(argv[1], "-l")==0){
            extendido=1;
        }else{
            fprintf(stderr, RED "ERROR: sintaxis correcta: ./mi_ls -l <disco> </ruta>\n" RESET);
            return FALLO;
        }
    }

    char buff[NFILAS][TAMFILA];
    
    //llamar a mi_dir()

    return EXITO;

}