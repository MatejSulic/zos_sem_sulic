#include "filesystem.h"
#include "help_functions.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

/* =============================================================================
 * GLOBÁLNÍ PROMĚNNÉ
 * ============================================================================= */
FILE *fs_file = NULL;
struct superblock sb;
int32_t current_dir_inode_num = 0;

/* =============================================================================
 * POMOCNÉ FUNKCE - PRÁCE S DISKEM
 * ============================================================================= */

/**
 * @brief Natáhne soubor na požadovanou velikost.
 */
static bool resize_file(int64_t size) {
    if (fseek(fs_file, size - 1, SEEK_SET) != 0) {
        return false;
    }
    if (fwrite("\0", 1, 1, fs_file) != 1) {
        return false;
    }
    return true;
}

/**
 * @brief Vynuluje oblast na disku.
 */
static bool zero_disk_area(long offset, long size) {
    char *zero_buffer = calloc(size, 1);
    if (zero_buffer == NULL) {
        return false;
    }
    
    fseek(fs_file, offset, SEEK_SET);
    size_t written = fwrite(zero_buffer, size, 1, fs_file);
    free(zero_buffer);
    
    return written == 1;
}

/**
 * @brief Nastaví bit v bitmapě.
 */
static bool set_bitmap_bit(long bitmap_start, int32_t bit_index, bool value) {
    int byte_index = bit_index / 8;
    int bit_position = 7 - (bit_index % 8);
    
    // Načti aktuální bajt
    fseek(fs_file, bitmap_start + byte_index, SEEK_SET);
    unsigned char byte;
    if (fread(&byte, 1, 1, fs_file) != 1) {
        return false;
    }
    
    // Uprav bit
    if (value) {
        byte |= (1 << bit_position);
    } else {
        byte &= ~(1 << bit_position);
    }
    
    // Zapiš zpět
    fseek(fs_file, bitmap_start + byte_index, SEEK_SET);
    return fwrite(&byte, 1, 1, fs_file) == 1;
}

/* =============================================================================
 * POMOCNÉ FUNKCE - PRÁCE S I-UZLY
 * ============================================================================= */

/**
 * @brief Načte i-uzel z disku.
 */
struct pseudo_inode fs_read_inode(int32_t inode_num) {
    struct pseudo_inode node;
    long address = sb.inode_start_address + (inode_num * sizeof(struct pseudo_inode));
    
    fseek(fs_file, address, SEEK_SET);
    fread(&node, sizeof(struct pseudo_inode), 1, fs_file);
    
    return node;
}

/**
 * @brief Zapíše i-uzel na disk.
 */
static bool fs_write_inode(int32_t inode_num, const struct pseudo_inode *node) {
    long address = sb.inode_start_address + (inode_num * sizeof(struct pseudo_inode));
    
    fseek(fs_file, address, SEEK_SET);
    return fwrite(node, sizeof(struct pseudo_inode), 1, fs_file) == 1;
}

/**
 * @brief Inicializuje prázdný i-uzel.
 */
static void init_inode(struct pseudo_inode *node, int32_t nodeid, bool is_dir) {
    memset(node, 0, sizeof(struct pseudo_inode));
    node->nodeid = nodeid;
    node->isDirectory = is_dir;
    node->references = 1;
    node->file_size = 0;
}

/* =============================================================================
 * POMOCNÉ FUNKCE - VÝPOČTY STRUKTURY FS
 * ============================================================================= */

/**
 * @brief Vypočítá rozložení souborového systému.
 */
typedef struct {
    int32_t cluster_count;
    int32_t inode_count;
    long bitmap_inode_size;
    long bitmap_data_size;
    long inode_table_size;
    int32_t bitmap_inode_start;
    int32_t bitmap_data_start;
    int32_t inode_table_start;
    int32_t data_block_start;
} fs_layout_t;

