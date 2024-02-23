#include "ficheros_basico.h"

int tamMB(unsigned int nbloques) {
    int tmMB = (nbloques/8)/BLOCKSIZE;

    if((nbloques/8)%BLOCKSIZE != 0) {
        tmMB++;
    }

    return tmMB;
}

int initMB() {
    
}