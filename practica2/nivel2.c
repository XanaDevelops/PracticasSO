#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define DEBUG 1

#define COMMAND_LINE_SIZE 1024 // max size command line
#define ARGS_SIZE 64

#define RESET "\033[0m"
#define NEGRO_T "\x1b[30m"
#define NEGRO_F "\x1b[40m"
#define GRIS_T "\x1b[90m" // #define GRIS_T "\x1b[94m"
#define ROJO_T "\x1b[31m"
#define VERDE_T "\x1b[32m"
#define AMARILLO_T "\x1b[33m"
#define AZUL_T "\x1b[34m"
#define MAGENTA_T "\x1b[35m"
#define CYAN_T "\x1b[36m"
#define BLANCO_T "\x1b[97m"
#define NEGRITA "\x1b[1m"

#define MENSAJE_DESPEDIDA "\nQue la fuerza te acompañe\n\n"

char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs();
int internal_fg(char **args);
int internal_bg(char **args);

void imprimir_prompt();
char *parse_cd(char **args);

const char *delim = " \t\n\r";

char line[COMMAND_LINE_SIZE];
char *args[ARGS_SIZE];

int main()
{
    while (1)
    {
        if (read_line(line) == NULL)
        {
            continue;
        }
        int r = execute_line(line);
        if (r > 0)
        {
            int isIn = check_internal(args);
            if (isIn)
            {
#if DEBUG
                fprintf(stdout, GRIS_T "main(): Se ha ejecutado comando interno\n" RESET);
#endif
                continue;
            }
        }
    }
}
/**
 * Funció: read_line
 * -------------------
 * Llegeix una linea per stdin
 *
 * param: line --> punter de la cadena de caràcters on guardar la linea
 *
 * return: punter de la cadena de caràcters amb la linea
 */
char *read_line(char *line)
{
    imprimir_prompt();
    // leer usuario
    fgets(line, COMMAND_LINE_SIZE, stdin);
    if (feof(stdin))
    {
#if DEBUG
        fprintf(stdout, GRIS_T "read_line(): detectado EOF" RESET);
#endif
        return NULL;
    }

    if (line != NULL)
    {
        char *endline = strrchr(line, '\n');
        *(endline) = '\0';
    }

    return line;
}

/**
 * Funció: execute_line
 * -------------------
 * Executa una linea
 *
 * param: line --> punter de la cadena de caràcters de la linea
 *
 * return: número de tokens dins la linea
 */
int execute_line(char *line)
{

    int n_tokens = parse_args(args, line);
#if DEBUG
    fprintf(stdout, GRIS_T "execute_line(): ntokens = %d\n", n_tokens);
#endif
    return n_tokens; // placeholder
}

/**
 * Funció: parse_args
 * -------------------
 * Parsea una linea de comandes
 *
 * param:
 *      args --> punter al punter dels tokens d'arguments
 *      line --> punter de la cadena de caràcters de la linea
 *
 * return: número de tokens dins la linea
 */
int parse_args(char **args, char *line)
{
#if DEBUG
    fprintf(stdout, GRIS_T "parse_args(): parseando %s\n" RESET, line);
#endif
    char *token = strtok(line, delim);

    int nt = 0;
    while (token != NULL)
    {
        if (nt >= ARGS_SIZE)
        {
            fprintf(stderr, ROJO_T "parse_args(): ERROR: demasiados argumentos\n" RESET);
            return -1;
        }
#if DEBUG
        fprintf(stdout, GRIS_T "parse_args(): token: %s\n" RESET, token);
#endif
        if (*(token) == '#')
        {
#if DEBUG
            fprintf(stdout, GRIS_T "parse_args(): Comentario detectado -> (null)\n" RESET);
#endif
            *(args + nt) = token;
            break;
        }
        *(args + nt++) = token;
        token = strtok(NULL, delim);
    }

    *(args + nt) = NULL;
    /*
    for (int i = 0; i < nt + 1; i++)
    {
        printf("ARGS: %s\n", *(args + i));
    }*/

    return nt;
}

/**
 * Funció: check_internal
 * -------------------
 * Comprova si es una comanda interna
 *
 * param: args --> punter al punter dels tokens d'arguments
 *
 * return: true si es intern, false si no
 */
