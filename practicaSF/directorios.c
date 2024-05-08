#include "directorios.h"
#include <string.h>

#define DEBUG7A 0
#define DEBUG7B 1
#define DEBUG8 1

// Implemnetada mejora nivel 9
static struct UltimaEntrada UltimaEntradaIO[CACHE_SIZE];
static int pos_UltimaEntradaIO = 0;

// Se ha aplicado mejora nivell7 pagina 10 nota de pie 7

//***************************************BUSCAR ENTRADA Y AUXILIARES**************************************
/**
 * Busca una determinada entrada, dado un camino y el número de inodo del directorio padre
 * Obtiene:
 *  - El número de inodo al que está asociado el nombre de la entrada
 *  - El número de entrada dentro del inodo padre que lo contiene
 *
 * return: ÉXITO o FALLO
 */
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo,
                   unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    // Declarar variables
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    memset(&inicial, '\0', sizeof(entrada.nombre));
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo, bytesleidos, trobat;
    int numInodo;

    // LECTURA SUPERBLOQUE
    struct superbloque sb;
    if (bread(posSB, &sb) == FALLO)
    {
        fprintf(stderr, RED "ERROR: buscar_entrada(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }

    // Comprobar si es el directorio raíz
    if (strcmp(camino_parcial, "/") == 0)
    {
        *p_inodo = sb.posInodoRaiz;
        *p_entrada = 0;
        return 0;
    }

    if (extraer_camino(camino_parcial, inicial, final, &tipo) == FALLO)
    {
        return ERROR_CAMINO_INCORRECTO;
    }
#if DEBUG7B
    fprintf(stderr, GRAY "[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n" RESET, inicial, final, reservar);
#endif

    // Buscar la entrada cuyo nombre se encuentra en inicial
    if (leer_inodo(*p_inodo_dir, &inodo_dir) == FALLO)
    {
        fprintf(stderr, RED "ERROR: buscar_entrada(): No se ha podido leer el inodo %d\n" RESET, *p_inodo_dir);
        return FALLO;
    }

    // Comprobar si el inodo tiene permisos de lectura
    if ((inodo_dir.permisos & 4) != 4)
    {
        return ERROR_PERMISO_LECTURA;
    }

    struct entrada buff_entradas[BLOCKSIZE / sizeof(struct entrada)];
    memset(buff_entradas, '\0', sizeof(buff_entradas));
    memset(&entrada, '\0', sizeof(struct entrada));

    // CALCULAR ENTRADES INODO
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    fprintf(stderr, RED "cant_entradas_inodo: %d\n" RESET, cant_entradas_inodo);

    // Número de entrada inicial
    num_entrada_inodo = 0;

    bytesleidos = 0;

    trobat = 0;
    if (cant_entradas_inodo > 0)
    {
        while ((num_entrada_inodo < cant_entradas_inodo) && !trobat)
        {
            bytesleidos += mi_read_f(*p_inodo_dir, buff_entradas, bytesleidos, BLOCKSIZE);

            for (int i = 0; (i < (BLOCKSIZE / sizeof(struct entrada))) && !trobat && (num_entrada_inodo < cant_entradas_inodo); i++)
            {
                num_entrada_inodo++;
                if (strcmp(inicial, buff_entradas[i].nombre) == 0)
                {
                    trobat = 1;
                    memcpy(&entrada, &buff_entradas[i], sizeof(struct entrada));
                    num_entrada_inodo--;
                }

                fprintf(stderr, RED "num_ %d  --- cant_ %d\n" RESET, num_entrada_inodo, cant_entradas_inodo);
            }
        }
    }
    fprintf(stderr, RED "strcmp(inicial, entrada.nombre)= %d\n" RESET, strcmp(inicial, entrada.nombre));
    // Comprobar si la entrada existe

    if ((strcmp(inicial, entrada.nombre) != 0) && (num_entrada_inodo == cant_entradas_inodo))
    {
        switch (reservar)
        {
        case 0:
            // Modo consulta, como no existe se devuelve error
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
            break;
        case 1:
            // Modo escritura
            // Crear entrada en el directorio referenciado por *p_inodo_dir
            // Si es fichero no se permite la escritura
            if (inodo_dir.tipo == 'f')
            {
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }

            // Si es directorio comprobar que tiene permisos de escritura
            if ((inodo_dir.permisos & 2) != 2)
            {
                return ERROR_PERMISO_ESCRITURA;
            }
            else
            {
                // COPIAR *inicial EN EL NOMBRE DE LA ENTRADA
                memset(&entrada, '\0', sizeof(struct entrada));
                memcpy(entrada.nombre, inicial, sizeof(struct entrada));

                if (tipo == 'd')
                {
                    if (strcmp(final, "/") == 0)
                    {
                        // Reservar inodo como directorio y asignarlo a la entrada
                        numInodo = reservar_inodo('d', permisos);
                        entrada.ninodo = numInodo;
                    }
                    else
                    {
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                }
                else
                {
                    if (strcmp(final, "") == 0)
                    {
                        // Reservar inodo como fichero y asignarlo a la entrada
                        numInodo = reservar_inodo('f', permisos);
                        entrada.ninodo = numInodo;
                    }
                    else
                    {
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                }
#if DEBUG7B
                fprintf(stderr, GRAY "[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n" RESET, numInodo, tipo, permisos, inicial);
#endif
                int bytesescitos = mi_write_f(*p_inodo_dir, &entrada, inodo_dir.tamEnBytesLog, sizeof(struct entrada));
                if (bytesescitos == FALLO)
                {
                    if (entrada.ninodo != -1)
                    {
                        liberar_inodo(entrada.ninodo);
                    }
                    return FALLO;
                }
#if DEBUG7B
                fprintf(stderr, GRAY "[buscar_entrada()→ creada entrada: %s, %d]\n" RESET, entrada.nombre, entrada.ninodo);
#endif
            }
            break;
        }
    } // Comprobar si se ha llegado al final del camino
    if (strcmp(final, "") == 0 || strcmp(final, "/") == 0)
    {
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1))
        {
            // Modo escritura y entrada ya existente
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        if (num_entrada_inodo == cant_entradas_inodo)
        {
            *p_inodo = numInodo;
        }
        else
        {
            *p_inodo = entrada.ninodo;
        }
        *p_entrada = num_entrada_inodo;

        return EXITO;
    }
    else
    {
        // Asignar a *p_inodo_dir el puntero al inodo que se indica en la entrada encontrada 
        *p_inodo_dir = entrada.ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }

    return EXITO;
}

/**
 * Separa camino en inicio, final, configura tipo
 * Asume tamaño inicial y final igual a camino, problemas si no...
 *
 * return: ÉXITO o FALLO
 */
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{
#if DEBUG7A
    fprintf(stderr, GRAY "[extraer_camino() -> extrayendo %s]\n" RESET, camino);
#endif
    // comprobar que empieza por '/'
    if (*camino != '/')
    {
        return FALLO;
    }

    int len = strlen(camino);
    if (len == 1)
    { // caso "/"
        *tipo = 'f';
        *inicial = '\0';
        *final = '\0';
#if DEBUG7A
        fprintf(stderr, GRAY "[extraer_camino() -> inicial: |%s|, final: |%s| tipo %s]\n" RESET, inicial, final, tipo);
#endif
        return EXITO;
    }
    // dividir siguiente '/'
    char *pos = strchr(camino + 1, '/');
#if DEBUG7A
    fprintf(stderr, GRAY "[extraer_camino() -> pos: %s]\n" RESET, pos);
#endif
    if (pos != NULL)
    {
        strncpy(inicial, camino + 1, pos - camino);
        strcpy(final, pos);
    }
    else
    {
        strncpy(inicial, camino + 1, len - 1);
        *final = '\0';
    }

    pos = strchr(inicial, '/');
    if (pos == NULL) // es fichero
    {
        *tipo = 'f';
    }
    else // es directorio
    {
        *tipo = 'd';
        *pos = '\0';
    }
#if DEBUG7A
    fprintf(stderr, GRAY "[extraer_camino -> inicial: |%s|, final: |%s| tipo %s]\n" RESET, inicial, final, tipo);
#endif

    return EXITO;
}

/**
 * Muestra el texto asociado a un error de buscar entrada
 */
void mostrar_error_buscar_entrada(int error)
{
    // fprintf(stderr, "Error: %d\n", error);
    switch (error)
    {
    case -2:
        fprintf(stderr, RED "Error: Camino incorrecto.\n" RESET);
        break;
    case -3:
        fprintf(stderr, RED "Error: Permiso denegado de lectura.\n" RESET);
        break;
    case -4:
        fprintf(stderr, RED "Error: No existe el archivo o el directorio.\n" RESET);
        break;
    case -5:
        fprintf(stderr, RED "Error: No existe algún directorio intermedio.\n" RESET);
        break;
    case -6:
        fprintf(stderr, RED "Error: Permiso denegado de escritura.\n" RESET);
        break;
    case -7:
        fprintf(stderr, RED "Error: El archivo ya existe.\n" RESET);
        break;
    case -8:
        fprintf(stderr, RED "Error: No es un directorio.\n" RESET);
        break;
    }
}
//***************************************OTRAS FUNCIONES***********************************************

//******************************Creación de ficheros y directorios*************************************
/**
 * Crea un fichero/directorio y su entrada de directorio
 *
 * return: ÉXITO o FALLO
 */
int mi_creat(const char *camino, unsigned char permisos)
{
    // LECTURA SUPERBLOQUE
    struct superbloque sb;
    if (bread(posSB, &sb) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_creat(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }

    unsigned int p_inodo = 0, p_entrada = 0;
    int return_buscar_entrada;

    printf("camino: %s\n", camino);
    return_buscar_entrada = buscar_entrada(camino, &sb.posInodoRaiz, &p_inodo, &p_entrada, 1, permisos);

    printf("return_buscar_entrada: %d\n", return_buscar_entrada);
    printf("p_inodo: %d\n", p_inodo);
    printf("p_entrada: %d\n", p_entrada);

    if (return_buscar_entrada != EXITO)
    {
        // Notificar error devuelto por buscar_entrada()
        mostrar_error_buscar_entrada(return_buscar_entrada);
        // Devolver FALLO
        return FALLO;
    }

    return EXITO;
}

//******************************Listado del contenido de un directorio*********************************

/**
 * PLACEHOLDER mi_dir()
 * return: EXITO o FALLO
 */
int mi_dir(const char *camino, char *buffer, char tipo, char flag)
{
    struct superbloque sb;
    if (bread(posSB, &sb) == FALLO)
    {
        fprintf(stderr, RED "ERROR mi_dir() -> No se ha podido leer el SB\n" RESET);
        return FALLO;
    }
    int be_ret;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    be_ret = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 7); // permisos lectura
    printf("be_ret %d \n",be_ret);
    if (be_ret != EXITO)
    {
        if (be_ret != FALLO)
        {
            mostrar_error_buscar_entrada(be_ret);
        }
        else
        {
            fprintf(stderr, RED "ERROR mi_dir() -> error en buscar entrada\n" RESET);
        }
        return FALLO;
    }
#if DEBUG8
    fprintf(stderr, GRAY "[mi_dir() -> resultado buscar_entrada() p_inodo_dir:%d, p_inodo:%d, p_entrada:%d]\n" RESET, p_inodo_dir, p_inodo, p_entrada);
#endif
    struct inodo inodo;
    if (leer_inodo(p_inodo, &inodo) == FALLO)
    {
        return FALLO;
    }

    struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
    if (mi_read_f(p_inodo, entradas, p_entrada, BLOCKSIZE) == FALLO)
    {
        fprintf(stderr, RED "ERROR mi_dir() -> fallor mi_read_f\n");
        return FALLO;
    }
    // PROVISIONAL (chungo si se sale del buffer)
    int entradas_inodo = inodo.tamEnBytesLog / sizeof(struct entrada);
    for (int i = 0; i < entradas_inodo; i++)
    {
        printf("%s\n", entradas[i].nombre);
    }

    return EXITO;
}

//******************************Cambio de permisos de un fichero o directorio**************************
/**
 * Cambia los permisos de un fichero o directorio
 *
 * return: ÉXITO o FALLO
 */
int mi_chmod(const char *camino, unsigned char permisos)
{
    // LECTURA SUPERBLOQUE
    struct superbloque sb;
    if (bread(posSB, &sb) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_chmod(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }

    unsigned int p_inodo = 0, p_entrada = 0;
    int return_buscar_entrada;

    return_buscar_entrada = buscar_entrada(camino, &sb.posInodoRaiz, &p_inodo, &p_entrada, 0, 4);

    if (return_buscar_entrada != EXITO)
    {
        // Notificar error devuelto por buscar_entrada()
        mostrar_error_buscar_entrada(return_buscar_entrada);
        // Devolver FALLO
        return FALLO;
    }

    // Llamada a mi_chmod_f de la capa de ficheros, pasandole el p_inodo
    mi_chmod_f(p_inodo, permisos);

    return EXITO;
}

//******************************Visualización metadatos del inodo**************************************
/**
 * Muestra la información acerca del inodo de un fichero o directorio
 *
 * return: ÉXITO o FALLO
 */
int mi_stat(const char *camino, struct STAT *p_stat)
{
    // LECTURA SUPERBLOQUE
    struct superbloque sb;
    if (bread(posSB, &sb) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_stat(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }

    unsigned int p_inodo = 0, p_entrada = 0;
    int return_buscar_entrada;

    return_buscar_entrada = buscar_entrada(camino, &sb.posInodoRaiz, &p_inodo, &p_entrada, 0, 4);

    if (return_buscar_entrada != EXITO)
    {
        // Notificar error devuelto por buscar_entrada()
        mostrar_error_buscar_entrada(return_buscar_entrada);
        // Devolver FALLO
        return FALLO;
    }

    // Llamada a mi_stat_f de la capa de ficheros, pasandole el p_inodo
    mi_stat_f(p_inodo, p_stat);

    // Mostrar el número de inodo junto con su información de estado
    fprintf(stdout, "Nº de inodo: %d \n", p_inodo);
    imprimir_stat(p_stat);

    return EXITO;
}

// AUXILIAR
/**
 * imprime todos los parametros de struct STAT
 *
 * return: EXITO o FALLO
 */
int imprimir_stat(struct STAT *p_stat)
{
    fprintf(stdout, "tipo: %c\n", p_stat->tipo);
    fprintf(stdout, "permisos: %d\n", p_stat->permisos);
    fprintf(stdout, "atime: %s", ctime(&p_stat->atime));
    fprintf(stdout, "ctime: %s", ctime(&p_stat->ctime));
    fprintf(stdout, "mtime: %s", ctime(&p_stat->mtime));
    fprintf(stdout, "nlinks: %d\n", p_stat->nlinks);
    fprintf(stdout, "tamEnBytesLog: %d\n", p_stat->tamEnBytesLog);
    fprintf(stdout, "numBloquesOcupados: %d\n", p_stat->numBloquesOcupados);

    return EXITO;
}

//************************* Escritura en un offset de un fichero***************************************
/**
 * Escribe contenido en un fichero indicado por el camino
 *
 * return: Deuvelve los bytes escritos
 */
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
{
    // LECTURA SUPERBLOQUE
    struct superbloque sb;
    if (bread(posSB, &sb) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_write(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }

    unsigned int p_inodo = 0, p_entrada = 0;
    int return_buscar_entrada, bytesEscritos;
    int pos = buscar_en_cache(camino);

    if (pos != -1)
    {
        p_inodo = UltimaEntradaIO[pos].p_inodo;
        return_buscar_entrada = EXITO;
    }
    else
    {
        return_buscar_entrada = buscar_entrada(camino, &sb.posInodoRaiz, &p_inodo, &p_entrada, 0, 4);
        struct UltimaEntrada aux;
        strncpy(aux.camino, camino, sizeof(aux.camino) - 1);
        aux.camino[sizeof(aux.camino) - 1] = '\0';

        aux.p_inodo = p_inodo;
        actualizar_cache(&aux);
    }

    if (return_buscar_entrada != EXITO)
    {
        // Notificar error devuelto por buscar_entrada()
        mostrar_error_buscar_entrada(return_buscar_entrada);
        // Devolver FALLO
        return FALLO;
    }

    bytesEscritos = mi_write_f(p_inodo, buf, offset, nbytes);

    return bytesEscritos;
}

//************************* Lectura secuencial de todo el contenido de un fichero**********************
/**
 * Lee los nbytes del fichero indicado por el camino
 *
 * return: Devuelve los bytes leídos
 */
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes)
{
    // LECTURA SUPERBLOQUE
    struct superbloque sb;
    if (bread(posSB, &sb) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_read(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }

    unsigned int p_inodo = 0, p_entrada = 0;
    int return_buscar_entrada, bytesLeidos;
    int pos = buscar_en_cache(camino);

    if (pos != -1)
    {
        p_inodo = UltimaEntradaIO[pos].p_inodo;
        return_buscar_entrada = EXITO;
    }
    else
    {
        return_buscar_entrada = buscar_entrada(camino, &sb.posInodoRaiz, &p_inodo, &p_entrada, 0, 4);
        struct UltimaEntrada aux;
        strncpy(aux.camino, camino, sizeof(aux.camino) - 1);
        aux.camino[sizeof(aux.camino) - 1] = '\0';

        aux.p_inodo = p_inodo;
        actualizar_cache(&aux);
    }

    if (return_buscar_entrada != EXITO)
    {
        // Notificar error devuelto por buscar_entrada()
        mostrar_error_buscar_entrada(return_buscar_entrada);
        // Devolver FALLO
        return FALLO;
    }

    bytesLeidos = mi_read_f(p_inodo, buf, offset, nbytes);

    return bytesLeidos;
}

//************************************* MILLORA NIVELL 9***********************************************
/**
 * Busca si el camino pasado por parámetro esta almacenado en la caché
 *
 * return: número de inodo si se ha encontrado la entrada o -1
 */
int buscar_en_cache(const char *camino)
{
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        if (strcmp(UltimaEntradaIO[i].camino, camino) == 0)
        {
            return i;
        }
    }
    // Indica que el camino no se encontró en la caché
    return -1;
}

/**
 * Actualiza la caché con un camino y número de inodo pasado por parámetro
 */
void actualizar_cache(const struct UltimaEntrada *nueva_entrada)
{
    // Si el camino ya está en la caché, no es necesario actualizar
    if (buscar_en_cache(nueva_entrada->camino) != -1)
    {
        return;
    }

    // Si no se encuentra el camino en la caché, actualizar la entrada en la posición del puntero de cola circular
    strcpy(UltimaEntradaIO[pos_UltimaEntradaIO].camino, nueva_entrada->camino);
    UltimaEntradaIO[pos_UltimaEntradaIO].p_inodo = nueva_entrada->p_inodo;

    // Avanzar el puntero de cola circular
    pos_UltimaEntradaIO = (pos_UltimaEntradaIO + 1) % CACHE_SIZE;
}
