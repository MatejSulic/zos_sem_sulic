#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

struct CommandInput {
    int argc;
    char* argv[10];
};

struct CommandInput create_command();
struct CommandInput parse_command(const char* command);
void free_command(struct CommandInput* cmd);

#endif