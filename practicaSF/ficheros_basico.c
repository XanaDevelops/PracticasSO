#include "ficheros_basico.h"

int tamMB(unsigned int nbloques) {
    // Calcular el número de bytes necesarios para representar el mapa de bits
    // Dividir este número por el tamaño de bloque para obtener el número de bloques necesarios.
    int tmMB = (nbloques/8)/BLOCKSIZE;

    // Si el número de bytes necessarios no caben en un número exacto de bloques,
    // significa que necesitamos un bloque adicional para cubrir los bytes restantes.
    if((nbloques/8)%BLOCKSIZE != 0) {
        tmMB++;
    }

    // Devolver el tamaño calculado del mapa de bits en bloques
    return tmMB;
}

int initMB() {
    // Leer el superbloque para obtener la información del sistema de archivos
    struct superbloque SB;
    if(bread(posSB, &SB) == -1) {
        perror("Error");
        return FALLO;
    }

    // Calcular la cantidad total de bloques que ocupan los metadatos del sistema de archivos
    int bloquesMetaDatos = tamSB + tamMB(SB.totBloques) + tamAI(SB.totInodos);

    // Declarar un buffer para almacenar los datos del mapa de bits
    char bufferMB[BLOCKSIZE];

    // Calcular el resto de dividir la cantidad total de bloques de metadatos entre el tamaño de bloque
    // Esto nos da la cantidad de bytes restantes del último bloque
    int bytesRestantesOcupados = (bloquesMetaDatos/8)%BLOCKSIZE;

    // Calcular la cantidad de bits que necesitaremos para los bloques de metadatos ocupados
    int bloquesOcupados = (bloquesMetaDatos/8)/BLOCKSIZE;

    // Si necesitamos al menos un bloque para almacenar los bits de metadatos
    if(bloquesOcupados >= 1) {
        // Llenar el buffer con bytes a 1 (todos los bits a 1)
        memset(bufferMB, 255, BLOCKSIZE);

        // Escribir los bloques completos ocupados de metadatos en el dispositivo virtual
        for(int i = 0; i < bloquesOcupados; i++) {
            if(bwrite(SB.posPrimerBloqueMB + i, bufferMB) == -1) {
                perror("Error");
                return FALLO;
            }
        }
    }

    // Si hay bytes restantes que no ocupan un bloque completo
    if(bytesRestantesOcupados > 0) {
        // Llenar los bytes completos con bits a 1
        for(int i = 0; i < bytesRestantesOcupados; i++) {
            bufferMB[i] = 255;
        }
        
        // Calcular la cantidad de bits restantes en el último byte
        int bitsRestantesOcupados = bloquesMetaDatos%8;
        
        // Si hay bits restantes que no ocupan un byte completo
        if(bitsRestantesOcupados > 0) {
            // Establecer los bits a 1 en el último byte
            for (int i = 0; i < bitsRestantesOcupados; i++) {
                bufferMB[bytesRestantesOcupados] |= (1 << (7 - i));
            }

            // Incrementar el valor de bytes restantes para ajustar la siguiente posición a modificar
            bytesRestantesOcupados++;
        }

        // Poner 0s en los bytes restantes del buffer
        for (int i = bytesRestantesOcupados; i < BLOCKSIZE; i++) {
            bufferMB[i] = 0;
        }

        // Escribir el último bloque del mapa de bits en el dispositivo virtual
        if(bwrite(SB.posPrimerBloqueMB + bloquesOcupados, bufferMB) == -1) {
            perror("Error");
            return FALLO;
        }
    }

    return EXITO;
}