/*Ferriol Font, Josep
García Vázquez, Daniel
Perelló Perelló, Biel*/

#include "ficheros.h"

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
#define CACHE_SIZE 3 // Tamaño máximo de la caché

struct entrada
{
    char nombre[TAMNOMBRE];
    unsigned int ninodo;
};

// Estructura para representar una entrada de la caché
struct UltimaEntrada{
   char camino [TAMNOMBRE*PROFUNDIDAD];
   int p_inodo;
};

// Array de caché
static struct UltimaEntrada UltimasEntradas[CACHE_SIZE];

// Puntero de cola circular para gestionar la caché
static int puntero_cola = 0;


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

// auxiliar
int imprimir_stat(struct STAT *p_stat);
int buscar_en_cache(const char *camino);
void actualizar_cache(const char *camino, int p_inodo);

