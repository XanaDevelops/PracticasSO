#include "simulacion.h"
#include <stddef.h>


int main(int argc, char **argv)
{
    srand(time(NULL) + getpid());
    struct REGISTRO registro;
    registro.nRegistro = rand() % REGMAX;


}