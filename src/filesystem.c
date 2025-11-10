
#include "filesystem.h"
#include "help_functions.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>



FILE *fs_file = NULL;

struct superblock sb;

int32_t current_dir_inode_num = 0;

int64_t total_disk_size = 0;

int32_t cluster_size = CLUSTER_SIZE;

int32_t cluster_count = 0;





// Předpokládám, že máš někde nahoře definované globální proměnné:
// extern FILE *fs_file;
// extern struct superblock sb;
// extern int32_t current_dir_inode_num;
// A konstanty jako SIGNATURE, DESCRIPTOR, cluster_size...

void handle_format(int argc, char **argv) {
    printf("handle_format was called\n");

    // 1. Získáme string s velikostí (např. "600M")
    char *size_str = argv[1];

    // 2. Převedeme ho na reálný počet bajtů
    int64_t total_disk_size = parse_size_to_bytes(size_str);

    if (total_disk_size <= 0) {
        printf("Chyba: Neplatná velikost disku.\n");
        return;
    }
    
    printf("Formátování na %lld bajtů...\n", total_disk_size);
    
    // --- KROK A: NATÁHNUTÍ SOUBORU NA POŽADOVANOU VELIKOST ---
    // (Musí být před výpočty, pokud by fs_file byl globální)
    // Pokud je fs_file již otevřený z fs_init:
    fseek(fs_file, total_disk_size - 1, SEEK_SET);
    fwrite("\0", 1, 1, fs_file);
    printf("Soubor roztažen na požadovanou velikost.\n");


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

    // (Kontrolní výpisy máš dobře)
    // ...

    // 9. Vytvoříme superblok v lokální proměnné
    struct superblock new_sb;

    // Vyplníme superblok
    snprintf(new_sb.signature, sizeof(new_sb.signature), "%s", SIGNATURE);
    snprintf(new_sb.volume_descriptor, sizeof(new_sb.volume_descriptor), "%s", DESCRIPTOR);
    new_sb.disk_size = total_disk_size;
    new_sb.cluster_size = cluster_size;
    new_sb.cluster_count = cluster_count;
    // Tady bys měl přidat i inode_count, pokud ho máš ve struktuře
    // new_sb.inode_count = inode_count; 
    new_sb.bitmapi_start_address = bitmap_inode_start;
    new_sb.bitmap_start_address = bitmap_data_start;
    new_sb.inode_start_address = inode_table_start;
    new_sb.data_start_address = data_block_start;

    // --- KROK B: FYZICKÝ ZÁPIS SUPERBLOCKU NA DISK ---
    fseek(fs_file, 0, SEEK_SET);
    fwrite(&new_sb, sizeof(struct superblock), 1, fs_file);
    printf("Superblock zapsán na disk.\n");

    // --- KROK C: VYČIŠTĚNÍ (VYNULOVÁNÍ) BITMAP ---
    // Použijeme calloc, který alokuje paměť a rovnou ji vynuluje
    char *zero_buffer = calloc(bitmap_inode_size, 1);
    if (zero_buffer == NULL) { printf("Chyba alokace paměti pro bitmapu i-uzlů\n"); return; }
    fseek(fs_file, new_sb.bitmapi_start_address, SEEK_SET);
    fwrite(zero_buffer, bitmap_inode_size, 1, fs_file);
    free(zero_buffer); // Uvolníme paměť

    zero_buffer = calloc(bitmap_data_size, 1);
    if (zero_buffer == NULL) { printf("Chyba alokace paměti pro bitmapu dat\n"); return; }
    fseek(fs_file, new_sb.bitmap_start_address, SEEK_SET);
    fwrite(zero_buffer, bitmap_data_size, 1, fs_file);
    free(zero_buffer); // Uvolníme paměť
    printf("Bitmapy vynulovány.\n");

    // --- KROK D: VYTVOŘENÍ KOŘENOVÉHO ADRESÁŘE (INODE 0 + DATA 0) ---

    // 1. Vytvoříme i-uzel pro kořen (v paměti)
    struct pseudo_inode root_inode;
    root_inode.nodeid = 0;
    root_inode.isDirectory = true;
    root_inode.references = 1;
    root_inode.file_size = 2 * sizeof(struct directory_item); // Bude obsahovat '.' a '..'
    root_inode.direct1 = 0; // Odkaz na první datový blok (blok č. 0)
    root_inode.direct2 = 0; // 0 znamená nevyužito
    root_inode.direct3 = 0;
    root_inode.direct4 = 0;
    root_inode.direct5 = 0;
    root_inode.indirect1 = 0;
    root_inode.indirect2 = 0;

    // 2. Zápis i-uzlu na disk (na začátek tabulky i-uzlů)
    fseek(fs_file, new_sb.inode_start_address, SEEK_SET);
    fwrite(&root_inode, sizeof(struct pseudo_inode), 1, fs_file);

    // 3. Vytvoření obsahu kořenového adresáře ('.' a '..')
    struct directory_item root_content[2];
    root_content[0].inode = 0; // '.' ukazuje sám na sebe
    strncpy(root_content[0].item_name, ".", 12);
    
    root_content[1].inode = 0; // '..' v kořeni ukazuje taky na sebe
    strncpy(root_content[1].item_name, "..", 12);

    // 4. Zápis obsahu do prvního datového bloku
    fseek(fs_file, new_sb.data_start_address, SEEK_SET);
    fwrite(&root_content, sizeof(root_content), 1, fs_file);
    printf("Kořenový adresář (inode 0 + data 0) vytvořen.\n");

    // --- KROK E: AKTUALIZACE BITMAP (OZNAČENÍ PRVNÍCH BITŮ) ---
    // Označíme i-uzel 0 a datový blok 0 jako obsazené
    char first_bit_set = 0x80; // Bajt, kde je první bit 1 (10000000)

    // Zápis do bitmapy i-uzlů
    fseek(fs_file, new_sb.bitmapi_start_address, SEEK_SET);
    fwrite(&first_bit_set, 1, 1, fs_file);

    // Zápis do bitmapy datových bloků
    fseek(fs_file, new_sb.bitmap_start_address, SEEK_SET);
    fwrite(&first_bit_set, 1, 1, fs_file);
    printf("Bitmapy aktualizovány (obsazeno 0. i-uzel a 0. datový blok).\n");

    // --- KROK F: FINALIZACE ---
    // Zkopírujeme lokální new_sb do globální 'sb' (náš "hlavní zápisník")
    sb = new_sb; 
    current_dir_inode_num = 0; // Jsme v kořeni

    printf("OK\n");
}
/**
 * @brief Inicializuje souborový systém.
 * Otevře soubor a načte Superblock do globální proměnné 'sb'.
 * @param filename Cesta k souboru s FS (např. "muj_fs.dat")
 * @return 0 při úspěchu, -1 při chybě.
 */
