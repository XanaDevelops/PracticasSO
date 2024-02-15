#include "bloques.h"

static int descriptor = 0;

int bmount(const char *camino) {
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);

    if(descriptor == -1) {}
    
}