int check_internal(char **args)
{
    char *cmd = *(args);
#if DEBUG
    fprintf(stdout, GRIS_T "check_internal(): comprobando %s...\n" RESET, cmd);
#endif
    const int n_cmd = 7;
    const char *cmds_text[] = {"cd", "export", "source", "fg", "bg", "jobs", "exit"};
    const int (*cmds[])(char **) = {internal_cd, internal_export, internal_source, internal_fg, internal_bg};
    for (int i = 0; i < n_cmd; i++)
    {
        if (!strcmp(cmd, *(cmds_text + i)))
        {
            switch (i)
            {
            case 5:
                internal_jobs();
                break;
            case 6:
                printf(MENSAJE_DESPEDIDA);
                exit(0);

            default:
                cmds[i](args);
                break;
            }
            return true;
        }
    }

    return false;
}

int internal_cd(char **args)
{
    printf("Cambiar de directori\n");
    char *cd = parse_cd(args);
    fprintf(stdout, GRIS_T "cd : %s\n" RESET, cd);
    return 0;
}

char *parse_cd(char **args)
{
    bool s_comilla = false, d_comilla = false;

    char *arg = *(++args);
    char new_cd[COMMAND_LINE_SIZE];
    memset(new_cd, '\0', COMMAND_LINE_SIZE);
    int cd_len = 0;
    while (arg != NULL)
    {
        fprintf(stdout, GRIS_T "parse_cd(): comprobando cd: %s\n" RESET, arg);
        if (!d_comilla)
        {
            char *arg_c = strchr(arg, '"');
            if (arg_c != NULL)
            {
                fprintf(stdout, GRIS_T "d_comilla %s\n" RESET, arg_c);
                d_comilla = true;
            }
        }
        else
        {
        }
        strcat(new_cd, arg);

        if (d_comilla)
        {
            arg = *(++args);
        }
        else
        {
            break;
        }
    }
    fprintf(stdout, GRIS_T "parse_cd(): cd final=%s\n" RESET, new_cd);
    return new_cd;
}

/**
 * Funció: internal_export
 * -------------------
 * Obtenció de variables d'entorn
 *
 * param: args --> punter al punter dels tokens d'arguments
 * args[1] -> NOM=VALOR
 *
 * return: 1 si existeix un error de sintaxi, 0 si s'ha
 * fet l'exportació correctament.
 */
int internal_export(char **args)
{
    const char *delim = "=";

    if (args[1] == NULL)
    {
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        return 1;
    }

    char *variable = strtok(args[1], delim);
    char *valor = strtok(NULL, delim);

    if (variable == NULL || valor == NULL)
    {
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        return 1;
    }

#if DEBUG
    fprintf(stdout, GRIS_T "[internal_export()→ nombre: %s]\n" RESET, variable);
    fprintf(stdout, GRIS_T "[internal_export()→ valor: %s]\n" RESET, valor);
#endif

    char *antic_valor = getenv(variable);

    if (antic_valor == NULL)
    {
#if DEBUG
        fprintf(stdout, GRIS_T "[internal_export()→ antiguo valor para %s: (null)]\n" RESET, variable);
#endif
    }
    else
    {
#if DEBUG
        fprintf(stdout, GRIS_T "[internal_export()→ antiguo valor para %s: %s]\n" RESET, variable, antic_valor);
#endif
    }

    setenv(variable, valor, 1);

#if DEBUG
    fprintf(stdout, GRIS_T "[internal_export()→ nuevo valor para %s: %s]\n" RESET, variable, valor);
#endif

    return 0;
}

int internal_source(char **args)
{
    printf("Executar ordres des d'un fitxer en el context actual de l'intèrpret d'ordres\n");
    return 0;
}

int internal_jobs()
{
    printf("Imprimeix la llista de treballs\n");
    return 0;
}

int internal_fg(char **args)
{
    printf("Porta el procés passat per paràmetre a primer plà\n");
    return 0;
}

int internal_bg(char **args)
{
    printf("Seguir executant el procés passat per paràmetre però en segon pla\n");
    return 0;
}

void imprimir_prompt()
{
    fprintf(stdout, ROJO_T "PROMTP$ " RESET);
    fflush(stdout);
    sleep(0.5);
}