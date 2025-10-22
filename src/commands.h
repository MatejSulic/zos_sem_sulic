
#ifndef COMMANDS_H
#define COMMANDS_H


typedef void (*CommandHandler)(int argc, char **argv);

typedef struct {
    const char *name;       // Název příkazu (např. "cp")
    int required_args;      // Počet povinných argumentů (např. 2)
    CommandHandler handler; // Ukazatel na funkci, která příkaz vykoná
} CommandEntry;

void execute_command(int token_count, char **tokens);


void handle_cp(int argc, char **argv);
void handle_mv(int argc, char **argv);
void handle_rm(int argc, char **argv);
void handle_mkdir(int argc, char **argv);
void handle_rmdir(int argc, char **argv);
void handle_cat(int argc, char **argv);
void handle_ls(int argc, char **argv);
void handle_cd(int argc, char **argv);
void handle_pwd(int argc, char **argv);
void handle_info(int argc, char **argv);
void handle_incp(int argc, char **argv);
void handle_outcp(int argc, char **argv);
void handle_load(int argc, char **argv);
void handle_format(int argc, char **argv);
void handle_statfs(int argc, char **argv);




#endif // COMMANDS_H