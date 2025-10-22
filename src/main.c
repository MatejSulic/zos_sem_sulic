#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./command_parser/command_parser.h"

int main(int argc, char* argv[]) {
    char command[256];
    struct CommandInput cmd;
    char* filesystem_name;

    if (argc != 2) {
        printf("Invalid arguments\n");
        return 1;
    }

    filesystem_name = argv[1];
    printf("Filesystem name: %s\n", filesystem_name);

    for (;;) {
        printf("> ");
        if (fgets(command, sizeof(command), stdin) == NULL)
            break;

        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "exit") == 0)
            break;

        cmd = parse_command(command);
        

        
        free_command(&cmd);

    }

    return 0;
}
