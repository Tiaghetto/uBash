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

//---------------AUX functions---------------
void init_empty_array(char **, int);
void free_dblptr(char **);
void free_mtrx(char ***);
//-----------------functions-----------------
char * getprompt();
char * getcommand();
char *** parser(char *);
size_t token_counter(const char const *, char);
bool double_occurrence(char*, char, size_t);
bool line_semantic_correctness(char *);
bool arg_semantic_correctness(char *, int);