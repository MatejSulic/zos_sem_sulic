#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include "../filesystem.h"

#define MAX_TOKENS 10
#define MAX_COMMAND_LENGTH 256

// Struktura s tokeny
typedef struct {
    char *tokens[MAX_TOKENS];
    int count;
} CommandTokens;

// Rozdělí příkaz na tokeny
CommandTokens parse_command(char *input);

void print_command_tokens(CommandTokens tokens);


typedef void (*CommandHandler)(int argc, char **argv);

typedef struct {
    const char *name;       // Název příkazu (např. "cp")
    int required_args;      // Počet povinných argumentů (např. 2)
    CommandHandler handler; // Ukazatel na funkci, která příkaz vykoná
} CommandEntry;

void execute_command(int token_count, char **tokens);



#endif
