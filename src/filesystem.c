
#include "filesystem.h"

#include <stdio.h>
#include <stdbool.h>


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