#include "directorios.h"
#include <string.h>

// Implementada mejora nivel 9
#if USARCACHE == 2 || USARCACHE == 3
static struct UltimaEntrada UltimaEntradaIO[CACHE_SIZE];
#if USARCACHE == 2
static int pos_UltimaEntradaIO = 0;
#endif
#endif
#define DEBUGMIDIR 0
#define COLORD BLUE
#define COLORF GREEN

#define ENTRADASBLOQUE (BLOCKSIZE / sizeof(struct entrada))

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
            }
        }
    }
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
                memcpy(entrada.nombre, inicial, sizeof(entrada.nombre));

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
    mi_waitSem();
    // LECTURA SUPERBLOQUE
    struct superbloque sb;
    if (bread(posSB, &sb) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_creat(): No se ha podido leer SB\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    unsigned int p_inodo = 0, p_entrada = 0;
    int return_buscar_entrada;

    return_buscar_entrada = buscar_entrada(camino, &sb.posInodoRaiz, &p_inodo, &p_entrada, 1, permisos);

    if (return_buscar_entrada != EXITO)
    {
        // Notificar error devuelto por buscar_entrada()
        mostrar_error_buscar_entrada(return_buscar_entrada);
        // Devolver FALLO
        mi_signalSem();
        return FALLO;
    }

    mi_signalSem();
    return EXITO;
}

//******************************Listado del contenido de un directorio*********************************

/**
 * mi_dir()
 * return: nº de entradas o FALLO
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
#if DEBUG8 || DEBUGMIDIR
    fprintf(stderr, GRAY "[mi_dir() -> resultado buscar_entrada() p_inodo_dir:%d, p_inodo:%d, p_entrada:%d]\n" RESET, p_inodo_dir, p_inodo, p_entrada);
#endif
    struct inodo inodo;
    if (leer_inodo(p_inodo, &inodo) == FALLO)
    {
        return FALLO;
    }
    int entradas_inodo = inodo.tamEnBytesLog / sizeof(struct entrada);

    struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
    // struct entrada entradas[entradas_inodo * sizeof(struct entrada)];

    // PROVISIONAL (chungo si se sale del buffer)

    struct inodo inodoEntrada;
    int nEntradas = 0;
    if (tipo == 'd')
    {
        int contadorLinea = 0;
        int nBloques = 0;
        while (nEntradas < entradas_inodo)
        {
            // mirar si esto esta bien
            if (mi_read_f(p_inodo, entradas, nBloques * BLOCKSIZE, sizeof(entradas)) == FALLO)
            {
                fprintf(stderr, RED "ERROR mi_dir() -> fallor mi_read_f\n");
                return FALLO;
            }
            for (int i = 0; nEntradas < entradas_inodo && i < ENTRADASBLOQUE; i++)
            {
#if DEBUG8 || DEBUGMIDIR
                fprintf(stderr, GRAY "[mi_dir() -> entrada.nombre: %s]\n" RESET, entradas[i].nombre);
                fprintf(stderr, GRAY "[mi_dir() -> entrada.ninodo: %u]\n" RESET, entradas[i].ninodo);
#endif
                // mirar optimizar
                leer_inodo(entradas[i].ninodo, &inodoEntrada);
#if DEBUG8 || DEBUGMIDIR
                fprintf(stderr, GRAY "tipo inoodo %c\n" RESET, inodoEntrada.tipo);
#endif
                if (inodoEntrada.tipo == 'l')
                {
                    continue;
                }
                if (flag == 0)
                {
                    if (inodoEntrada.tipo == 'd')
                    {
                        strcat(buffer, COLORD);
                    }
                    else
                    {
                        strcat(buffer, COLORF);
                    }
                    strcat(buffer, entradas[i].nombre);
                    strcat(buffer, RESET);
                    contadorLinea += strlen(entradas[i].nombre);
                    if (contadorLinea > 80)
                    {
                        strcat(buffer, "\n");
                        contadorLinea = 0;
                    }
                    else
                    {
                        strcat(buffer, "\t");
                    }
                }
                else
                { // flag == 1
                    auxiliarInodoEntradaDir(buffer, inodoEntrada, entradas[i], tipo);
                }
                // printf("buf: %s\n", buffer);
                nEntradas++;
            }
            nBloques++;
        }
    }
    else
    {
        auxiliarInodoEntradaDir(buffer, inodo, entradas[0], tipo);
        // obtener el nombre
        char aux[strlen(camino)];
        strcpy(aux, camino);
        char *last = NULL;
        char *token = strtok(aux, "/");
        while (token != NULL)
        {
            last = token;
            token = strtok(NULL, "/");
        }
        if (inodo.tipo == 'f')
            strcat(buffer, COLORF);
        else
            strcat(buffer, COLORD);
        strcat(buffer, last);
        strcat(buffer, RESET);
        strcat(buffer, "\n");
    }
    return nEntradas;
}

int auxiliarInodoEntradaDir(char *buffer, struct inodo inodo, struct entrada entrada, char tipo)
{
    char tmp[64];
    memset(tmp, '\0', sizeof(tmp));
    *tmp = inodo.tipo;
    strcat(buffer, tmp);
    strcat(buffer, "\t");
    if (inodo.permisos & 4)
        strcat(buffer, "r");
    else
        strcat(buffer, "-");
    if (inodo.permisos & 2)
        strcat(buffer, "w");
    else
        strcat(buffer, "-");
    if (inodo.permisos & 1)
        strcat(buffer, "x");
    else
        strcat(buffer, "-");

    strcat(buffer, "\t");

    struct tm *tm; // ver info: struct tm
    tm = localtime(&inodo.mtime);

    sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    strcat(buffer, tmp);

    strcat(buffer, "\t");

    sprintf(tmp, "%d", inodo.tamEnBytesLog);
    strcat(buffer, tmp);

    strcat(buffer, "\t\t");

    if (tipo == 'd')
    {
        if (inodo.tipo == 'f')
            strcat(buffer, COLORF);
        else
            strcat(buffer, COLORD);
        strcat(buffer, entrada.nombre);
        strcat(buffer, RESET);
        strcat(buffer, "\n");
    }
    return EXITO;
};

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

#ifdef DEBUG8
    // Mostrar el número de inodo
    fprintf(stdout, BLUE "Nº de inodo: %d \n" RESET, p_inodo);
#endif

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
#if DEBUG9
        fprintf(stderr, ORANGE "\n[mi_write() -> Utilizamos la caché de lectura en vez de llamar a buscar_entrada()]\n" RESET);
#endif
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
#if DEBUG9
        fprintf(stderr, ORANGE "[mi_write() -> Actualizamos la caché de escritura]\n" RESET);
#endif
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
#if DEBUG9
        fprintf(stderr, ORANGE "\n[mi_read() -> Utilizamos la caché de lectura en vez de llamar a buscar_entrada()]\n" RESET);
#endif
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
#if DEBUG9
        fprintf(stderr, ORANGE "[mi_read() -> Actualizamos la caché de lectura]\n" RESET);
#endif
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
//************************* **********************
/**
 * Crea el enlace de una entrada de directorio camino2 al inodo especificado por otra entrada de directorio camino1 .
 * return: ÉXITO o FALLO
 */
