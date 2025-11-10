
#include "filesystem.h"
#include "help_functions.h"

#include <stdio.h>
#include <stdbool.h>



FILE *fs_file = NULL;

struct superblock sb;

int32_t current_dir_inode_num = 0;

int64_t total_disk_size = 0;

int32_t cluster_size = CLUSTER_SIZE;

int32_t cluster_count = 0;





void handle_format(int argc, char **argv) {
    printf("handle_format was called\n");

    // 1. Získáme string s velikostí (např. "600M")
    char *size_str = argv[1];

    // 2. Převedeme ho na reálný počet bajtů
    total_disk_size = parse_size_to_bytes(size_str);

    if (total_disk_size <= 0) {
        printf("Chyba: Neplatná velikost disku.\n");
        return;
    }
    
    printf("Formátování na %lld bajtů...\n", total_disk_size);

    // 3. Vypočítáme počet clusterů
    int32_t cluster_count = total_disk_size / cluster_size;

    // 4. Vypočítáme pocet i-uzlů (přibližně 1 i-uzel na 4 clustery)
    int32_t inode_count = cluster_count / 4;

    // 5. Vypočítáme velikosti bitmap a tabulky i-uzlů
    long bitmap_inode_size = (inode_count / 8) + 1; // +1 je pojistka pro zaokrouhlení

    // 6. Velikost bitmapy datových bloků
    long bitmap_data_size = (cluster_count / 8) + 1;

    // 7. Velikost tabulky i-uzlů
    long inode_table_size = inode_count * sizeof(struct pseudo_inode);

    // 8. Vypočítáme startovní adresy jednotlivých částí FS

    int32_t bitmap_inode_start = sizeof(struct superblock);
    int32_t bitmap_data_start = bitmap_inode_start + bitmap_inode_size;
    int32_t inode_table_start = bitmap_data_start + bitmap_data_size;
    int32_t data_block_start = inode_table_start + inode_table_size;

    // Kontrolni vypis
    printf("Cluster size: %d bajtů\n", cluster_size);
    printf("Cluster count: %d\n", cluster_count);
    printf("Inode count: %d\n", inode_count);


    printf("Bitmapa i-uzlů začíná na: %d\n",    bitmap_inode_start);
    printf("Bitmapa datových bloků začíná na: %d\n", bitmap_data_start);
    printf("Tabulka i-uzlů začíná na: %d\n", inode_table_start);
    printf("Datové bloky začínají na: %d\n", data_block_start);


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



void handle_statfs(int argc, char **argv) {
    printf("handle_statfs was called\n");
}