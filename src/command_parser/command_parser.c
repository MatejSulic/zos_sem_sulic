#include "command_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Vytvori prazdnou strukturu CommandInput */
struct CommandInput create_command() {
    struct CommandInput cmd;
    int i;
    cmd.argc = 0;
    /* Inicializuj vsechny ukazatele na NULL */
    for (i = 0; i < 10; i++) {
        cmd.argv[i] = NULL;
    }
    return cmd;
}

/* Parsuje prikaz ze stringu a rozdeli ho podle mezer */
struct CommandInput parse_command(const char* command) {
    struct CommandInput cmd = create_command();
    int i = 0;
    int start;
    int length;
    
    printf("Parsing command: %s\n", command);
    
    /* Projdi cely string a rozdeluj podle mezer */
    while (command[i] != '\0' && cmd.argc < 10) {
        /* Preskoc vsechny mezery */
        while (command[i] == ' ' && command[i] != '\0') {
            i++;
        }
        
        /* Najdi zacatek slova */
        start = i;
        
        /* Najdi konec slova (mezera nebo konec stringu) */
        while (command[i] != ' ' && command[i] != '\0') {
            i++;
        }
        
        /* Pokud jsme nasli nejake slovo, zkopiruj ho */
        if (i > start) {
            length = i - start;
            
            /* Alokuj pamet pro slovo */
            cmd.argv[cmd.argc] = (char*)malloc(length + 1);
            if (cmd.argv[cmd.argc] == NULL) {
                /* Pri chybe alokace uvolni vse a vrat prazdny prikaz */
                free_command(&cmd);
                cmd.argc = 0;
                return cmd;
            }
            
            /* Zkopiruj slovo do argv */
            strncpy(cmd.argv[cmd.argc], &command[start], length);
            cmd.argv[cmd.argc][length] = '\0';
            
            printf("Argument %d: %s\n", cmd.argc, cmd.argv[cmd.argc]);
            cmd.argc++;
        }
    }
    
    return cmd;
}

/* Uvolni vsechnu alokovanou pamet v CommandInput */
void free_command(struct CommandInput* cmd) {
    int i;
    /* Projdi vsechny argumenty a uvolni pamet */
    for (i = 0; i < cmd->argc; i++) {
        if (cmd->argv[i] != NULL) {
            free(cmd->argv[i]);
            cmd->argv[i] = NULL;
        }
    }
    cmd->argc = 0;
}