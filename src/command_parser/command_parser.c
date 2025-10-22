#include <stdio.h>
#include <string.h>
#include "command_parser.h"

CommandTokens parse_command(char *input) {
    CommandTokens result = { .count = 0 };

    char *token = strtok(input, " ");
    while (token != NULL && result.count < MAX_TOKENS) {
        result.tokens[result.count++] = token;
        token = strtok(NULL, " ");
    }

    return result;
}

void print_command_tokens(CommandTokens tokens) {
    for (int i = 0; i < tokens.count; i++) {
        printf("Token %d: %s\n", i, tokens.tokens[i]);
    }
}