int mi_link(const char *camino1, const char *camino2)
{
    mi_waitSem();
    int longitud_ruta1 = strlen(camino1);
    int longitud_ruta2 = strlen(camino2);

    // Comprobar si las dos rutas pasadas por parámetro se corresponden a ficheros
    if ((*(camino1 + longitud_ruta1 - 1) == '/') || (*(camino2 + longitud_ruta2 - 1) == '/'))
    {
        fprintf(stderr, RED "ERROR: mi_link(): Las rutas especificadas no se corresponden a un fichero\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    // LECTURA SUPERBLOQUE
    struct superbloque sb;
    if (bread(posSB, &sb) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_link(): No se ha podido leer SB\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    unsigned int p_inodo_dir1 = sb.posInodoRaiz;
    unsigned int p_inodo1 = 0, p_entrada1 = 0;
    int return_buscar_entrada1;

    return_buscar_entrada1 = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 6);

    if (return_buscar_entrada1 != EXITO)
    {
        // Notificar error devuelto por buscar_entrada()
        mostrar_error_buscar_entrada(return_buscar_entrada1);
        mi_signalSem();
        // Devolver FALLO
        return FALLO;
    }

    unsigned int p_inodo_dir2 = sb.posInodoRaiz;
    unsigned int p_inodo2 = 0, p_entrada2 = 0;
    int return_buscar_entrada2;

    return_buscar_entrada2 = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6);

    if (return_buscar_entrada2 != EXITO)
    {
        // Notificar error devuelto por buscar_entrada()
        mostrar_error_buscar_entrada(return_buscar_entrada2);
        mi_signalSem();
        // Devolver FALLO
        return FALLO;
    }

    // Leer la entrada creada correspondiente a camino2
    int num_bloque = p_inodo_dir2 / (BLOCKSIZE / sizeof(struct entrada));
    int entrada_buffer = p_entrada2 % (BLOCKSIZE / sizeof(struct entrada));
    struct entrada entrada;
    struct entrada buff_entradas[BLOCKSIZE / sizeof(struct entrada)];

    memset(buff_entradas, '\0', sizeof(buff_entradas));
    memset(&entrada, '\0', sizeof(struct entrada));

    mi_read_f(p_inodo_dir2, buff_entradas, num_bloque * BLOCKSIZE, BLOCKSIZE);

    memcpy(&entrada, &buff_entradas[entrada_buffer], sizeof(struct entrada));

    int tam_entrada = sizeof(struct entrada);
    int offset = p_entrada2 * tam_entrada;

    mi_read_f(p_inodo_dir2, &entrada, offset, tam_entrada);

    // Asociar a esta entrada el mismo inodo que el asociado a la entrada del camino1
    entrada.ninodo = p_inodo1;

    // Escribir la entrada modificada en p_inodo_dir_2
    memcpy(&buff_entradas[entrada_buffer], &entrada, sizeof(struct entrada));

    mi_write_f(p_inodo_dir2, buff_entradas, offset, tam_entrada);

    // Liberar el inodo que se ha asociado a la entrada creada, p_inodo2
    liberar_inodo(p_inodo2);

    // Incrementar la cantidad de enlaces (nlinks) de p_inodo1 y actualizar ctime
    struct inodo inodo_enlace;
    if (leer_inodo(p_inodo1, &inodo_enlace) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_link(): No se pudo leer el inodo\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    // Actualizar el número de enlaces del inodo
    inodo_enlace.nlinks++;
    // Actualizar el tiempo de cambio a la hora actual
    inodo_enlace.ctime = time(NULL);

    // Salvar el inodo
    if (escribir_inodo(p_inodo1, &inodo_enlace) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_link(): No se pudo escribir el inodo\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    mi_signalSem();
    // Devolver ÉXITO
    return EXITO;
}
//**************************** Borrado de enlaces, ficheros y directorios *****************************
/**
 *
 * return: ÉXITO o FALLO
 * */
int mi_unlink(const char *camino)
{
    mi_waitSem();
    // LECTURA SUPERBLOQUE
    struct superbloque sb;
    if (bread(posSB, &sb) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_unlink(): No se ha podido leer SB\n" RESET);
        mi_signalSem();
        return FALLO;
    }
    unsigned int p_inodo_dir = sb.posInodoRaiz;
    unsigned int p_inodo = 0, p_entrada = 0;
    int rt_be;

    rt_be = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
    if (rt_be != EXITO)
    {
        // Notificar error devuelto por buscar_entrada()
        mostrar_error_buscar_entrada(rt_be);
        mi_signalSem();
        // Devolver FALLO
        return FALLO;
    }
    int num_bloque = p_inodo_dir / (BLOCKSIZE / sizeof(struct entrada));
    int entrada_buffer = p_entrada % (BLOCKSIZE / sizeof(struct entrada));
    struct entrada buff_entradas[BLOCKSIZE / sizeof(struct entrada)];
    memset(buff_entradas, '\0', sizeof(buff_entradas));

    struct inodo inodo_eliminar;
    struct inodo inodo_padre;
    int num_eliminar = p_inodo;
    leer_inodo(num_eliminar, &inodo_eliminar);

    if (inodo_eliminar.tipo == 'd' && inodo_eliminar.tamEnBytesLog > 0)
    {
        fprintf(stderr, RED "ERROR: mi_unlink(): El directorio %s no está vacio\n" RESET, camino);
        mi_signalSem();
        return FALLO;
    }

    else
    {
        leer_inodo(p_inodo_dir, &inodo_padre);
        int num_e = inodo_padre.tamEnBytesLog / sizeof(struct entrada);
        if (p_entrada != num_e - 1)
        {
            memset(buff_entradas, '\0', sizeof(buff_entradas));
            mi_read_f(p_inodo_dir, buff_entradas, num_bloque * BLOCKSIZE, BLOCKSIZE);
            // memcpy(&buff_entradas[entrada_buffer], &buff_entradas[num_e - 1], sizeof(struct entrada));
            // asumiendo continuidad, creo que esta mal
            // fprintf(stderr, "%d\n", num_e%ENTRADASBLOQUE); si esto se tenia que mostrar descomentar...
            if ((num_e % ENTRADASBLOQUE) == num_bloque)
            {
                // mi_write_f(p_inodo_dir, &buff_entradas[num_e - 1], entrada_buffer * sizeof(struct entrada), sizeof(struct entrada));
                memcpy(&buff_entradas[entrada_buffer], &buff_entradas[num_e - 1], sizeof(struct entrada));
            }
            else
            {
                struct entrada lastEntradas[BLOCKSIZE / sizeof(struct entrada)];
                mi_read_f(p_inodo_dir, lastEntradas, ((num_e - 1) / ENTRADASBLOQUE) * BLOCKSIZE, BLOCKSIZE);
                // mi_write_f(p_inodo_dir, &buff_entradas[num_e - 1], entrada_buffer * sizeof(struct entrada), sizeof(struct entrada));
                memcpy(&buff_entradas[entrada_buffer], &lastEntradas[(num_e) % ENTRADASBLOQUE - 1], sizeof(struct entrada));
            }
            mi_write_f(p_inodo_dir, buff_entradas, num_bloque * BLOCKSIZE, BLOCKSIZE);
        }
        mi_truncar_f(p_inodo_dir, inodo_padre.tamEnBytesLog - sizeof(struct entrada));
    }
    inodo_eliminar.nlinks--;
    if (inodo_eliminar.nlinks == 0)
    {
        liberar_inodo(num_eliminar);
    }
    else
    {
        // Actualizar ctime
        inodo_eliminar.ctime = time(NULL);
        // Salvar inodo
        if (escribir_inodo(num_eliminar, &inodo_eliminar) == FALLO)
        {
            fprintf(stderr, RED "ERROR: mi_unlink(): No se ha podido escribir el inodo %d \n" RESET, num_eliminar);
            mi_signalSem();
            return FALLO;
        }
    }

    mi_signalSem();
    return EXITO;
}
//************************************* MILLORA NIVELL 9***********************************************
/**
 * Busca si el camino pasado por parámetro esta almacenado en la caché
 *
 * return: número de inodo si se ha encontrado la entrada o -1
 */
int buscar_en_cache(const char *camino)
{
#if USARCACHE == 0
    return FALLO;
#endif
#if USARCACHE > 0
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        if (strcmp(UltimaEntradaIO[i].camino, camino) == 0)
        {
#if USARCACHE == 3
            gettimeofday(&UltimaEntradaIO[i].ultimaConsulta, NULL);
#endif
#if DEBUG9
            fprintf(stderr, BLUE "mi_write() -> usar cache[%d] con %s\n" RESET, i, camino);
#endif
            return i;
        }
    }
    // Indica que el camino no se encontró en la caché
    return -1;
#endif
}

/**
 * Actualiza la caché con un camino y número de inodo pasado por parámetro
 */
void actualizar_cache(const struct UltimaEntrada *nueva_entrada)
{
#if USARCACHE == 2 // FIFO Circular
    // Si el camino ya está en la caché, no es necesario actualizar
    if (buscar_en_cache(nueva_entrada->camino) != -1)
    {
        return;
    }

    // Si no se encuentra el camino en la caché, actualizar la entrada en la posición del puntero de cola circular

    strcpy(UltimaEntradaIO[pos_UltimaEntradaIO].camino, nueva_entrada->camino);
    UltimaEntradaIO[pos_UltimaEntradaIO].p_inodo = nueva_entrada->p_inodo;
#if DEBUG9
    fprintf(stderr, ORANGE "mi_write() -> actulizar cache[%d] con %s\n" RESET, pos_UltimaEntradaIO, nueva_entrada->camino);
#endif
    // Avanzar el puntero de cola circular
    pos_UltimaEntradaIO = (pos_UltimaEntradaIO + 1) % CACHE_SIZE;

#endif
#if USARCACHE == 3 // LRU
    if (buscar_en_cache(nueva_entrada->camino) != -1)
    {
        return;
    }
    int i = 0;
    struct timeval antiguo = UltimaEntradaIO[i].ultimaConsulta;
    for (int j = 0; j < CACHE_SIZE; j++)
    {
        struct timeval check = UltimaEntradaIO[j].ultimaConsulta;
        if ((check.tv_sec + check.tv_usec) < (antiguo.tv_sec + antiguo.tv_usec))
        {
            i = j;
            antiguo = UltimaEntradaIO[j].ultimaConsulta;
        }
    }
    memcpy(&UltimaEntradaIO[i], nueva_entrada, sizeof(struct UltimaEntrada));
    gettimeofday(&UltimaEntradaIO[i].ultimaConsulta, NULL);
#if DEBUG9
#if USARCACHE == 2
    fprintf(stderr, ORANGE "mi_write() -> actulizar cache[%d] con %s\n" RESET, pos_UltimaEntradaIO, nueva_entrada->camino);
#endif
#if USARCACHE == 3
    fprintf(stderr, ORANGE "mi_write() -> actulizar cache[%d] con %s\n" RESET, i, nueva_entrada->camino);
#endif
#endif

#endif
    return;
}

//********************************* Extra **********************

/**
 * mi_cp()
 * return: EXITO o FALLO
 */
int mi_cp(const char *origen, const char *destino, char tipoO, char tipoD)
{
    mi_waitSem();
    struct superbloque sb;
    if (bread(posSB, &sb) == FALLO)
    {
        // error sb
        mi_signalSem();
        return FALLO;
    }

    unsigned int p_inodo_origen = 0, p_entrada_origen = 0,
                 p_inodo_destino = 0, p_entrada_destino = 0;
    int r_buscar_e_origen, r_buscar_e_destino;

    r_buscar_e_origen = buscar_en_cache(origen);
    if (r_buscar_e_origen < 0)
    {                                                                                                           // actualizar cache
        r_buscar_e_origen = buscar_entrada(origen, &sb.posInodoRaiz, &p_inodo_origen, &p_entrada_origen, 0, 4); // permisos lectura
        struct UltimaEntrada entrada;
        strncpy(entrada.camino, origen, sizeof(entrada.camino) - 1);
        entrada.camino[sizeof(entrada.camino) - 1] = '\0';
        entrada.p_inodo = p_inodo_origen;
#if DEBUG9
        fprintf(stderr, ORANGE "[mi_read() -> Actualizamos la caché de lectura]\n" RESET);
#endif
        actualizar_cache(&entrada);
    }
    if (r_buscar_e_origen != EXITO)
    {
        mostrar_error_buscar_entrada(r_buscar_e_origen);
        fprintf(stderr, RED "ERROR: mi_cp() -> no se ha podido buscar entrada origen\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    // tenemos origen, si es fichero, destino puede ser fichero, crear en esa ruta, o directorio, crear dentro de ese directorio

    struct inodo inodoOrigen, inodoDestino;
    if (leer_inodo(p_inodo_origen, &inodoOrigen) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }

    if (tipoO == 'f')
    {
        char nuevoDestino[strlen(origen) + strlen(destino)];
        if (tipoD == 'd')
        {
#if DEBUGEXTRA
            fprintf(stderr, GRAY "[mi_cp(): f-> d]\n" RESET);
#endif
            // copiar nombre a la carpeta, luego f->f
            char copia[strlen(origen)];

            memset(nuevoDestino, '\0', sizeof(nuevoDestino));
            memset(copia, '\0', sizeof(copia));
            strcpy(copia, origen);
            strcpy(nuevoDestino, destino);
            char *last = NULL;
            char *tok = strtok(copia, "/");
            while (tok != NULL)
            {
                last = tok;
                tok = strtok(NULL, "/");
            }

            strcat(nuevoDestino, last);
            destino = nuevoDestino;
#if DEBUGEXTRA
            fprintf(stderr, GRAY "[mi_cp(): nueva ruta destino %s]\n" RESET, destino);
#endif
        }
#if DEBUGEXTRA
        fprintf(stderr, GRAY "[mi_cp(): f-> f]\n" RESET);
#endif
        if (bread(posSB, &sb) == FALLO)
        {
            // error sb
            mi_signalSem();
            return FALLO;
        }
        r_buscar_e_destino = buscar_entrada(destino, &sb.posInodoRaiz, &p_inodo_destino, &p_entrada_destino, 1, 6); // reservamos
        if (r_buscar_e_destino < EXITO)
        {
            mostrar_error_buscar_entrada(r_buscar_e_destino);
            fprintf(stderr, RED "ERROR: mi_cp() -> no se ha podido buscar entrada destino\n" RESET);

            mi_signalSem();
            return FALLO;
        }

        if (leer_inodo(p_inodo_destino, &inodoDestino) == FALLO)
        {
            mi_signalSem();
            return FALLO;
        }

        mi_cp_aux(inodoOrigen, p_inodo_origen, p_inodo_destino);
    }
    else
    {
        mi_cp_dir(inodoOrigen, p_inodo_origen, destino, sb.posInodoRaiz);
    }
    mi_signalSem();
    return EXITO;
}

int mi_cp_dir(const struct inodo inodoOrigen, const int p_inodo_origen, const char *ruta_destino, unsigned int posInicialAI)
{
    if (inodoOrigen.tipo != 'd')
    {
        return FALLO;
    }

#if DEBUGEXTRA
    fprintf(stderr, GRAY "[mi_cp_dir() -> copiando a %s]\n" RESET, ruta_destino);
#endif

    int entradasInodo = inodoOrigen.tamEnBytesLog / sizeof(struct entrada);
    struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
    struct inodo inodoEntrada;
    int nBloques = 0;
    int nEntradas = 0;
    while (nEntradas < entradasInodo)
    {
        if (mi_read_f(p_inodo_origen, entradas, nBloques * BLOCKSIZE, sizeof(entradas)) == FALLO)
        {
            return FALLO;
        }
        for (int i = 0; nEntradas < entradasInodo && i < ENTRADASBLOQUE; i++)
        {
            if (strcmp(entradas[i].nombre, "")==0){
                continue;
            }
            if (leer_inodo(entradas[i].ninodo, &inodoEntrada) == FALLO)
            {
                return FALLO;
            }

            if (inodoEntrada.tipo == 'l')
            {
                continue;
            }
            fprintf(stderr, GRAY "entrada.nombre: %s\n" RESET, entradas[i].nombre);
            char nuevoDestino[strlen(ruta_destino) + strlen(entradas[i].nombre)+2];
            memset(nuevoDestino, '\0', sizeof(nuevoDestino));
            strcpy(nuevoDestino, ruta_destino);
            strcat(nuevoDestino, entradas[i].nombre);
            if (inodoEntrada.tipo == 'd')
            {

                strcat(nuevoDestino, "/");
                fprintf(stderr, GRAY "[mi_cp`_dir() -> DIR nuevoDestino: %s]\n" RESET, nuevoDestino);

                unsigned int auxIA = posInicialAI;
                unsigned int p_inodo_dest, p_entrada_dest;
                fprintf(stderr, "auxIA: %u\n", auxIA);
                int r_buscar_e = buscar_entrada(nuevoDestino, &auxIA, &p_inodo_dest, &p_entrada_dest, 1, 6);
                if (r_buscar_e < EXITO)
                {
                    mostrar_error_buscar_entrada(r_buscar_e);
                    return FALLO;
                }
                if(mi_cp_dir(inodoEntrada, entradas[i].ninodo, nuevoDestino, auxIA)==FALLO){
                    return FALLO;
                }
            }
            else
            {
                fprintf(stderr, GRAY "[mi_cp`_dir() -> FILE nuevoDestino: %s]\n" RESET, nuevoDestino);

                unsigned int auxIA = posInicialAI;
                unsigned int p_inodo_dest, p_entrada_dest;
                int r_buscar_e = buscar_entrada(nuevoDestino, &auxIA, &p_inodo_dest, &p_entrada_dest, 1, 6);
                if (r_buscar_e < EXITO)
                {
                    mostrar_error_buscar_entrada(r_buscar_e);
                    return FALLO;
                }
                if(mi_cp_aux(inodoOrigen, p_inodo_origen, p_inodo_dest)==FALLO){
                    return FALLO;
                }
            }
        }
    }
    return EXITO;
}

/**
 * copia el contenido de un fichero en otro
 */
int mi_cp_aux(const struct inodo iOrigen, const int p_iOrigen, const int p_iDestino)
{
    char aux[BLOCKSIZE];
    char cmp[BLOCKSIZE];
    memset(aux, '\0', sizeof(aux));
    memset(cmp, '\0', sizeof(aux));

    int bloquesNoVacios = 0;
    for (int offset = 0; offset < iOrigen.tamEnBytesLog && bloquesNoVacios < iOrigen.numBloquesOcupados; offset += BLOCKSIZE)
    {
        int bytes_leidos = mi_read_f(p_iOrigen, aux, offset, BLOCKSIZE);
        if (bytes_leidos == FALLO)
        {
            mi_signalSem();
            return FALLO;
        }

        if (memcmp(aux, cmp, sizeof(aux)) == 0)
        {
            continue;
        }
        bloquesNoVacios++;
        if (mi_write_f(p_iDestino, aux, offset, bytes_leidos) == FALLO)
        {
            mi_signalSem();
            return FALLO;
        }
        memset(aux, '\0', sizeof(aux));
    }

    mi_chmod_f(p_iDestino, iOrigen.permisos);

    return EXITO;
}