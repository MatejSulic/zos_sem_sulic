#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./command_parser/command_parser.h"
#include "./filesystem.h"

int main(int argc, char* argv[]) {
    char fs_name[MAX_FILENAME_LENGTH]; 
    char command[MAX_COMMAND_LENGTH];

   if(argc != 2) {
    printf("Error: Invalid arguments\n");
    return 1;
}

    strncpy(fs_name, argv[1], MAX_FILENAME_LENGTH - 1);
    fs_name[MAX_FILENAME_LENGTH - 1] = '\0';

    printf("Filesystem: %s\n", fs_name);
    if (fs_init(fs_name) != 0) {
        printf("Error: Could not initialize filesystem\n");
        return 1;
    }


    for (;;) {
        printf("> ");
        if (fgets(command, sizeof(command), stdin) == NULL)
            break;

        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "exit") == 0)
            break;

        CommandTokens tokens = parse_command(command);
        execute_command(tokens.count, tokens.tokens);


    }
    fs_shutdown();

    return 0;
}