static fs_layout_t calculate_fs_layout(int64_t disk_size, int32_t cluster_size) {
    fs_layout_t layout;
    
    layout.cluster_count = disk_size / cluster_size;
    layout.inode_count = layout.cluster_count / 4;
    
    layout.bitmap_inode_size = (layout.inode_count / 8) + 1;
    layout.bitmap_data_size = (layout.cluster_count / 8) + 1;
    layout.inode_table_size = layout.inode_count * sizeof(struct pseudo_inode);
    
    layout.bitmap_inode_start = sizeof(struct superblock);
    layout.bitmap_data_start = layout.bitmap_inode_start + layout.bitmap_inode_size;
    layout.inode_table_start = layout.bitmap_data_start + layout.bitmap_data_size;
    layout.data_block_start = layout.inode_table_start + layout.inode_table_size;
    
    return layout;
}

/* =============================================================================
 * VYTVOŘENÍ KOŘENOVÉHO ADRESÁŘE
 * ============================================================================= */

/**
 * @brief Vytvoří kořenový adresář (i-uzel 0 + datový blok 0).
 */
static bool create_root_directory() {
    // Vytvoř i-uzel pro kořen
    struct pseudo_inode root_inode;
    init_inode(&root_inode, 0, true);
    root_inode.file_size = 2 * sizeof(struct directory_item);
    root_inode.direct1 = 0;
    
    // Zapiš i-uzel
    if (!fs_write_inode(0, &root_inode)) {
        return false;
    }
    
    // Vytvoř obsah adresáře ('.' a '..')
    struct directory_item root_content[2];
    root_content[0].inode = 0;
    strncpy(root_content[0].item_name, ".", 12);
    
    root_content[1].inode = 0;
    strncpy(root_content[1].item_name, "..", 12);
    
    // Zapiš obsah do prvního datového bloku
    fseek(fs_file, sb.data_start_address, SEEK_SET);
    if (fwrite(&root_content, sizeof(root_content), 1, fs_file) != 1) {
        return false;
    }
    
    // Označ i-uzel 0 a datový blok 0 jako obsazené
    if (!set_bitmap_bit(sb.bitmapi_start_address, 0, true)) {
        return false;
    }
    if (!set_bitmap_bit(sb.bitmap_start_address, 0, true)) {
        return false;
    }
    
    return true;
}

/* =============================================================================
 * INICIALIZACE A FORMÁTOVÁNÍ
 * ============================================================================= */

/**
 * @brief Naformátuje souborový systém.
 */
void handle_format(int argc, char **argv) {
    if (argc < 2) {
        printf("CHYBA: Chybí velikost disku (např. format 600M)\n");
        return;
    }
    
    // Parsuj velikost
    int64_t disk_size = parse_size_to_bytes(argv[1]);
    if (disk_size <= 0) {
        printf("CHYBA: Neplatná velikost disku.\n");
        return;
    }
    
    printf("Formátování na %lld bajtů...\n", disk_size);
    
    // Natáhni soubor
    if (!resize_file(disk_size)) {
        printf("CHYBA: Nelze změnit velikost souboru.\n");
        return;
    }
    
    // Vypočítej rozložení
    fs_layout_t layout = calculate_fs_layout(disk_size, CLUSTER_SIZE);
    
    // Vytvoř superblock
    struct superblock new_sb;
    memset(&new_sb, 0, sizeof(struct superblock));
    snprintf(new_sb.signature, sizeof(new_sb.signature), "%s", SIGNATURE);
    snprintf(new_sb.volume_descriptor, sizeof(new_sb.volume_descriptor), "%s", DESCRIPTOR);
    new_sb.disk_size = disk_size;
    new_sb.cluster_size = CLUSTER_SIZE;
    new_sb.cluster_count = layout.cluster_count;
    new_sb.bitmapi_start_address = layout.bitmap_inode_start;
    new_sb.bitmap_start_address = layout.bitmap_data_start;
    new_sb.inode_start_address = layout.inode_table_start;
    new_sb.data_start_address = layout.data_block_start;
    
    // Zapiš superblock
    fseek(fs_file, 0, SEEK_SET);
    if (fwrite(&new_sb, sizeof(struct superblock), 1, fs_file) != 1) {
        printf("CHYBA: Nelze zapsat superblock.\n");
        return;
    }
    
    // Vynuluj bitmapy
    if (!zero_disk_area(new_sb.bitmapi_start_address, layout.bitmap_inode_size)) {
        printf("CHYBA: Nelze vynulovat bitmapu i-uzlů.\n");
        return;
    }
    if (!zero_disk_area(new_sb.bitmap_start_address, layout.bitmap_data_size)) {
        printf("CHYBA: Nelze vynulovat bitmapu dat.\n");
        return;
    }
    
    // Aktualizuj globální superblock
    sb = new_sb;
    
    // Vytvoř kořenový adresář
    if (!create_root_directory()) {
        printf("CHYBA: Nelze vytvořit kořenový adresář.\n");
        return;
    }
    
    current_dir_inode_num = 0;
    printf("OK\n");
}

