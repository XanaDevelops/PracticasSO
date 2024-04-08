/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include "ficheros.h"

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    // LECTURA INODO
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_write_f(): No se ha podido leer el inodo %d \n" RESET, ninodo);
        return FALLO;
    }

    if ((inodo.permisos & 2) != 2)
    {
        fprintf(stderr, RED "ERROR: mi_write_f(): No hay permisos de escritura\n" RESET);
        return FALLO;
    }

    // DECLARAR VARIABLES
    int primerBL, ultimoBL, desp1, desp2, nbfisico;
    int bytesescritos = 0;
    char buf_bloque[BLOCKSIZE];

    // INICIALIZAR VARIABLES
    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    // CASO 1 BLOQUE
    if (primerBL == ultimoBL)
    {
        nbfisico = traducir_bloque_inodo(&inodo, primerBL, 1);
        bread(nbfisico, buf_bloque);
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        bwrite(nbfisico, buf_bloque);
        bytesescritos = desp2 - desp1 + 1;
    }
    else
    {
        int bl = primerBL + 1;

        // PRIMER BLOQUE LÓGICO
        nbfisico = traducir_bloque_inodo(&inodo, primerBL, 1);
        bread(nbfisico, buf_bloque);
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        bwrite(nbfisico, buf_bloque);
        bytesescritos += BLOCKSIZE - desp1;

        // BLOQUES INTERMEDIOS
        while (bl < ultimoBL)
        {
            nbfisico = traducir_bloque_inodo(&inodo, bl, 1);
            int bytes = bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (bl - primerBL - 1) * BLOCKSIZE);
            bytesescritos += bytes;
            bl++;
        }

        // ÚLTIMO BLOQUE LÓGICO
        nbfisico = traducir_bloque_inodo(&inodo, ultimoBL, 1);
        bread(nbfisico, buf_bloque);
        memcpy(buf_bloque, buf_original + (nbytes - (desp2 + 1)), desp2 + 1);
        bwrite(nbfisico, buf_bloque);
        bytesescritos += desp2 + 1;
    }

    // ACTUALIZAR METAINFORMACIÓN INODO

    // Actualizar el tamaño en bytes lógico del fichero, solo si hemos escrito más allá del final del fichero
    if (offset >= inodo.tamEnBytesLog)
    {
        inodo.tamEnBytesLog = offset + nbytes;
    }
    // Actualizar mtime
    inodo.mtime = time(NULL);
    // Actualizar ctime
    inodo.ctime = time(NULL);
    // Salvar inodo
    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_write_f(): No se ha podido escribir el inodo %d \n" RESET, ninodo);
        return FALLO;
    }

    return bytesescritos;
}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    // LECTURA INODO
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_read_f(): No se ha podido leer el inodo %d \n" RESET, ninodo);
        return FALLO;
    }

    if ((inodo.permisos & 4) != 4)
    {
        fprintf(stderr, RED "No hay permisos de lectura\n" RESET);
        return FALLO;
    }

    // DECLARAR VARIABLES
    int primerBL, ultimoBL, desp1, desp2, nbfisico;
    int bytesleidos = 0;
    char buf_bloque[BLOCKSIZE];

    if (offset >= inodo.tamEnBytesLog)
    {
        bytesleidos = 0; // No podemos leer nada
        return bytesleidos;
    }
    if ((offset + nbytes) >= inodo.tamEnBytesLog)
    { // pretende leer más allá de EOF
        nbytes = inodo.tamEnBytesLog - offset;
        // leemos sólo los bytes que podemos desde el offset hasta EOF
    }

    // INICIALIZAR VARIABLES
    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;


    // CASO 1 BLOQUE
    if (primerBL == ultimoBL)
    {
        nbfisico = traducir_bloque_inodo(&inodo, primerBL, 0);
        if (nbfisico != FALLO)
        {
            bread(nbfisico, buf_bloque);
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }
        bytesleidos = desp2 - desp1 + 1;
    }
    else
    {
        int bl = primerBL + 1;

        // PRIMER BLOQUE LÓGICO
        nbfisico = traducir_bloque_inodo(&inodo, primerBL, 0);
        if (nbfisico != FALLO)
        {
            bread(nbfisico, buf_bloque);
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }
        bytesleidos += BLOCKSIZE - desp1;

        // BLOQUES INTERMEDIOS
        int contbl = 0;
        while (bl < ultimoBL)
        {
            int byte;
            nbfisico = traducir_bloque_inodo(&inodo, bl, 0);
            if (nbfisico != FALLO)
            {
                byte = bread(nbfisico, buf_bloque);
                memcpy(buf_original + desp1 + (BLOCKSIZE * (contbl)), buf_bloque, BLOCKSIZE);
            }
            else
            {
                byte = BLOCKSIZE;
            }
            bytesleidos += byte;
            bl++;
            contbl++;
        }

        // ÚLTIMO BLOQUE LÓGICO
        nbfisico = traducir_bloque_inodo(&inodo, ultimoBL, 0);
        if (nbfisico != FALLO)
        {
            bread(nbfisico, buf_bloque);
            memcpy(buf_original + (nbytes - (desp2 + 1)), buf_bloque, desp2 + 1);
        }
        bytesleidos += desp2 + 1;
    }
    // Actualizar atime
    inodo.atime = time(NULL);

    // Salvar inodo
    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_read_f(): No se ha podido escribir el inodo %d \n" RESET, ninodo);
        return FALLO;
    }
    return bytesleidos;
}

int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    // Declarar y leer el inodo correspondiente
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_stat_f(): No se ha podido leer el inodo %d \n" RESET, ninodo);
        return FALLO;
    }

    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;
    // falta guardar reservado_alineacion1
    // p_stat->reservado_alineacion1 = inodo.reservado_alineacion1;
    p_stat->atime = inodo.atime;
    p_stat->mtime = inodo.mtime;
    p_stat->ctime = inodo.ctime;
    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;

    return EXITO;
}

int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    // Declarar y leer el inodo correspondiente
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_chmod_f(): No se ha podido leer el inodo %d \n" RESET, ninodo);
        return FALLO;
    }

    // Actualizar permisos y ctime
    inodo.permisos = permisos;
    inodo.ctime = time(NULL);

    // Salvar inodo
    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_chmod_f(): No se ha podido escribir el inodo %d \n" RESET, ninodo);
        return FALLO;
    }

    return EXITO;
}
/**
 * Trunca un fichero/directorio (correspondiente al nº de inodo, ninodo, pasado como argumento) a los bytes
 * indicados como nbytes, liberando los bloques necesarios.
 */
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes)
{
    int primerBL;

    // LECTURA INODO
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_truncar_f(): No se ha podido leer el inodo %d \n" RESET, ninodo);
        return FALLO;
    }

    // PERMISOS ESCRPITURA
    if ((inodo.permisos & 2) != 2)
    {
        fprintf(stderr, RED "ERROR: mi_truncar_f(): No hay permisos de escritura\n" RESET);
        return FALLO;
    }

    // CALCULAR 1r BLOQUE
    if (nbytes % BLOCKSIZE == 0)
    {
        primerBL = nbytes / BLOCKSIZE;
    }
    else
    {
        primerBL = nbytes / BLOCKSIZE + 1;
    }

    int BL_lib = liberar_bloques_inodo(primerBL, &inodo);

    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados -= BL_lib;

    // Salvar inodo
    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_truncar_f(): No se ha podido escribir el inodo %d \n" RESET, ninodo);
        return FALLO;
    }
    return BL_lib;
}
