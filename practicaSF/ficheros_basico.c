#include <limits.h>
#include "ficheros_basico.h"

#define DEBUG2 0
#define DEBUG3 0

//*******************************************TAMAÑOS INICIALIZACIÓN***********************************************
int tamMB(unsigned int nbloques)
{
    // Calcular el número de bytes necesarios para representar el mapa de bits
    // Dividir este número por el tamaño de bloque para obtener el número de bloques necesarios.
    int tmMB = (nbloques / 8) / BLOCKSIZE;

    // Si el número de bytes necessarios no caben en un número exacto de bloques,
    // significa que necesitamos un bloque adicional para cubrir los bytes restantes.
    if ((nbloques / 8) % BLOCKSIZE != 0)
    {
        tmMB++;
    }

    // Devolver el tamaño calculado del mapa de bits en bloques
    return tmMB;
}

int tamAI(unsigned int ninodos)
{
    // Calcular el número de bytes necesarios para representar el array de inodos
    int tam = (ninodos * INODOSIZE) / BLOCKSIZE;

    // Si el número de bytes necessarios no caben en un número exacto de bloques,
    // significa que necesitamos un bloque adicional para cubrir los bytes restantes.
    if ((ninodos * INODOSIZE) % BLOCKSIZE != 0)
    {
        tam++;
    }

    // Devolver el tamaño calculado
    return tam;
}

//*******************************************INICIALIZACIÓN***********************************************

int initSB(unsigned int nbloques, unsigned int ninodos)
{
    struct superbloque sb;
    // inicializar
    sb.posPrimerBloqueMB = posSB + tamSB;
    sb.posUltimoBloqueMB = sb.posPrimerBloqueMB + tamMB(nbloques) - 1;
    sb.posPrimerBloqueAI = sb.posUltimoBloqueMB + 1;
    sb.posUltimoBloqueAI = sb.posPrimerBloqueAI + tamAI(ninodos) - 1;
    sb.posPrimerBloqueDatos = sb.posUltimoBloqueAI + 1;
    sb.posUltimoBloqueDatos = nbloques - 1;
    sb.posInodoRaiz = 0;
    sb.posPrimerInodoLibre = 0;
    sb.cantBloquesLibres = nbloques;
    sb.cantInodosLibres = ninodos;
    sb.totBloques = nbloques;
    sb.totInodos = ninodos;

#if DEBUG2
    fprintf(stderr, GRAY "[initSB() -> sb.posPrimerBloqueMB es %d\n]" RESET, sb.posPrimerBloqueMB);
#endif

    // escribir
    if (bwrite(posSB, &sb) == FALLO)
    {
        return FALLO;
    }
    return EXITO;
}

