#include "directorios.h"

#define REGMAX 50
#define NUMPROCESOS 100

struct REGISTRO { //sizeof(struct REGISTRO): 24 bytes
   int fecha; //Precisión segundos [opcionalmente microsegundos con struct timeval]
   int pid; //PID del proceso que lo ha creado
   int nEscritura; //Entero con el nº de escritura, de 1 a 50 (orden por tiempo)
   int nRegistro; //Entero con el nº del registro dentro del fichero: [0..REGMAX-1] (orden por posición)
};