int fs_init(const char *filename) {
    // 1. Otevři soubor pro čtení i zápis v binárním režimu
    fs_file = fopen(filename, "r+b");

    if (fs_file == NULL) {
        // Soubor neexistuje. Zkusíme ho vytvořit.
        fs_file = fopen(filename, "w+b");
        if (fs_file == NULL) {
            perror("CHYBA: Nelze otevřít ani vytvořit soubor FS");
            return -1;
        }
        
        // Soubor je nový, ale prázdný. 
        // Je potřeba ho naformátovat.
        printf("Soubor '%s' neexistuje nebo je prázdný. Bude vytvořen. Pro použití je nutné ho naformátovat.\n", filename);
        // Necháme 'sb' prázdný, 'format' ho naplní.
        return 0; // Vracíme úspěch, i když je prázdný
        
    } else {
        // Soubor existuje, načteme z něj Superblock
        fseek(fs_file, 0, SEEK_SET);
        int read_count = fread(&sb, sizeof(struct superblock), 1, fs_file);
        
        if (read_count != 1) {
            // Soubor je poškozený nebo prázdný
            printf("Soubor '%s' je poškozený. Před použitím ho naformátujte.\n", filename);
            // Můžeme starou 'sb' vynulovat
            memset(&sb, 0, sizeof(struct superblock));
        } else {
            // Vše je v pořádku, Superblock je načten v 'sb'
            printf("Souborový systém '%s' úspěšně načten.\n", filename);
        }
    }

    current_dir_inode_num = 0; // Vždy začínáme v kořeni
    return 0;
}

/**
 * @brief Bezpečně zavře soubor FS.
 */
void fs_shutdown() {
    if (fs_file != NULL) {
        fclose(fs_file);
        fs_file = NULL;
        printf("Souborový systém uzavřen.\n");
    }
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

    // Jednoducho vytlačíme hodnoty z globálnej 'sb',
    // ktorú by mal načítať fs_init() hneď po spustení.
    
    printf("Signature: %s\n", sb.signature);
    printf("Volume Descriptor: %s\n", sb.volume_descriptor);
    printf("Disk Size: %d B\n", sb.disk_size);
    printf("Cluster Size: %d B\n", sb.cluster_size);
    printf("Cluster Count: %d\n", sb.cluster_count);
    
    // Tu by si ešte mohol pridať aj inode_count, ak ho máš v sb
    // printf("Inode Count: %d\n", sb.inode_count);

    printf("Bitmap Inode Start: %d\n", sb.bitmapi_start_address);
    printf("Bitmap Data Start: %d\n", sb.bitmap_start_address);
    printf("Inode Table Start: %d\n", sb.inode_start_address);
    printf("Data Blocks Start: %d\n", sb.data_start_address);

    // TODO: Neskôr môžeš pridať výpočet voľného miesta
    // (prejdením oboch bitmap a sčítaním voľných bitov)

    printf("OK\n");
}