int initMB()
{
    // Leer el superbloque para obtener la información del sistema de archivos
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        fprintf(stderr, RED "ERROR: initMB(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }

    // Calcular la cantidad total de bloques que ocupan los metadatos del sistema de archivos
    int bloquesMetaDatos = tamSB + tamMB(SB.totBloques) + tamAI(SB.totInodos);

    // Declarar un buffer para almacenar los datos del mapa de bits
    char bufferMB[BLOCKSIZE];

    // Calcular el resto de dividir la cantidad total de bloques de metadatos entre el tamaño de bloque
    // Esto nos da la cantidad de bytes restantes del último bloque
    int bytesRestantesOcupados = (bloquesMetaDatos / 8) % BLOCKSIZE;

    // Calcular la cantidad de bits que necesitaremos para los bloques de metadatos ocupados
    int bloquesOcupados = (bloquesMetaDatos / 8) / BLOCKSIZE;

    // Si necesitamos al menos un bloque para almacenar los bits de metadatos
    if (bloquesOcupados >= 1)
    {
        // Llenar el buffer con bytes a 1 (todos los bits a 1)
        memset(bufferMB, 255, BLOCKSIZE);

        // Escribir los bloques completos ocupados de metadatos en el dispositivo virtual
        for (int i = 0; i < bloquesOcupados; i++)
        {
            if (bwrite(SB.posPrimerBloqueMB + i, bufferMB) == -1)
            {
                fprintf(stderr, RED "ERROR: initMB(): No se ha podido escribir el bloque en el dispositivo\n" RESET);
                return FALLO;
            }
        }
    }

    // Si hay bytes restantes que no ocupan un bloque completo
    if (bytesRestantesOcupados > 0)
    {
        // Llenar los bytes completos con bits a 1
        for (int i = 0; i < bytesRestantesOcupados; i++)
        {
            bufferMB[i] = 255;
        }

        // Calcular la cantidad de bits restantes en el último byte
        int bitsRestantesOcupados = bloquesMetaDatos % 8;

        // Si hay bits restantes que no ocupan un byte completo
        if (bitsRestantesOcupados > 0)
        {
            // Establecer los bits a 1 en el último byte
            for (int i = 0; i < bitsRestantesOcupados; i++)
            {
                bufferMB[bytesRestantesOcupados] |= (1 << (7 - i));
            }

            // Incrementar el valor de bytes restantes para ajustar la siguiente posición a modificar
            bytesRestantesOcupados++;
        }

        // Poner 0s en los bytes restantes del buffer
        for (int i = bytesRestantesOcupados; i < BLOCKSIZE; i++)
        {
            bufferMB[i] = 0;
        }

        // Escribir el último bloque del mapa de bits en el dispositivo virtual
        if (bwrite(SB.posPrimerBloqueMB + bloquesOcupados, bufferMB) == -1)
        {
            fprintf(stderr, RED "ERROR: initMB(): No se ha podido escribir el bloque en el dispositivo\n" RESET);
            return FALLO;
        }
    }
    // Restar cantidad de bloques libres
    SB.cantBloquesLibres = SB.cantBloquesLibres - bloquesMetaDatos;

    // Salvar el campo SuperBloque
    if (bwrite(posSB, &SB) == -1)
    {
        fprintf(stderr, RED "ERROR: initMB(): No se ha podido salvar SB\n" RESET);
        return FALLO;
    }
    return EXITO;
}

int initAI()
{
    // Leer el superbloque para obtener la información del sistema de archivos
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        fprintf(stderr, RED "ERROR: initAI(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }

    // Declarar un buffer
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    // Contador de inodos
    int contInodos = SB.posPrimerInodoLibre + 1;

    // Iterar los bloques de inodos
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {
        // Leer un bloque en el array de inodos
        if (bread(i, inodos) == -1)
        {
            fprintf(stderr, RED "ERROR: initAI(): No se ha podido leer el bloque en el dispositivo\n" RESET);
            return FALLO;
        }
        // Iterar para cada bloque de inodos (desde el primer bloque hasta el último)
        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++)
        {
            inodos[j].tipo = 'l';

            // Enlazar los inodos
            // Si no es el último, inicialmente enlazamos cada uno apunta con el siguiente
            if (contInodos < SB.totInodos)
            {
                inodos[j].punterosDirectos[0] = contInodos;
                contInodos++;
            }
            // Si es el último, hacemos que apunte al máximo valor para un unsigned int
            else
            {
                inodos[j].punterosDirectos[0] = UINT_MAX;
                break;
            }
        }

        // Escribir bloque AI actualizado
        if (bwrite(i, inodos) == FALLO)
        {
            fprintf(stderr, RED "ERROR: initAI(): No se ha podido escribir el bloque en el dispositivo\n" RESET);
            return FALLO;
        }
    }

    return EXITO;
}

//*******************************************BIT***********************************************

int escribir_bit(unsigned int nbloque, unsigned int bit)
{
    // Leer el superbloque para obtener la información del sistema de archivos
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        fprintf(stderr, RED "ERROR: escribir_bit(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }

    // Calcular que byte contiene el bit que representa el bloque nbloque en MB
    int posbyte = nbloque / 8;
    // Calcular la posición del bit dentro del byte
    int posbit = nbloque % 8;
    // Hallar en que bloque del MB se encuentra el byte
    int nbloqueMB = posbyte / BLOCKSIZE;
    // Obtener la posición absoluta en el dispositivo virtual del bloque
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    // Declarar un buffer para almacenar el bloque que contiene el bit a escribir
    char bufferMB[BLOCKSIZE];

    // Leer el bloque físico que contiene el bit y cargar su contenido en bufferMB
    if (bread(nbloqueabs, bufferMB) == -1)
    {
        fprintf(stderr, RED "ERROR: escribir_bit(): No se ha podido leer el bloque %d del dispositivo\n" RESET, nbloqueabs);
        return FALLO;
    }

    // Localizar la posición del byte indicado que contiene el bit a escribir en el bloque
    posbyte = posbyte % BLOCKSIZE;
    // Declarar mascara para poder modificar el bit
    unsigned char mascara = 128;
    // Desplazar la mascara tantos bits como indica posbit
    mascara >>= posbit;

    // Comprobar si se debe escribir un 1 o un 0 en el bit deseado
    if (bit == 1)
    {
        // Realizar una operación OR para preservar los otros bits y solo modificar el indicado con un 1
        bufferMB[posbyte] |= mascara;
    }
    else if (bit == 0)
    {
        // Realizar una operación AND para preservar los otros bits y solo modificar el indicado con un 0
        bufferMB[posbyte] &= ~mascara;
    }

    // Escribir el bufferMB con el bit modificado en la posición nbloqueabs
    if (bwrite(nbloqueabs, bufferMB) == -1)
    {
        fprintf(stderr, RED "ERROR: escribir_bit(): No se ha podido escribir en el bloque %d del dispositivo\n" RESET, nbloqueabs);
        return FALLO;
    }

    // Devolver el número de bloque absoluto si se ha realizado con éxito la escritura
    return nbloqueabs;
}

char leer_bit(unsigned int nbloque)
{
    // Leer el superbloque para obtener la información del sistema de archivos
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        fprintf(stderr, RED "ERROR: leer_bit(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }

    // Calcular que byte contiene el bit que representa el bloque nbloque en MB
    int posbyte = nbloque / 8;
    // Calcular la posición del bit dentro del byte
    int posbit = nbloque % 8;
    // Hallar en que bloque del MB se encuentra el byte
    int nbloqueMB = posbyte / BLOCKSIZE;
    // Obtener la posición absoluta en el dispositivo virtual del bloque
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    // Declarar un buffer para almacenar el bloque que contiene el bit a leer
    char bufferMB[BLOCKSIZE];

    // Leer el bloque físico que contiene el bit y cargar su contenido en bufferMB
    if (bread(nbloqueabs, bufferMB) == -1)
    {
        fprintf(stderr, RED "ERROR: leer_bit(): No se ha podido leer el bloque %d del dispositivo\n" RESET, nbloqueabs);
        return FALLO;
    }

    // Localizar la posición del byte indicado que contiene el bit a leer en el bloque
    posbyte = posbyte % BLOCKSIZE;
    // Declarar mascara para poder leer el bit
    unsigned char mascara = 128;
    // Desplazar la mascara tantos bits como indica posbit
    mascara >>= posbit;
    // Operador AND sobre la mascara para obtener el valor del bit a leer
    mascara &= bufferMB[posbyte];
    // Desplazar bits a la derecha para dejar el valor del bit en el extremo derecho
    mascara >>= (7 - posbit);

#if DEBUG3
    fprintf(stderr, GRAY "[leerbit(%d) -> posbyte: %d, posbyte (ajustado): %d, posbit: %d, nbloqueMB: %d, nbloqueabs: %d]\n]" RESET,
            nbloque, nbloque / 8, posbyte, posbit, nbloqueMB, nbloqueabs);
#endif

    // Devolver el valor leído del bit
    return mascara;
}

//*******************************************BLOQUES***********************************************
/**
 * Encuentra el primer bloque libre,
 * lo ocupa y devuelve su posición.
 */
int reservar_bloque()
{
#if DEBUG3
    fprintf(stderr, GRAY "reservar_bloque(): inicio reserva bloque\n" RESET);
#endif
    struct superbloque sb;
    if (bread(posSB, &sb))
    {
        fprintf(stderr, RED "ERROR: reservar_bloque(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }
    if (sb.cantBloquesLibres == 0)
    {
        fprintf(stderr, RED "ERROR: reservar_bloque(): no hay bloques libres!!\n" RESET);
        return FALLO;
    }

#if DEBUG3
    fprintf(stderr, GRAY "reservar_bloque(): superbloque reporta %d bloques libres\n" RESET, sb.cantBloquesLibres);
#endif

    unsigned char *bufferAux = malloc(BLOCKSIZE);
    if (bufferAux == NULL)
    {
        perror(RED "reservar_bloque(): ERROR malloc() bufferAux\n" RESET);
        free(bufferAux);
        return FALLO;
    }

    memset(bufferAux, 255, BLOCKSIZE);

    unsigned char *bufferMB = malloc(BLOCKSIZE);
    if (bufferMB == NULL)
    {
        perror(RED "reservar_bloque(): ERROR malloc() bufferMB\n" RESET);
        free(bufferAux);
        free(bufferMB);
        return FALLO;
    }

    unsigned int nBloqueMB = 0;
#if DEBUG3
    fprintf(stderr, GRAY "reservar_bloque(): ultimoMB:%d\n" RESET, sb.posUltimoBloqueMB);
#endif
    // buscar bloque
    while (nBloqueMB < sb.posUltimoBloqueMB)
    {
#if DEBUG3
        fprintf(stderr, GRAY "reservar_bloque(): comprobando nBloqueMB:%d\n" RESET, nBloqueMB);
#endif
        bread(nBloqueMB, bufferMB);
        if (memcmp(bufferAux, bufferMB, BLOCKSIZE))
        {
#if DEBUG3
            fprintf(stderr, GRAY "reservar_bloque(): bloqueMB:%d diferente de aux\n" RESET, nBloqueMB);
#endif
            break;
        }
        nBloqueMB++;
    }

    // buscar byte
    int posbyte;
    for (posbyte = 0; posbyte < BLOCKSIZE; posbyte++)
    {
        if (bufferMB[posbyte] == 255)
        {
#if DEBUG3
            fprintf(stderr, GRAY "reservar_bloque(): posByte:%d diferente de 255\n" RESET, posbyte);
#endif
            break;
        }
    }

    // buscar bit
    unsigned char mask = 128;
    int posbit;
    for (posbit = 0; posbit < 8; posbit++)
    {
        if (bufferMB[posbyte] & mask)
        {
            bufferMB[posbyte] <<= 1;
        }
        else
        {
#if DEBUG3
            fprintf(stderr, GRAY "reservar_bloque(): posBit:%d diferente de mask\n" RESET, posbit);
#endif
            break;
        }
    }

    int nbloque = (nBloqueMB * BLOCKSIZE + posbyte) * 8 + posbit;
#if DEBUG3
    fprintf(stderr, GRAY "reservar_bloque(): nBloque:%d\n" RESET, nbloque);
#endif

    // guardar valores
    sb.cantBloquesLibres--;
    if (bwrite(posSB, &sb))
    {
        fprintf(stderr, RED "reservar_bloque: ERROR guardar SB\n" RESET);
        free(bufferAux);
        free(bufferMB);
        return FALLO;
    }

    // reservar en MB
    escribir_bit(nbloque, 1);

    // limpiar bloque
    memset(bufferAux, 0, BLOCKSIZE);
    bwrite(nbloque, bufferAux);

    free(bufferAux);
    free(bufferMB);
    return nbloque;
}

/**
 * Mediante escribir_bit actualiza MB,
 *  libera un bloque y devuelve su posición.
 */
int liberar_bloque(unsigned int nbloque)
{
    // leer superbloque
    struct superbloque sb;
    if (bread(posSB, &sb))
    {
        fprintf(stderr, RED "ERROR: liberar_bloque(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }
    // liberar bloque en el mapa de bits
    escribir_bit(nbloque, 0);

    // Modificar el nº de bloques libres y salvar SB
    sb.cantBloquesLibres++;
    if (bwrite(posSB, &sb))
    {
        fprintf(stderr, RED "liberar_bloque: ERROR guardar SB\n" RESET);
        return FALLO;
    }
    return nbloque;
}

//*******************************************INODOS***********************************************

int escribir_inodo(unsigned int ninodo, struct inodo *inodo)
{
    // Leer el superbloque para obtener la información del sistema de archivos
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        fprintf(stderr, RED "ERROR: escribir_inodo(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }

    // Declarar buffer de lectura de array de inodos
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    // Calcular el numero de bloque dentro del array de inodos del inodo solicitado
    int nbloqueAI = (ninodo * INODOSIZE) / BLOCKSIZE;
    // Obtener posición absoluta del bloque en el dispositivo virtual
    int nbloqueabs = nbloqueAI + SB.posPrimerBloqueAI;

    // Lectura del bloque que contiene el numero de inodo a escribir
    if (bread(nbloqueabs, inodos) == -1)
    {
        fprintf(stderr, RED "ERROR: escribir_inodo(): No se ha podido leer el bloque %d del dispositivo\n" RESET, nbloqueabs);
        return FALLO;
    }

    // Calcular la posición del inodo a escribir dentro del array de inodos
    int posinodo = ninodo % (BLOCKSIZE / INODOSIZE);
    // Escribir el contenido del inodo pasado por parámetro en la posición correspondiente
    inodos[posinodo] = *inodo;

    // Escribir el buffer inodos modificado en el dispositivo virtual
    if (bwrite(nbloqueabs, inodos) == -1)
    {
        fprintf(stderr, RED "ERROR: escribir_inodo(): No se ha podido escribir en el bloque %d del dispositivo\n" RESET, nbloqueabs);
        return FALLO;
    }
    // Devolver EXITO en caso de operación correcta
    return EXITO;
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{
    // Leer el superbloque para obtener la información del sistema de archivos
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        fprintf(stderr, RED "ERROR: leer_inodo(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }

    // Calcular el numero de bloque dentro del array de inodos del inodo solicitado
    int nbloqueAI = (ninodo * INODOSIZE) / BLOCKSIZE;
    // Obtener posición absoluta del bloque en el dispositivo virtual
    int nbloqueabs = nbloqueAI + SB.posPrimerBloqueAI;

    // Declarar buffer de lectura de array de inodos
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    // Lectura del bloque que contiene el numero de inodo a escribir
    if (bread(nbloqueabs, inodos) == -1)
    {
        fprintf(stderr, RED "ERROR: leer_inodo(): No se ha podido leer el bloque %d del dispositivo\n" RESET, nbloqueabs);
        return FALLO;
    }

    // Calcular la posición del inodo a escribir dentro del array de inodos
    int posinodo = ninodo % (BLOCKSIZE / INODOSIZE);

    *inodo = inodos[posinodo];

    // Devolver EXITO en caso de operación correcta
    return EXITO;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
    fprintf(stderr, YELLOW "WARNING: reservar_inodo INCOMPLETO\n" RESET);

    int posInodoReservado;

#if DEBUG3
    fprintf(stderr, GRAY "reservar_inodo(): inicio reserva inodo\n" RESET);
#endif
    struct superbloque sb;
    if (bread(posSB, &sb))
    {
        fprintf(stderr, RED "ERROR: reservar_inodo(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }
    
    if (sb.cantInodosLibres == 0)
    {
        fprintf(stderr, RED "ERROR: reservar_inodo(): No hay inodos libres\n" RESET);
        return FALLO;
    }

    struct inodo inodoReservado;
    posInodoReservado = sb.posPrimerInodoLibre; // TODO: no se exactamente como va, mañana lo miro....

    
    leer_inodo(posInodoReservado, &inodoReservado);

    //inicializar
    inodoReservado.tipo = tipo;
    inodoReservado.permisos = permisos;
    inodoReservado.nlinks = 1;
    inodoReservado.tamEnBytesLog = 0;
    inodoReservado.atime = time(NULL);
    inodoReservado.ctime = time(NULL);
    inodoReservado.mtime = time(NULL);
    inodoReservado.numBloquesOcupados = 0;

    for (int i = 1; i < sizeof(inodoReservado.punterosDirectos) / sizeof(unsigned int); i++)
    {
        inodoReservado.punterosDirectos[i] = 0;
    }
    for (int i = 0; i < sizeof(inodoReservado.punterosIndirectos) / sizeof(unsigned int); i++)
    {
        inodoReservado.punterosIndirectos[i] = 0;
    }

    escribir_inodo(posInodoReservado, &inodoReservado);
    sb.cantInodosLibres--;
    sb.posPrimerInodoLibre++; //Esta bien?
    bwrite(posSB, &sb);

    return posInodoReservado;
}

int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr) 
{

}

int obtener_indice(unsigned int nblogico, int nivel_punteros) 
{
    // Comprobar en que puntero de bloques y nivel se encuentra el bloque lógico y devolver su índice.
    if (nblogico < DIRECTOS) {
        // Devolver el índice directo de la lista de punteros
        return nblogico;
    } else if (nblogico < INDIRECTOS0) {
        // Devolver el índice menos los bloques directos anteriores
        return nblogico - DIRECTOS;
    } else if (nblogico < INDIRECTOS1) {
        if (nivel_punteros == 2) {
            // Devolver el índice del bloque que se encuentra en el nivel más alto 
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        } else if (nivel_punteros == 1) {
            // Devolver el índice del bloque que se encuenctra en el nivel más bajo
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    } else if (nblogico < INDIRECTOS2) {
        if (nivel_punteros == 3) {
            // Devolver el índice del bloque que se encuentra en el nivel más alto
            return (nblogico - INDIRECTOS1) / (NPUNTEROS*NPUNTEROS);
        } else if (nivel_punteros == 2) {
            // Devolver el índice del bloque que se encuentra en el nivel medio
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS*NPUNTEROS)) / NPUNTEROS;
        } else if (nivel_punteros == 1) {
            // Devolver el índice del bloque que se encuentra en el nivel más bajo
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS*NPUNTEROS)) % NPUNTEROS;
        }
    }
}

int traducir_bloque_inodo(struct inodo *inodo, unsigned int nblogico, unsigned char reservar)
{
    // Declarar variables para los cálculos pertinentes
    unsigned int ptr, ptr_ant;
    int nRangoBL, nivel_punteros, indice;
    unsigned int buffer[NPUNTEROS];

    // Inicializar los punteros a bloque lógicos
    ptr = 0;
    ptr_ant = 0;

    // Obtener el rango de bloque lógico en el que nos encontramos
    nRangoBL = obtener_nRangoBL(inodo, nblogico, &ptr);
    nivel_punteros = nRangoBL;

    // Iterar para cada nivel de punteros indirectos
    while(nivel_punteros > 0) {
        // No cuelgan bloque de punteros
        if (ptr == 0) {
            if (reservar == 0) {
                // Bloque inexistente
                return -1;
            } else {
                // Reservar bloques de punteros
                ptr = reservar_bloque();
                // Aumentar el número de bloque ocupados por el indodo
                inodo -> numBloquesOcupados++;
                // Almacenar la fecha actual en el inodo
                inodo -> ctime = time(NULL);
            }

        }
    }
}
