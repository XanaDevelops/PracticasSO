#include <limits.h>
#include "ficheros_basico.h"

#define DEBUG2 1

int main(int argc, char **argv)
{
    if(access(*(argv+1), W_OK|R_OK)){
        fprintf(stderr, RED "ERROR: leer_sf(): no existe el archivo\n" RESET);
        return FALLO;
    }

    if (bmount(*(argv + 1)) == FALLO)
    {
        return FALLO;
    }

    struct superbloque SB;

    if (bread(posSB, &SB) == -1)
    {
        fprintf(stderr, RED "ERROR: escribir_bit(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }

    fprintf(stdout, "DATOS DEL SUPERBLOQUE\n\n");

    printf("posPrimerBloqueMB is: %d\n", SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB is: %d\n", SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI is: %d\n", SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI is: %d\n", SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos is: %d\n", SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos is: %d\n", SB.posUltimoBloqueDatos);
    printf("posInodoRaiz is: %d\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre is: %d\n", SB.posPrimerInodoLibre);
    printf("cantBloquesLibres is: %d\n", SB.cantBloquesLibres);
    printf("cantInodosLibres is: %d\n", SB.cantInodosLibres);
    printf("totBloques is: %d\n", SB.totBloques);
    printf("totInodos is: %d\n", SB.totInodos);

    printf("sizeof struct inodo is: %lu\n", sizeof(struct superbloque));
    printf("sizeof struct inodo is: %lu\n", sizeof(struct inodo));

    int contInodos = SB.posPrimerBloqueAI;

    int num = BLOCKSIZE / INODOSIZE;
    // Declarar un buffer
    struct inodo inodos[num];

    // Leer un bloque en el array de inodos
    if (bread(contInodos, inodos) == -1)
    {
        fprintf(stderr, RED "ERROR: initAI(): No se ha podido leer el bloque en el dispositivo\n" RESET);
        return FALLO;
    }
    /*   int cont = 0;
       struct inodo inodo = inodos[0];
       while (cont != SB.totInodos)
       {
           printf("inodo is %d: %d //   ", cont++, inodo.punterosDirectos[0]);
           int aux = inodo.punterosDirectos[0];
           contInodos = (aux / num) + SB.posPrimerBloqueAI;
           bread(contInodos, inodos);
           inodo = inodos[aux % num];
    }*/

    //parte nivel 3

    
    printf("\n");
    fprintf(stdout, "RESERVAR, y liberar, BLOQUE\n");

    int posBloque = reservar_bloque();

    if(posBloque==FALLO){
        fprintf(stderr, RED "ERROR: reservar_bloque(): no se ha podido reservar bloque\n" RESET);
        return FALLO;
    }
    if(posBloque!= SB.posPrimerBloqueDatos){
        fprintf(stderr, "ERROR: main(): posBloque != sb.SB.posPrimerBloqueDatos %d!=%d\n", posBloque, SB.posPrimerBloqueDatos);
        return FALLO;
    }

    if (bread(posSB, &SB) == -1)
    {
        fprintf(stderr, RED "ERROR: bread(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }

    fprintf(stdout, "Se ha reservado correctamente %d, que era el primer bloque libre.\n", posBloque);
    fprintf(stdout, "SB.cantBloquesLibres=%d\n", SB.cantBloquesLibres);
    fprintf(stdout, "liberando...\n");
    if(liberar_bloque(posBloque)==FALLO){
        fprintf(stderr, RED "ERROR: liberar_bloque()\n" RESET);
    }
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED "ERROR: bread(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }
    fprintf(stdout, "SB.cantBloquesLibres=%d\n", SB.cantBloquesLibres);

    fprintf(stdout, "MAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");

    fprintf(stdout, "posSB: %d -> leer_bit(%d) = %d\n", posSB, posSB, leer_bit(posSB));

    fprintf(stdout, "SB.posPrimerBloqueMB: %d -> leer_bit(%d) = %d\n", SB.posPrimerBloqueMB, SB.posPrimerBloqueMB, leer_bit(SB.posPrimerBloqueMB));
    fprintf(stdout, "SB.posUltimoBloqueMB: %d -> leer_bit(%d) = %d\n", SB.posUltimoBloqueMB, SB.posUltimoBloqueMB, leer_bit(SB.posUltimoBloqueMB));

    fprintf(stdout, "SB.posPrimerBloqueMB: %d -> leer_bit(%d) = %d\n", SB.posPrimerBloqueAI, SB.posPrimerBloqueAI, leer_bit(SB.posPrimerBloqueAI));
    fprintf(stdout, "SB.posUltimoBloqueMB: %d -> leer_bit(%d) = %d\n", SB.posUltimoBloqueAI, SB.posUltimoBloqueAI, leer_bit(SB.posUltimoBloqueAI));

    fprintf(stdout, "SB.posPrimerBloqueMB: %d -> leer_bit(%d) = %d\n", SB.posPrimerBloqueDatos, SB.posPrimerBloqueDatos, leer_bit(SB.posPrimerBloqueDatos));
    fprintf(stdout, "SB.posUltimoBloqueMB: %d -> leer_bit(%d) = %d\n", SB.posUltimoBloqueDatos, SB.posUltimoBloqueDatos, leer_bit(SB.posUltimoBloqueDatos));

    fprintf(stdout, "DATOS DEL DIRECTORIO RAÍZ\n");
    struct inodo raiz;
    leer_inodo(0, &raiz);

    return EXITO;
}
