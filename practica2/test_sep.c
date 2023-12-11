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
int aux_line_index = 0, linesize=0;
char *args[ARGS_SIZE];

int main()
{
    int nt = 0;
    while (true)
    {
        nt = 0;
        memset(aux_line, '\000', COMMAND_LINE_SIZE);
        aux_line_index = 0;
        fgets(line, COMMAND_LINE_SIZE, stdin);
        fprintf(stdout, "Line es %s\n", line);
        linesize = strlen(line);
        char *token = strtok(line, delim);
        while (token != NULL)
        {
            int aux_start = aux_line_index;
            char *p_dc = strchr(token, '\"');
            if (p_dc != NULL)
            {

                char c = *(token);
                int m = strlen(token);
                for (int i = 0; i < m; i++)
                {
                    if (c != '\"')
                    {
                        aux_line[aux_line_index++] = c;
                    }
                    c = *(++token);
                }
                
                int linei = 0;
                aux_line[aux_line_index++] = ' ';
                c = line[token-line+ ++linei];
                while(true){
                    if(linesize<aux_line_index){
                        break;
                    }
                    if(c=='\"'){
                        break;
                    }
                    aux_line[aux_line_index++] = c;
                    c = line[token-line+ ++linei];
                }
                linei++;
                aux_line_index++;
                while(true){
                    if(linesize<aux_line_index){
                        break;
                    }
                    if(c==' '){
                        break;
                    }
                    aux_line[aux_line_index++] = c;
                    c = line[token-line+linei++];
                }
                token = strtok(NULL, delim);

                *(args + nt++) = aux_line + aux_start;
                token = strtok(NULL, delim);
            }
            else
            {

                *(args + nt++) = token;
                token = strtok(NULL, delim);
            }
        }

        *(args + nt) = NULL;
        for (int i = 0; i < nt + 1; i++)
        {
            fprintf(stdout, "ARG: |%s|\n", *(args + i));
        }
    }
}