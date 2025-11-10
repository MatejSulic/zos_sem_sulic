
#ifndef HELP_FUNCTIONS_H
#define HELP_FUNCTIONS_H

#include <stdint.h>
/**
 * @brief Převede string s velikostí (např. "600M", "1G") na počet bajtů.
 * @param size_str String s velikostí.
 * @return Počet bajtů jako int64_t, nebo -1 při chybě.
 */
int64_t parse_size_to_bytes(char *size_str);

#endif // HELP_FUNCTIONS_H