#include "directorios.h"
#include <string.h>

#define DEBUG7A 0
#define DEBUG7B 1

// Se ha aplicado mejora nivell7 pagina 10 nota de pie 7

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

    // CALCULAR ENTRADES INODO (COMPROVAR SI ESTÀ BÉ)
    cant_entradas_inodo = inodo_dir.nlinks;

    // Número de entrada inicial
    num_entrada_inodo = 0;

    bytesleidos = 0;

    trobat = 0;
    if (cant_entradas_inodo > 0)
    {
        while ((num_entrada_inodo < cant_entradas_inodo) && !trobat)
        {
            bytesleidos += mi_read_f(*p_inodo_dir, buff_entradas, bytesleidos, BLOCKSIZE);
            for (int i = 0; i < ((BLOCKSIZE / sizeof(struct entrada)) && !trobat && (num_entrada_inodo < cant_entradas_inodo)); i++)
            {
                if (strcmp(inicial, buff_entradas[i].nombre) == 0)
                {
                    trobat = 1;
                    memcpy(&entrada, &buff_entradas[i], sizeof(struct entrada));
                    break;
                }
                num_entrada_inodo++;
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
                // COPIAR *inicial EN EL NOMBRE DE LA ENTRADA (COMPROVAR SI ESTA BE !!!!!!!)
                memset(&entrada, '\0', sizeof(struct entrada));
                memcpy(entrada.nombre, inicial, sizeof(struct entrada));

                if (tipo == 'd')
                {
                    if (strcmp(final, "/") == 0)
                    {
                        // Reservar inodo como directorio y asignarlo a la entrada (REVISAR !!!!!!!!!!!!)
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
                    // Reservar inodo como fichero y asignarlo a la entrada (REVISAR !!!!!!!!!!!!)
                    numInodo = reservar_inodo('f', permisos);
                    entrada.ninodo = numInodo;
                }
#if DEBUG7B
                fprintf(stderr, GRAY "[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n" RESET, numInodo, tipo, permisos, inicial);
#endif
                //*******************************************************
                // Actualizar nlinks
                inodo_dir.nlinks++;

                // Salvar inodo
                if (escribir_inodo(*p_inodo_dir, &inodo_dir) == FALLO)
                {
                    fprintf(stderr, RED "ERROR: buscar_entrada(): No se ha podido escribir el inodo %d \n" RESET, *p_inodo_dir);
                    return FALLO;
                }
                //*************************************************
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
        // Asignar a *p_inodo_dir el puntero al inodo que se indica en la entrada encontrada (REVISAR !!!!!!!!)
        *p_inodo_dir = entrada.ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }

    return EXITO;
}

/**
 * Separa camino en inicio, final, configura tipo
 * Asume tamaño inicial y final igual a camino, problemas si no...
 * return: EXITO o FALLO
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
        tipo = "d\0";
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
    if (pos == NULL) //es fichero
    {
        tipo = "f\0";
    }
    else //es directorio
    {
        tipo = "d\0";
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


/**
 * Crea un fichero/directorio y su entrada de directorio
*/
/*
int mi_creat(const char *camino, unsigned char permisos)
{
    // LECTURA SUPERBLOQUE
    struct superbloque sb;
    if (bread(posSB, &sb) == FALLO)
    {
        fprintf(stderr, RED "ERROR: mi_creat(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }

    buscar_entrada(camino, sb.posInodoRaiz, p inodo, p entrada ,1, permisos);
}
*/