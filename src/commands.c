

#include "commands.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const CommandEntry command_table[] = {
    // Název, Počet argumentů, Funkce
    {"cp",      2,  handle_cp},
    {"mv",      2,  handle_mv},
    {"rm",      1,  handle_rm},
    {"mkdir",   1,  handle_mkdir},
    {"rmdir",   1,  handle_rmdir},
    {"ls",      0,  handle_ls},  
    {"cat",     1,  handle_cat},
    {"cd",      1,  handle_cd},
    {"pwd",     0,  handle_pwd},
    {"info",    1,  handle_info},
    {"incp",    2,  handle_incp},
    {"outcp",   2,  handle_outcp},
    {"load",    1,  handle_load},
    {"format",  1,  handle_format},
    {"statfs",  0,  handle_statfs},
    {NULL,      0,  NULL}          // Zarážka, která označuje konec tabulky
};

/**
 * @brief Hlavní dispečer. Najde a bezpečně zavolá funkci pro daný příkaz.
 * @param token_count Počet naparsovaných tokenů.
 * @param tokens Pole naparsovaných tokenů.
 */
void execute_command(int token_count, char **tokens) {
    if (token_count == 0) {
        return; // Prázdný řádek
    }

    char *command_name = tokens[0];
    int arg_count = token_count - 1; // Skutečný počet argumentů

    // Projdeme tabulku a hledáme shodu
    for (int i = 0; command_table[i].name != NULL; i++) {
        if (strcmp(command_name, command_table[i].name) == 0) {
            // Našli jsme příkaz! Teď zkontrolujeme počet argumentů.
            int required = command_table[i].required_args;

            if (required != -1 && arg_count != required) {
                fprintf(stderr, "CHYBA: Příkaz '%s' vyžaduje přesně %d argumentů, ale bylo zadáno %d.\n",
                        command_name, required, arg_count);
                return; // Chyba, nevoláme funkci
            }
            
            // Speciální případ pro `ls` (0 nebo 1 argument)
            if (required == -1 && arg_count > 1) {
                fprintf(stderr, "CHYBA: Příkaz '%s' vyžaduje 0 nebo 1 argument, ale bylo zadáno %d.\n",
                        command_name, arg_count);
                return;
            }

            // Vše je v pořádku, zavoláme obslužnou funkci
            command_table[i].handler(token_count, tokens);
            return; // Hotovo
        }
    }

    // Pokud cyklus doběhl do konce, příkaz nebyl v tabulce nalezen
    fprintf(stderr, "Unknown command: %s\n", command_name);
}



/* =============================================================================
 * Základní příkazy pro manipulaci se soubory a adresáři
 * ============================================================================= */

void handle_cp(int argc, char **argv) {
    printf("handle_cp was called\n");
}

void handle_mv(int argc, char **argv) {
    printf("handle_mv was called\n");
}

void handle_rm(int argc, char **argv) {
    printf("handle_rm was called\n");
}

void handle_mkdir(int argc, char **argv) {
    printf("handle_mkdir was called\n");
}

void handle_rmdir(int argc, char **argv) {
    printf("handle_rmdir was called\n");
}

void handle_cat(int argc, char **argv) {
    printf("handle_cat was called\n");
}

/* =============================================================================
 * Příkazy pro navigaci a výpis informací
 * ============================================================================= */

void handle_ls(int argc, char **argv) {
    printf("handle_ls was called\n");
}

void handle_cd(int argc, char **argv) {
    printf("handle_cd was called\n");
}

void handle_pwd(int argc, char **argv) {
    printf("handle_pwd was called\n");
}

void handle_info(int argc, char **argv) {
    printf("handle_info was called\n");
}

/* =============================================================================
 * Příkazy pro přenos souborů a správu FS
 * ============================================================================= */

void handle_incp(int argc, char **argv) {
    printf("handle_incp was called\n");
}

void handle_outcp(int argc, char **argv) {
    printf("handle_outcp was called\n");
}

void handle_load(int argc, char **argv) {
    printf("handle_load was called\n");
}

void handle_format(int argc, char **argv) {
    printf("handle_format was called\n");
}

void handle_statfs(int argc, char **argv) {
    printf("handle_statfs was called\n");
}


