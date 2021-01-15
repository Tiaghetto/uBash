#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>                    
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <stdbool.h>

//ERROR MESSAGES
#define ERRORTYPE_LINE "\nERRORE: uBash non ammette due \"|\" senza un comando tra di loro o una \"|\" all'inizio o alla fine di una linea!\n"
#define ERRORTYPE_REDIR1 "\nERRORE: uBash non ammette redirezioni che non siano attaccate al file al quale fanno riferimento. Es: >file (corretto), > file (sbagliato).\n"
#define ERRORTYPE_REDIR2 "\nERRORE: uBash non ammette 2 redirezioni dello stesso tipo per un solo comando. Es: grep a.out <file1 <file2 (sbagliato)\n"
#define ERRORTYPE_REDIR3 "\nERRORE: se una linea è composta da più comandi(separati da \"|\"), soltanto il primo può redirigere l'input e soltanto l'ultimo può redirigere l'output\n"
#define ERRORTYPE_ALLOC "\nERRORE: errore nell'allocazione della memoria.\n"
#define ERRORTYPE_ENVVAR "\nERRORE: una o più delle variabili d'ambiente che hai scritto non esiste.\n"
#define ERRORTYPE_CD "\nERRORE: il comando cd dev'essere usato con un unico argomento!\n"

//PIPE ENDS
#define READ_END 0
#define WRITE_END 1

//EXIT STATUS
#define SUCCESS 0
#define FAIL -1

//---------------ERROR HANDLING functions---------------
bool psafety_ctrl(bool, char ***, char **, char *);
bool xsafety_ctrl(bool, int **, int, pid_t *, char *);
//---------------MEMORY HANDLING functions--------------
void int_empty_array(int **, int);
void char_empty_array(char **, int);
void free_dblptr(char **);
void free_mtrx(char ***);
//----------------FILE HANDLING functions---------------
bool add_pipe(int *);
void close_unused_pipes(int **, int);
int mywait(pid_t *, int);
//-------------------PARSING functions------------------
int token_counter(const char const *, char);
bool double_occurrence(char*, char, size_t);
bool line_semantic_error(char *);
bool arg_semantic_error(char *, int, char ***, char **);
char * envvar_filter(char *, size_t);
char *** parser(char *);
//-------------------COMMANDS functions-----------------
int cmds_num(char *** mtrx);
char ** getfirst_redir(char **);
char * getprompt(int);
char * getcommand(int);
int exec_command(char ***);
int change_directory(char **);