/**
 * @brief Inicializuje souborový systém.
 */
int fs_init(const char *filename) {
    fs_file = fopen(filename, "r+b");
    
    if (fs_file == NULL) {
        fs_file = fopen(filename, "w+b");
        if (fs_file == NULL) {
            perror("CHYBA: Nelze otevřít ani vytvořit soubor FS");
            return -1;
        }
        
        printf("Soubor '%s' vytvořen. Pro použití ho naformátujte.\n", filename);
        return 0;
    }
    
    // Načti superblock
    fseek(fs_file, 0, SEEK_SET);
    if (fread(&sb, sizeof(struct superblock), 1, fs_file) != 1) {
        printf("Soubor '%s' je poškozený. Před použitím ho naformátujte.\n", filename);
        memset(&sb, 0, sizeof(struct superblock));
    } else {
        printf("Souborový systém '%s' úspěšně načten.\n", filename);
    }
    
    current_dir_inode_num = 0;
    return 0;
}

/**
 * @brief Ukončí práci se souborovým systémem.
 */
void fs_shutdown() {
    if (fs_file != NULL) {
        fclose(fs_file);
        fs_file = NULL;
        printf("Souborový systém uzavřen.\n");
    }
}

/* =============================================================================
 * PŘÍKAZY PRO MANIPULACI SE SOUBORY A ADRESÁŘI
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
 * PŘÍKAZY PRO NAVIGACI A VÝPIS
 * ============================================================================= */
/**
 * @brief Najde položku podle jména v daném adresáři.
 * ZJEDNODUŠENÁ VERZE: Prohledává pouze první datový blok adresáře.
 *
 * @param dir_inode_num Číslo i-uzlu adresáře, ve kterém se má hledat.
 * @param item_name Jméno souboru/adresáře, který hledáme.
 * @return Číslo i-uzlu nalezené položky, nebo -1 pokud nenalezeno.
 */
int32_t fs_find_item_in_dir(int32_t dir_inode_num, const char *item_name) {
    // 1. Načteme i-uzel adresáře, ve kterém hledáme
    struct pseudo_inode dir_node = fs_read_inode(dir_inode_num);

    // 2. Zkontrolujeme, jestli je to vůbec adresář
    if (dir_node.isDirectory == false) {
        return -1; // Není to adresář, nemůžeme v něm hledat
    }

    // 3. Vytvoříme si buffer o velikosti jednoho clusteru
    // Ujisti se, že 'cluster_size' je dostupná (např. ze 'sb.cluster_size')
    char *buffer = malloc(sb.cluster_size);
    if (buffer == NULL) {
        printf("CHYBA: Nelze alokovat paměť.\n");
        return -1;
    }

    // 4. Načteme první datový blok tohoto adresáře (kde jsou položky)
    int32_t data_block_num = dir_node.direct1; // Zjednodušení!
    long data_address = sb.data_start_address + (data_block_num * sb.cluster_size);
    fseek(fs_file, data_address, SEEK_SET);
    fread(buffer, sb.cluster_size, 1, fs_file);

    // 5. Projdeme položky v načteném bloku
    struct directory_item *items = (struct directory_item *)buffer;
    int total_items = dir_node.file_size / sizeof(struct directory_item);
    
    int32_t found_inode = -1; // Nenalezeno
    for (int i = 0; i < total_items; i++) {
        if (strncmp(items[i].item_name, item_name, 12) == 0) {
            // Našli jsme shodu! Vrátíme číslo i-uzlu, na který ukazuje.
            found_inode = items[i].inode;
            break; // Ukončíme hledání
        }
    }

    // 6. Uvolníme paměť a vrátíme výsledek
    free(buffer);
    return found_inode;
}

