#ifndef _H
#define STRUCTURES_H

#include <stdint.h>
#include <stdbool.h>

// Příklad datových struktur pro semestrální práci ZOS
// Jedná se o zjednodušenou verzi fs pro účely semestrální práce
// Verze 2025-01

// První i-uzel bude odkaz na hlavní adresář (1. datový blok)
// V reálném fs je více přímých odkazů a nepřímé odkazy 3. řádu
// Adresář vždy obsahuje dvojici číslo i-uzlu - název souboru
// Jde jen o příklad, vlastní datové struktury si můžete upravit

const int32_t ID_ITEM_FREE = 0;

/**
 * Superblok - hlavní struktura popisující celý filesystem
 */
struct superblock {
    char signature[9];              // Login autora FS
    char volume_descriptor[251];    // Popis vygenerovaného FS (cokoliv)
    int32_t disk_size;              // Celková velikost VFS
    int32_t cluster_size;           // Velikost clusteru
    int32_t cluster_count;          // Počet clusterů
    int32_t bitmapi_start_address;  // Adresa počátku bitmapy i-uzlů
    int32_t bitmap_start_address;   // Adresa počátku bitmapy datových bloků
    int32_t inode_start_address;    // Adresa počátku i-uzlů
    int32_t data_start_address;     // Adresa počátku datových bloků
};

/**
 * Pseudo i-uzel - struktura popisující soubor nebo adresář
 */
struct pseudo_inode {
    int32_t nodeid;                 // ID i-uzlu, pokud ID = ID_ITEM_FREE, je položka volná
    bool isDirectory;               // Soubor, nebo adresář
    int8_t references;              // Počet odkazů na i-uzel, používá se pro hardlinky
    int32_t file_size;              // Velikost souboru v bytech
    int32_t direct1;                // 1. přímý odkaz na datové bloky
    int32_t direct2;                // 2. přímý odkaz na datové bloky
    int32_t direct3;                // 3. přímý odkaz na datové bloky
    int32_t direct4;                // 4. přímý odkaz na datové bloky
    int32_t direct5;                // 5. přímý odkaz na datové bloky
    int32_t indirect1;              // 1. nepřímý odkaz (odkaz -> datové bloky)
    int32_t indirect2;              // 2. nepřímý odkaz (odkaz -> odkaz -> datové bloky)
};

/**
 * Položka adresáře - obsahuje referenci na i-uzel a název souboru/adresáře
 */
struct directory_item {
    int32_t inode;                  // Inode odpovídající souboru
    char item_name[12];             // 8+3 + \0 C/C++ ukončovací string znak
};

#endif // STRUCTURES_H
