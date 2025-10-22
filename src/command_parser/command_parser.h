#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

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

#endif