void handle_ls(int argc, char **argv) {
    printf("handle_ls was called\n");
    
    int32_t target_inode_num = -1; // Číslo i-uzlu adresáře, který vypisujeme

    if (argc == 1) {
        // ----- Případ 1: "ls" (bez argumentů) -----
        // Chceme vypsat aktuální adresář
        target_inode_num = current_dir_inode_num;
        
    } else {
        // ----- Případ 2: "ls a1" (s argumentem) -----
        char *target_name = argv[1];
        
        // Zpracování absolutních cest (/a/b) by bylo složitější.
        // Tato verze předpokládá relativní cestu z aktuálního adresáře.
        
        // 1. Najdeme položku 'a1' v aktuálním adresáři
        target_inode_num = fs_find_item_in_dir(current_dir_inode_num, target_name);

        if (target_inode_num == -1) {
            printf("PATH NOT FOUND\n");
            return;
        }
    }

    // ----- Společná část pro oba případy -----
    // Teď, když máme v 'target_inode_num' i-uzel, který chceme vypsat:

    // 1. Načteme i-uzel cílového adresáře
    struct pseudo_inode target_dir_node = fs_read_inode(target_inode_num);

    // 2. Zkontrolujeme, jestli je to vůbec adresář
    if (target_dir_node.isDirectory == false) {
        printf("PATH NOT FOUND\n"); // 'ls' na soubor není povolen
        return;
    }

    // 3. Zjistíme počet položek
    int total_items = target_dir_node.file_size / sizeof(struct directory_item);
    
    // 4. Načteme první datový blok adresáře (zjednodušení!)
    int32_t data_block_num = target_dir_node.direct1;
    long data_address = sb.data_start_address + (data_block_num * sb.cluster_size);

    char *buffer = malloc(sb.cluster_size);
    if (buffer == NULL) {
        printf("CHYBA: Nelze alokovat paměť.\n");
        return;
    }
    
    fseek(fs_file, data_address, SEEK_SET);
    fread(buffer, sb.cluster_size, 1, fs_file);
    
    struct directory_item *items = (struct directory_item *)buffer;
    
    // 5. Projdeme položky a vypíšeme je
    for (int i = 0; i < total_items; i++) {
        // Musíme načíst i-uzel každé položky, abychom znali typ
        struct pseudo_inode item_node = fs_read_inode(items[i].inode);
        printf("%s: %s\n", item_node.isDirectory ? "DIR" : "FILE", items[i].item_name);
    }
    
    free(buffer);
    printf("OK\n");
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
 * PŘÍKAZY PRO PŘENOS SOUBORŮ
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
    printf("Signature: %s\n", sb.signature);
    printf("Volume Descriptor: %s\n", sb.volume_descriptor);
    printf("Disk Size: %d B\n", sb.disk_size);
    printf("Cluster Size: %d B\n", sb.cluster_size);
    printf("Cluster Count: %d\n", sb.cluster_count);
    printf("Bitmap Inode Start: %d\n", sb.bitmapi_start_address);
    printf("Bitmap Data Start: %d\n", sb.bitmap_start_address);
    printf("Inode Table Start: %d\n", sb.inode_start_address);
    printf("Data Blocks Start: %d\n", sb.data_start_address);
    printf("OK\n");
}