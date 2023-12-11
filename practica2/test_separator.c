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

const char *delim = " \t\n\r";
char *dc_delim = "\"";

char line[COMMAND_LINE_SIZE];
char aux_line[COMMAND_LINE_SIZE];
char *args[ARGS_SIZE];

int main()
{
    int nt = 0;
    while (true)
    {
        nt = 0;
        fgets(line, COMMAND_LINE_SIZE, stdin);
        fprintf(stdout, "Line es %s\n", line);
        // strcpy(line, "hola\" lol   3\"espacios\ sep");
        
        char *token = strtok(line, delim);
        char *lineleft = line;
        bool d_comilla = false;
        char *p_dc = NULL;

        memset(aux_line, '\000', COMMAND_LINE_SIZE);

        while (token != NULL)
        {
            fprintf(stdout, "token |%s|\n", token);
            fprintf(stdout, "len token %lu\n", strlen(token));
            p_dc = strchr(token, '\"');
            if (p_dc != NULL)
            {
                d_comilla = !d_comilla;
                fprintf(stdout, "comillas detectadas %i\n", d_comilla);
                strncat(aux_line, token, p_dc - token);
                if (p_dc != token)
                {
                    if (*(p_dc + 1) == ' ' || *(p_dc + 1) == '\0')
                    {
                        strcat(aux_line, " "); // fix strtok
                    }
                }
                char *p_cd2 = strchr(p_dc+1, '\"');
                if(p_cd2==NULL)
                {
                    strcat(aux_line, p_dc + 1);
                }else{
                    strncat(aux_line, p_dc+1, p_cd2-p_dc-1);
                    strcat(aux_line, p_cd2+1);
                }
                if (p_dc == token)
                {
                    strcat(aux_line, " "); // fix strtok
                }

                fprintf(stdout, "aux_line %s\n", aux_line);

                lineleft = line + (token + strlen(token) - line) + 1;
                fprintf(stdout, "lineleft |%s|\n", lineleft);
                p_dc = strchr(lineleft, '\"'); // check if not
                if (p_dc == NULL && strlen(lineleft) == 0)
                {
                    if (*(aux_line + strlen(aux_line) - 1) == '\"')
                    {
                        *(aux_line + strlen(aux_line) - 1) = '\0';
                    }
                    token = strtok(NULL, dc_delim);
                    token = strtok(NULL, delim);
                }
                else
                {
                    strncat(aux_line, lineleft, p_dc - lineleft);

                    fprintf(stdout, "aux_line2 %s\n", aux_line);
                    if (*(p_dc + 1) != ' ')
                    { // check all delim, check "\ "!
                        token = strtok(NULL, dc_delim);
                        token = strtok(NULL, delim);
                        if (token != NULL)
                        {
                            strcat(aux_line, token);
                        }
                    }
                    else
                    {
                        token = strtok(NULL, dc_delim);
                        token = strtok(NULL, delim);
                    }
                }
                *(args + nt++) = (char *)aux_line;
            }

            else
            {
                *(args + nt++) = token;
            }
            token = strtok(NULL, delim);
        }

        *(args + nt) = NULL;
        for (int i = 0; i < nt + 1; i++)
        {
            fprintf(stdout, "ARG: |%s|\n", *(args + i));
        }
    }
    return nt;
}