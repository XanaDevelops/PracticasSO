#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>

#define DEBUG 1

#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64

#define RESET "\033[0m"
#define NEGRO_T "\x1b[30m"
#define NEGRO_F "\x1b[40m"
#define GRIS_T "\x1b[94m"
#define ROJO_T "\x1b[31m"
#define VERDE_T "\x1b[32m"
#define AMARILLO_T "\x1b[33m"
#define AZUL_T "\x1b[34m"
#define MAGENTA_T "\x1b[35m"
#define CYAN_T "\x1b[36m"
#define BLANCO_T "\x1b[97m"
#define NEGRITA "\x1b[1m"

char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);

int internal_jobs() {
    printf("Imprimeix la llista de treballs");
    return 0;
}

int internal_fg(char **args) {
    printf("Porta processos a primer plà");
    return 0;
}

int internal_bg(char **args) {
    printf("Continuar processos en el front");
    return 0;
}
