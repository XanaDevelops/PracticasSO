/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include "ficheros.h"
#include <sys/time.h>

#define TAMNOMBRE 60 // tamaño del nombre de directorio o fichero
#define ERROR_CAMINO_INCORRECTO -2
#define ERROR_PERMISO_LECTURA -3
#define ERROR_NO_EXISTE_ENTRADA_CONSULTA -4
#define ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO -5
#define ERROR_PERMISO_ESCRITURA -6
#define ERROR_ENTRADA_YA_EXISTENTE -7
#define ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO -8
#define TAMNOMBRE 60 //tamaño del nombre de directorio o fichero
#define PROFUNDIDAD 32 //profundidad máxima del árbol de directorios


// 0 -> NO CACHE
// 1 -> L/E
// 2 -> FIFO
// 3 -> LRU
#define USARCACHE 3
#if USARCACHE == 1
#define CACHE_SIZE 1
#else
#define CACHE_SIZE 3 // Tamaño máximo de la caché
#endif

struct entrada
{
    char nombre[TAMNOMBRE];
    unsigned int ninodo;
};

// Estructura para representar una entrada de la caché
struct UltimaEntrada{
   char camino[TAMNOMBRE*PROFUNDIDAD];
   int p_inodo;
   #if USARCACHE == 3
   struct timeval ultimaConsulta;
   #endif
};

/* NIVEL 7 */
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo);
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, 
                unsigned int *p_entrada, char reservar, unsigned char permisos);
void mostrar_error_buscar_entrada(int error);

/* NIVEL 8 */
int mi_creat(const char *camino, unsigned char permisos);
int mi_dir(const char *camino, char *buffer, char tipo, char flag);
int mi_chmod(const char *camino, unsigned char permisos);
int mi_stat(const char *camino, struct STAT *p_stat);

/* NIVEL 9 */
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes);
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes);

/* NIVEL 10 */
int mi_link(const char *camino1, const char *camino2);
int mi_unlink(const char *camino);

//EXTRA
int mi_cp(const char *origen, const char *destino, char tipoO, char tipoD);
int mi_cp_aux(const struct inodo iOrigen, const int p_iOrigen, const int p_iDestino);
int mi_cp_rec(const struct inodo inodo_ori, const int p_inodo_ori, const char *ruta_destino, const unsigned int posInodoR);

int mi_rn(const char *ruta_antigua, const char *nuevo_nombre, const char tipo);

int mi_unlink_r(const char *camino);
// auxiliar
int buscar_en_cache(const char *camino);
void actualizar_cache(const struct UltimaEntrada *nueva_entrada);
int auxiliarInodoEntradaDir(char *buffer, struct inodo inodo, struct entrada entrada, char tipo);