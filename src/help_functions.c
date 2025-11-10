#include "help_functions.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * @brief Převede string s velikostí (např. "600M", "1G") na počet bajtů.
 * @param size_str String s velikostí.
 * @return Počet bajtů jako int64_t, nebo -1 při chybě.
 */
int64_t parse_size_to_bytes(char *size_str) {
    if (size_str == NULL) {
        return -1; // Chyba: žádný vstup
    }

    char *endptr; // Ukazatel na zbytek stringu po čísle
    long number_part = strtol(size_str, &endptr, 10); // Převede "600M" na číslo 600

    if (number_part <= 0) {
        return -1; // Chyba: neplatné číslo
    }

    int64_t multiplier = 1; // Výchozí jsou bajty
    char unit = ' ';

    // Zjistíme, co je za číslem (K, M, G)
    if (endptr != NULL && *endptr != '\0') {
        unit = toupper(*endptr); // Převedeme na velké písmeno
    }

    switch (unit) {
        case 'K':
            multiplier = 1024;
            break;
        case 'M':
            multiplier = 1024 * 1024;
            break;
        case 'G':
            multiplier = 1024 * 1024 * 1024;
            break;
        case ' ':
        case '\0':
            multiplier = 1; // Byly zadány jen bajty
            break;
        default:
            return -1; // Chyba: neznámá jednotka
    }

    return number_part * multiplier;
}