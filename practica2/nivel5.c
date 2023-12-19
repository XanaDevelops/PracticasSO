#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <signal.h>

#define DEBUG 1

#define COMMAND_LINE_SIZE 1024 // max size command line
#define ARGS_SIZE 64
#define N_JOBS 64

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

int is_background(char **args);
int jobs_list_add(pid_t pid, char estado, char *cmd);
int jobs_list_find(pid_t pid);
int jobs_list_remove(int pos);

void reaper(int signum);
void ctrlc(int signum);
void ctrlz(int signum);

void imprimir_prompt();

struct info_job
{
    pid_t pid;
    char estado;                 // ‘N’, ’E’, ‘D’, ‘F’ (‘N’: Ninguno, ‘E’: Ejecutándose y ‘D’: Detenido, ‘F’: Finalizado)
    char cmd[COMMAND_LINE_SIZE]; // línea de comando asociada
};

const char *delim = " \t\n\r";

static char my_shell[COMMAND_LINE_SIZE];
char line[COMMAND_LINE_SIZE];
char aux_line[COMMAND_LINE_SIZE];
char *args[ARGS_SIZE];

static struct info_job jobs_list[N_JOBS];
static int n_job;

int main(int argc, char **argsc)
{
    // inicializar senyals
    signal(SIGCHLD, reaper);
    signal(SIGINT, ctrlc);
    signal(SIGTSTP, ctrlz);

    strcpy(my_shell, argsc[0]);
    // inicializar jobs_list
    for (int i = 0; i < N_JOBS; i++)
    {
        jobs_list[i].pid = 0;
        jobs_list[i].estado = 'N';
        memset(jobs_list[i].cmd, '\0', COMMAND_LINE_SIZE);
    }

    n_job = 0;

    while (1)
    {
        // inicialitzar line, args, etc
        memset(line, '\0', COMMAND_LINE_SIZE);
        memset(aux_line, '\0', COMMAND_LINE_SIZE);
        for (int i = 0; i < ARGS_SIZE; i++)
        {
            *(args + i) = NULL;
        }
        if (read_line(line) == NULL)
        {
            continue;
        }
        int r = execute_line(line);
        if (r > 0)
        {
            if (r == 1)
            {
#if DEBUG
                fprintf(stdout, GRIS_T "[main(): Se ha ejecutado comando interno]\n" RESET);
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
        fprintf(stdout, GRIS_T "\n[read_line(): detectado EOF]\n" RESET);
#endif
        clearerr(stdin);
        return NULL;
    }

    if (line != NULL)
    {
        char *endline = strrchr(line, '\n');
        if (endline != NULL)
        {
            *(endline) = '\0';
        }
    }

    return line;
}


/**
 * Funció: is_background
 * --------------------
 * Comprova si una es una comanda en background .
 * 
 * param: args --> punter al punter dels tokens d'arguments
 * 
 * return: 1 si es tracta d'una comanda en background (conté token &), 
 * 0 en cas contrari.
*/
int is_background(char **args)
{
    int i = 0;

    // cerca del token '&'
    while(args[i]) {
        // comprovació per si s'ha trobat el token '&'
        if(!strcmp(args[i], "&")) {
            args[i] = NULL; 
            return 1;
        }

        i++;
    }

    return 0;
}

/**
 * Funció: execute_line
 * -------------------
 * Executa una linea
 *
 * param: line --> punter de la cadena de caràcters de la linea
 *
 * return: 1 si és una comanda interna, -1 si no hi ha arguments a la línia o no són vàlids,
 * 0 en altre cas.
 */
int execute_line(char *line)
{
    // variable per guardar els arguments de la línia
    char *args[ARGS_SIZE];

    // variable còpia de line
    char cline[COMMAND_LINE_SIZE];
    strcpy(cline, line);

    // comprovar si no hi ha arguments
    if (!parse_args(args, cline))
        return -1;

    // comprovar si és una comanda interna
    if (check_internal(args))
        return 1;

    // comprovar si és una comanda en background
    int isBack = is_background(args);

    // creació de fill per executar la comanda de forma externa
    pid_t child = fork();

    // error al crear el fill
    if (child == -1)
    {
        perror(ROJO_T "fork");
        return -1;
    }

    // comprovar si es tracata d'un procés fill o pare
    if (child == 0)
    { // procés fill
        // asginar acció per defecte a SIGCHLD
        signal(SIGCHLD, SIG_DFL);
        // ignorar la senyal SIGINT (Ctrl + C)
        signal(SIGINT, SIG_IGN);
        // ignorar la senyal SIGTSTP (Ctrl + Z)
        signal(SIGTSTP, SIG_IGN);

        // cridada al sistema per executar la comanda externa
        execvp(args[0], args);

        // comanda fallida
        fprintf(stderr, ROJO_T "Error, ordre inexistent: %s \n" RESET, args[0]);
        exit(-1);
    }
    else
    { // procés pare
        // visualització del PID del pare i del fill
#if DEBUG
        fprintf(stdout, GRIS_T "[execute_line(): PID pare: %d (%s)]\n" RESET, getppid(), my_shell);
        fprintf(stdout, GRIS_T "[execute_line(): PID fill: %d (%s)]\n" RESET, child, line);
#endif
        fprintf(stdout, RESET);
        // fflush(stdout);
        fflush(NULL);    
        
        if(!isBack) {   // foreground
            // actualització de les dades de jobs_list[0] amb els procesos fill en foreground
            jobs_list[0].pid = child;
            jobs_list[0].estado = 'E';
            strcpy(jobs_list[0].cmd, line);

            // espera a finalització del procés fill executant-se
            pause();

        } else {    // background
            jobs_list_add(child, 'E', line);
        }

        return 0;
    }
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
    bool global_d_comilla = false;
    int aux_line_index = 0, linesize = 0;

    if (line == NULL)
    {
        return -1;
    }

    int nt = 0;

    nt = 0;
    global_d_comilla = false;
    memset(aux_line, '\000', COMMAND_LINE_SIZE);
    aux_line_index = 0;

    linesize = strlen(line);
    char *token = strtok(line, delim);
    while (token != NULL)
    {
        int m = strlen(token);
        if (token >= line + aux_line_index)
        {
            // goto next_token;
            int aux_start = aux_line_index;
            char *p_dc = strchr(token, '\"');
            if (p_dc != NULL)
            {
                global_d_comilla = !global_d_comilla;
                if (global_d_comilla)
                {
                    bool d_comilla = true;
                    char c = *(token);

                    for (int i = 0; i < m; i++)
                    {
                        if (c != '\"')
                        {
                            aux_line[aux_line_index++] = c;
                        }
                        else
                        {
                            global_d_comilla = !global_d_comilla;
                            d_comilla = !d_comilla;
                        }
                        c = *(++token);
                    }
                    if (d_comilla)
                    {
                        aux_line_index++;
                    }
                    else
                    {
                        int linei = 0;
                        aux_line[aux_line_index++] = ' ';

                        c = line[token - line + ++linei];

                        bool hasmore = false;

                        while (true)
                        {
                            if (linesize < aux_line_index)
                            {
                                break;
                            }
                            if (c == '\"')
                            {
                                global_d_comilla = !global_d_comilla;
                                break;
                            }
                            if (c == ' ')
                            {
                                hasmore = true;
                            }
                            aux_line[aux_line_index++] = c;
                            c = line[token - line + ++linei];
                        }
                        linei++;
                        c = line[token - line + linei];

                        while (true)
                        {
                            if (linesize < aux_line_index)
                            {
                                break;
                            }
                            if (c == ' ' || c == '\0')
                            {
                                if (c == ' ')
                                {
                                    strtok(NULL, delim);
                                    if (hasmore)
                                    {
                                        strtok(NULL, delim);
                                    }
                                }
                                break;
                            }
                            hasmore = true;
                            aux_line[aux_line_index++] = c;
                            c = line[token - line + ++linei];
                        }
                        // token = strtok(NULL, delim);
                    }
                    *(args + nt++) = aux_line + aux_start;
                }
            }
            else
            {
                *(args + nt++) = token;
            }
        }

        token = strtok(NULL, delim);
        aux_line_index++;
        // testear
        if (token != NULL)
        {
            if (*(token) == ' ' && strlen(token) == 1)
            {
                token = NULL;
            }
        }
    }

    *(args + nt) = NULL;
#if DEBUG
    for (int i = 0; i < nt + 1; i++)
    {
        fprintf(stdout, GRIS_T "[parse_args(): token %i: |%s|]\n" RESET, i, *(args + i));
    }
#endif
    return nt;
}

/**
 *  Funció: ctrlc
 * --------------------
 *  S'executa quan hi ha un Ctrl + C, i interromp l'execució principal.
 *
 *  param: SIGINT (llençada pel Ctrl + C)
 *
 */
void ctrlc(int signum)
{
    signal(SIGINT, ctrlc);
    fprintf(stdout, "\n");
#if DEBUG
    fprintf(stdout, GRIS_T "[ctrlc(): Interromp execució]\n" RESET);
#endif
    // mirar si hi ha un procés a foreground
    if (jobs_list[0].pid != 0)
    {
#if DEBUG
        fprintf(stdout, GRIS_T "[ctrlc(): %s és procés fill. PID: %d]\n" RESET, jobs_list[0].cmd, getpid());
#endif
        // mirar si el procés és el nostre shell
        if (strcmp(jobs_list[0].cmd, my_shell))
        {
#if DEBUG
            fprintf(stdout, GRIS_T "[ctrlc(): %s no és una execució del nostre mini shelll, per tant s'interromprà. PID: %d]\n" RESET, jobs_list[0].cmd, getpid());
#endif
            kill(jobs_list[0].pid, SIGTERM);
        }
        pause();
        return;
    }
#if DEBUG
    fprintf(stdout, GRIS_T "[ctrlc(): %s no és una execució en foreground, per tant no se la interromprà. PID: %d]\n" RESET, jobs_list[0].cmd, getpid());
#endif
    fflush(stdout);
    return;
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
    fprintf(stdout, GRIS_T "[check_internal(): comprobando %s]\n" RESET, cmd);
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

/**
 * Funció: internal_cd
 * -------------------
 * Cambiar de directori.
 *
 * param: args --> punter al punter dels tokens d'arguments
 * args[1] -> NOM=VALOR
 *
 * return: int 0 si s'executa correctament.
 */
int internal_cd(char **args)
{
#if DEBUG
    fprintf(stdout, GRIS_T "[internal_cd(): Canviant directori...]\n" RESET);
#endif
    char cwd[COMMAND_LINE_SIZE];
    memset(cwd, '\0', sizeof(cwd));
    // Si no hi ha atributs anar a Home
    if (!args[1])
    {
        strcpy(cwd, getenv("HOME")); // Variable d'entorn de la home de l'usuari
    }
    else
    {
        strcat(cwd, args[1]);
    }
#if DEBUG
    fprintf(stdout, GRIS_T "[internal_cd(): Directori a canviar: %s]\n" RESET, cwd);
#endif
    if (chdir(cwd) == -1)
    {
        perror(ROJO_T "chdir(): Directori no trobat");
        return -1;
    }

#if DEBUG
    fprintf(stdout, GRIS_T "[internal_cd(): Directori canviat]\n" RESET);
#endif
    return 0;
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
    fprintf(stdout, GRIS_T "[internal_export(): nombre: %s]\n" RESET, variable);
    fprintf(stdout, GRIS_T "[internal_export(): valor: %s]\n" RESET, valor);
#endif

    char *antic_valor = getenv(variable);

    if (antic_valor == NULL)
    {
#if DEBUG
        fprintf(stdout, GRIS_T "[internal_export(): antic valor per %s: (null)]\n" RESET, variable);
#endif
    }
    else
    {
#if DEBUG
        fprintf(stdout, GRIS_T "[internal_export(): antic valor per %s: %s]\n" RESET, variable, antic_valor);
#endif
    }

    setenv(variable, valor, 1);

#if DEBUG
    fprintf(stdout, GRIS_T "[internal_export(): antic valor per %s: %s]\n" RESET, variable, valor);
#endif

    return 0;
}

/**
 * Funció: internal_source
 * -------------------
 * Executar comandos des d'un fitxer en el constext actual del shell.
 *
 * param: args --> punter al punter dels tokens d'arguments
 * args[1] -> NOM=VALOR
 *
 * return: int 0 si s'executa correctament.
 */
int internal_source(char **args)
{

#if DEBUG
    fprintf(stdout, GRIS_T "[internal_source(): Executant fitxer ...]\n" RESET);
#endif
    char aux[COMMAND_LINE_SIZE];
    memset(aux, '\0', sizeof(aux));

    if (!args[1])
    {
        perror(ROJO_T "internal_source(): Fitxer no trobat");
        return -1;
    }
    strcpy(aux, args[1]);
    FILE *fp = fopen(aux, "r");
    if (!fp)
    {
        perror(ROJO_T "internal_source(): Fitxer no s'ha pogut obrir");
        return -1;
    }
    char linia[COMMAND_LINE_SIZE];
    while (fgets(linia, COMMAND_LINE_SIZE, fp))
    {
        char *fi = strrchr(linia, '\n');
        if (fi)
        {
            *(fi) = '\0';
        }
#if DEBUG
        fprintf(stdout, GRIS_T "[internal_source(): Executam línia %s]\n" RESET, linia);
#endif
    }

#if DEBUG
    fprintf(stdout, GRIS_T "[internal_source(): Fitxer executat]\n" RESET);
#endif
    return 0;
}

int internal_jobs()
{
    //printf("Imprimeix la llista de treballs\n");
    for(int i=0;i<n_job;i++){
        
        if(jobs_list[i].pid==0){
            continue;
        }
        //FORMATAR COMO job de bash!!!!
        fprintf(stdout, "[%d] status:%c cmd:%s\n", jobs_list[i].pid, jobs_list[i].estado, jobs_list[i].cmd);
    }
    return 0;
}

int jobs_list_add(pid_t pid, char estado, char *cmd){
    if(n_job<N_JOBS){
        jobs_list[n_job].pid = pid;
        jobs_list[n_job].estado = estado;
        strncpy(jobs_list[n_job].cmd, cmd, COMMAND_LINE_SIZE);
        n_job++;
        return 0;
    }else{
        fprintf(stderr, AMARILLO_T "jobs_list_add() W: Se ha alcanzado el maximo en jobs_list\n" RESET);
        return -1;
    }
}

int jobs_list_remove(int pos){
    if(pos>n_job || pos<0){
        fprintf(stderr, ROJO_T "jobs_list_remove() E: pos fuera de rango\n");
        return -1;
    }
    if(pos==n_job){
        jobs_list[pos].pid=0;
        jobs_list[pos].estado='F';
        memset(jobs_list[pos].cmd, '\0', COMMAND_LINE_SIZE);
    }else{
        jobs_list[pos].pid=jobs_list[n_job].pid;
        jobs_list[pos].estado=jobs_list[n_job].estado;
        strncpy(jobs_list[pos].cmd, jobs_list[n_job].cmd, COMMAND_LINE_SIZE);
    }

    n_job--;
    return 0;
}

int jobs_list_find(pid_t pid){
    for(int i=0;i<n_job;i++){
        if(jobs_list[i].pid==pid){
            return i;
        }
    }
    return -1;
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

void reaper(int signum)
{
    signal(SIGCHLD, reaper);
    int status;
    pid_t ended;
    while ((ended = waitpid(-1, &status, WNOHANG)) > 0)
    {
        fprintf(stdout, GRIS_T "[reaper(): loop: %i status %i]\n" RESET, ended, status);
        if (ended == jobs_list[0].pid)
        {
#if DEBUG
            // obtenció de les dades de finalització del fill
            if (WIFEXITED(status))
            {
                fprintf(stdout, GRIS_T "[reaper()→Procés fill %d (%s) finalitzat amb exit(), status: %d]\n" RESET,
                        jobs_list[0].pid, jobs_list[0].cmd, WEXITSTATUS(status));
            }
            else if (WIFSIGNALED(status))
            {
                fprintf(stdout, GRIS_T "[reaper()→Procés fill %d (%s) finalitzat amb senyal, status: %d]\n" RESET,
                        jobs_list[0].pid, jobs_list[0].cmd, WTERMSIG(status));
            }
#endif
            jobs_list[0].pid = 0;
            jobs_list[0].estado = 'F';
            memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);
        }
        ended = waitpid(-1, &status, WNOHANG);
    }
}

void imprimir_prompt()
{
    char cwd[COMMAND_LINE_SIZE];
    getcwd(cwd, COMMAND_LINE_SIZE);

    fprintf(stdout, ROJO_T "%s:" RESET, getenv("USER"));

    // Si esta dins usuari  retorna path relatiu a usuari (carpetes a partir d'usuari)
    if (!strncmp(cwd, getenv("HOME"), strlen(getenv("HOME"))))
    {
        fprintf(stdout, VERDE_T "~%s" RESET, &cwd[strlen(getenv("HOME"))]);
    }
    else
    // Sino imprimeix path sencer
    {
        fprintf(stdout, VERDE_T "%s" RESET, cwd);
    }

    fprintf(stdout, ROJO_T "$ " RESET);

    fflush(stdout);
    sleep(0.5);
}
