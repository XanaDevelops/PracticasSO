#include "directorios.h"

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, 
                unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    // Declarar variables
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;

    // LECTURA SUPERBLOQUE
    struct superbloque sb;
    if (bread(posSB, &sb) == FALLO)
    {
        fprintf(stderr, RED "ERROR: buscar_entrada(): No se ha podido leer SB\n" RESET);
        return FALLO;
    }

    // Comprobar si es el directorio raíz
    if(camino_parcial[0] == '/') {
        *p_inodo = sb.posInodoRaiz;
        *p_entrada = 0;
        return 0;
    }

    if(extraer_camino(camino_parcial, inicial, final, &tipo) == FALLO) 
    {
        return ERROR_CAMINO_INCORRECTO;
    }

    // Buscar la entrada cuyo nombre se encuentra en inicial
    if(leer_inodo(*p_inodo_dir, &inodo_dir) == FALLO) 
    {
        fprintf(stderr, RED "ERROR: buscar_entrada(): No se ha podido leer el inodo %d\n" RESET, *p_inodo_dir);
        return FALLO;
    }

    // Comprobar si el inodo tiene permisos de lectura
    if ((inodo_dir.permisos & 4) != 4)
    {
        return ERROR_PERMISO_LECTURA;
    }

    // REVISAR, NO SE SI ESTÀ BE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    struct entrada buff_entradas[BLOCKSIZE/sizeof(entrada)];
    memset(buff_entradas, '\0', sizeof(buff_entradas));

    // CALCULAR ENTRADES INODO (COMPROVAR SI ESTÀ BÉ)
    cant_entradas_inodo = inodo_dir.nlinks;

    // Número de entrada inicial
    num_entrada_inodo = 0;
    
    if(cant_entradas_inodo > 0) 
    {
        // AFEGIR FUNCIÓ LLETGIR ENTRADES
        // leer_entrada()

        while((num_entrada_inodo < cant_entradas_inodo) && (inicial != entrada.nombre))
        {
            num_entrada_inodo++;

            // Inicializar buffer de lectura a 0s
            memset(buff_entradas, '\0', sizeof(buff_entradas));

            // AFEGIR FUNCIÓ LLETGIR ENTRADA
            // leer_entrada() 

        }
    }

    // Comprobar si la entrada existe
    if((!strcmp(inicial,entrada.nombre)) && (num_entrada_inodo = cant_entradas_inodo))
    {
        switch(reservar)
        {
            case 0:
                // Modo consulta, como no existe se devuelve error 
                return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
                break;
            case 1:
                // Modo escritura
                // Crear entrada en el directorio referenciado por *p_inodo_dir
                // Si es fichero no se permite la escritura
                if(inodo_dir.tipo = 'f')
                {
                    return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
                }

                // Si es directorio comprobar que tiene permisos de lectura
                if((inodo_dir.permisos & 4) != 4)
                {
                    return ERROR_PERMISO_ESCRITURA;
                }
                else 
                {
                    // COPIAR *inicial EN EL NOMBRE DE LA ENTRADA (COMPROVAR SI ESTA BE !!!!!!!)
                    memcpy(entrada.nombre, *inicial, TAMNOMBRE);

                    if(tipo = 'd')
                    {
                        if(final == '/')
                        {
                            // Reservar inodo como directorio y asignarlo a la entrada (REVISAR !!!!!!!!!!!!)
                            int numInodo = reservar_inodo('d', permisos);
                            entrada.ninodo = numInodo;
                        }
                        else
                        {
                            ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                        }
                    }
                    else 
                    {
                        // Reservar inodo como fichero y asignarlo a la entrada (REVISAR !!!!!!!!!!!!)
                        int numInodo = reservar_inodo('f', permisos);
                        entrada.ninodo = numInodo;
                    }

                    // ESCRIBIR LA ENTRADA EN EL DIRECTORIO PADRE
                    // CONTROLAR ERROR DE ESCRITURA
                }
                break;
        }

        // Comprobar si se ha llegado al final del camino
        if(0) 
        {
            if((num_entrada_inodo < cant_entradas_inodo) && (reservar = 1))
            {
                // Modo escritura y entrada ya existente
                return ERROR_ENTRADA_YA_EXISTENTE;
            }

            // Asignar a *p_inodo el número de inodo del directorio o fichero creado o leído
            // *p_inodo = 
            // ASIGNAR A *p_entrada EL NUMERO DE SU ENTRADA DEL ÚLTIMO DIRECTORIO QUE LO CONTIENE
            // *p_entrada = 
        }
        else 
        {
            // Asignar a *p_inodo_dir el puntero al inodo que se indica en la entrada encontrada (REVISAR !!!!!!!!)
            *p_inodo_dir = entrada.ninodo;
            return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
        }

        return EXITO;
    }





    
}

void mostrar_error_buscar_entrada(int error) {
   // fprintf(stderr, "Error: %d\n", error);
   switch (error) {
   case -2: fprintf(stderr, "Error: Camino incorrecto.\n"); break;
   case -3: fprintf(stderr, "Error: Permiso denegado de lectura.\n"); break;
   case -4: fprintf(stderr, "Error: No existe el archivo o el directorio.\n"); break;
   case -5: fprintf(stderr, "Error: No existe algún directorio intermedio.\n"); break;
   case -6: fprintf(stderr, "Error: Permiso denegado de escritura.\n"); break;
   case -7: fprintf(stderr, "Error: El archivo ya existe.\n"); break;
   case -8: fprintf(stderr, "Error: No es un directorio.\n"); break;
   }
}
