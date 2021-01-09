#include "ubash.h"

void int_empty_array(int ** arr, int dim){
    for(int i=0; i<dim; ++i){
        arr[i] = NULL;
    }
}

void char_empty_array(char ** arr, int dim){
    for(int i=0; i<dim; ++i){
        arr[i] = NULL;
    }
}

void free_dblptr(char ** arr){
    if(arr == NULL)
        return;
    int i = 0;
    while(arr[i] != NULL)
        free(arr[i++]);
    free(arr);
    arr = NULL;
}

void free_mtrx(char *** mtrx){
    if(mtrx == NULL)
        return;
    int i = 0;
    while(mtrx[i] != NULL){
        free_dblptr(mtrx[i++]);
    }
    free(mtrx);
    mtrx = NULL;
}

int cmds_num(char *** mtrx){
    if(mtrx == NULL)
        return 0;
    int i = 0;
    while(mtrx[i] != NULL)
        ++i;
    return i;
}

char ** getfirst_redir(char ** arr){
    int i = 0;
    while(arr[i] != NULL){
        if((arr[i][0] == '<') || (arr[i][0] == '>'))
            return arr+i;
        ++i;
    }
    return NULL;
}

bool add_pipe(int * ptr){
    return ((ptr = malloc(2*sizeof(int))) != NULL);
}

char * getprompt(){
    char *cwd = getcwd(NULL, 0);                // Importante: usando getcwd in questo modo devi ricordarti di fare la FREE!!!
    if(cwd == NULL){
        perror("Couldn't get cwd");
        return NULL;
    }
    return cwd;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------

char * getcommand(){
    char * prompt = getprompt();
    if(prompt == NULL){
        perror("Couldn't get cwd-prompt");
        return NULL;
    }
    printf("%s $ ", prompt);
    char * cmd = readline(NULL);
    free(prompt);
    return cmd;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------

char*** parser(char* line){
    if(!line_semantic_correctness(line))
        return NULL;
    
    char*** mtrx = NULL;

    size_t cmds_num = token_counter(line, '|');
    size_t rowsto_alloc = cmds_num+1;
    if((mtrx=malloc(rowsto_alloc*sizeof(char*))) == NULL){             //sizeof corretta???
        perror("");
        return NULL;
    }
    char_empty_array(mtrx, rowsto_alloc);
    char* cmd;
    char* cmd_context;
    int cmd_index = 0;
    for(cmd=strtok_r(line, "|", &cmd_context); cmd!=NULL; cmd=strtok_r(NULL, "|", &cmd_context)){
        size_t args_num = token_counter(cmd, ' ');
        size_t colsto_alloc = args_num+1;                                                                      //perche se no, mi allocava 40 bytes invece di 48 quando facevo (5+1)*sizeof(char*)
        if((mtrx[cmd_index]=malloc(colsto_alloc*sizeof(char**))) == NULL){
            perror("");
            free_mtrx(mtrx);
            return NULL;
        }
        char_empty_array(mtrx[cmd_index], colsto_alloc);
        char** toredirect = NULL;
        size_t toredirect_sz = 0;
        int input_num = 0;
        int output_num = 0;
        char* arg;
        char* arg_context;
        int arg_index = 0;
        arg=strtok_r(cmd, " ", &arg_context);
        while(true){
            if((arg == NULL) && arg_index)
                break;
            if(!arg_semantic_correctness(arg, arg_index)){
                perror("");
                free_dblptr(toredirect);
                free_mtrx(mtrx);
                return NULL;
            }
            size_t arg_sz = strlen(arg) + 1;
            if((arg[0] == '<' ) || (arg[0] == '>' )){
                if(arg[0] == '<')
                    ++input_num;
                else
                    ++output_num;
                if((input_num > 1) || (output_num > 1)){
                    //controlli mancanti da fare nella exec_command
                    perror("");
                    free_dblptr(toredirect);
                    free_mtrx(mtrx);
                    return NULL;
                }
                size_t torealloc = toredirect_sz + 2;
                if((toredirect=realloc(toredirect, torealloc*sizeof(char*))) == NULL){
                    perror("");
                    free_dblptr(toredirect);
                    free_mtrx(mtrx);
                    return NULL;
                }
                toredirect[++toredirect_sz] = NULL;
                if((toredirect[toredirect_sz-1] = malloc(arg_sz)) == NULL){
                    perror("");
                    free_dblptr(toredirect);
                    free_mtrx(mtrx);
                    return NULL;                                                 //o ptr?
                }
                strncpy(toredirect[toredirect_sz-1], arg, arg_sz);
            }
            else{
                char * env_var_alias;
                char * env_var_value;
                bool is_env_var = false;
                if((arg[0] == '$') && (arg_sz > 2)){
                    is_env_var = true;
                    env_var_alias = arg+1;
                    env_var_value = getenv(env_var_alias);
                    if(env_var_value == NULL){
                        perror("");
                        free_dblptr(toredirect);
                        free_mtrx(mtrx);
                        return NULL;
                    }
                    arg_sz = strlen(env_var_value)+1;
                }
                if((mtrx[cmd_index][arg_index] = malloc(arg_sz)) == NULL){
                    perror("");
                    free_dblptr(toredirect);
                    free_mtrx(mtrx);
                    return NULL;
                }
                if(is_env_var)
                    strncpy(mtrx[cmd_index][arg_index], env_var_value, arg_sz);
                else
                    strncpy(mtrx[cmd_index][arg_index], arg, arg_sz);
                ++arg_index;
            }
            arg=strtok_r(NULL, " ", &arg_context);
        }
        //per ultimo copiamo tutte le redirezioni in fondo alla riga usata
        for(int i = 0; arg_index+i < args_num; ++i){
            size_t tocopy_sz = strlen(toredirect[i]) + 1;
            if((mtrx[cmd_index][arg_index+i] = malloc(tocopy_sz*sizeof(char))) == NULL){
                perror("");
                free_dblptr(toredirect);
                free_mtrx(mtrx);
                return NULL;
            }
            strncpy(mtrx[cmd_index][arg_index+i], toredirect[i], tocopy_sz);
        }
        free_dblptr(toredirect);
        ++cmd_index;
    }
    return mtrx;
}

size_t token_counter(const char const * str, char delim){
    size_t tok_num = 0;
    size_t str_sz = strlen(str);
    bool increment = true;
    for(int i = 0; i < str_sz; ++i){
        if(str[i] == delim){
            increment = true;
            continue;
        }
        else if(increment == true){
            ++tok_num;
            increment = false;
        }
    }
    return tok_num;
}

bool double_occurrence(char* str, char occ, size_t line_sz){
    for(int i = 0; i < (line_sz - 1); ++i){
        if((str[i] == occ) && (str[i+1] == occ))
            return true;
    }
    return false;
}

bool line_semantic_correctness(char* line){
    if(line == NULL){
        perror("");
        return false;
    }
    if(line[0] == '\0'){
        perror("");
        return false;
    }

    size_t line_size = strlen(line);

    if(double_occurrence(line, '|', line_size)){
        perror("");
        return false;
    }
    
    if((line[0] == '|') || (line[line_size - 1] == '|')){
        perror("");
        return false;
    }

    return true;
}

// se restituisce false, RICORDIAMOCI di fare la mega FREE
bool arg_semantic_correctness(char* arg, int arg_index){
    switch(arg_index){
        case 0:
            //controllo argomenti == stringa vuota ( cioe ________| o |_|)
            if(arg == NULL){
                perror("");
                return false;
            }
        default:
            //controllo redirezioni senza file attaccati 
            if(strlen(arg) == 1){
                if((arg[0] == '<') || (arg[0] == '>')){
                    perror("");
                    return false;
                }
            }
    }
    return true;
}

void exec_command(char *** mtrx){
    if(mtrx == NULL){
        perror("");
        return;
    }
    int cmds;
    int ** pipearr;
    if((cmds=cmds_num(mtrx)) > 1){
        pipearr = malloc((cmds)*sizeof(int*));
        int_empty_array(pipearr, cmds);
    }
    pid_t pid;
    int row = 0;
    while(mtrx[row] != NULL){
        if((cmds > 1) && (mtrx[row+1] != NULL)){
            if((pipearr[row] = malloc(2*sizeof(int))) == NULL){
                perror("");
                //close e free delle pipe
                //free della matrice
                return;
            }
            if(pipe(pipearr[row])){
		        perror("arrivo");//errno
                //close e free delle pipe
                //free della matrice
		        return;
	        }
        }
        pid = fork();
	    if(pid == -1){
            perror("");
            return;
        }
	    if(pid == 0){
            if(cmds > 1){
                if(row == 0){
                    if(dup2(pipearr[row][WRITE_END], STDOUT_FILENO) == -1){
		                perror("arrivo");//errno
                        //close e free delle pipe
                        //free della matrice
		                return;
	                }
                    close(pipearr[row][READ_END]);
                    close(pipearr[row][WRITE_END]);
                }
                else if(mtrx[row+1] != NULL){
                    if(dup2(pipearr[row-1][READ_END], STDIN_FILENO) == -1){
		                perror("arrivo");//errno
                        //close e free delle pipe
                        //free della matrice
		                return;
	                }
                    close(pipearr[row-1][READ_END]);                                // OJO: hai gia fatto la close nel figlio precedente
                    close(pipearr[row-1][WRITE_END]);                               // quindi vediamo se funziona
                    if(dup2(pipearr[row][WRITE_END], STDOUT_FILENO) == -1){
		                perror("arrivo");//errno
                        //close e free delle pipe
                        //free della matrice
		                return;
	                }
                    close(pipearr[row][READ_END]);
                    close(pipearr[row][WRITE_END]);
                }
                else{
                    if(dup2(pipearr[row-1][READ_END], STDIN_FILENO) == -1){
		                perror("2arrivo");//errno
                        //close e free delle pipe
                        //free della matrice
		                return;
	                }
                    close(pipearr[row-1][READ_END]);
                    close(pipearr[row-1][WRITE_END]);
                }
            }
            char ** redir = getfirst_redir(mtrx[row]);
            int i = 0;
            if(redir != NULL){
                while(redir[i] != NULL){
                    int fd = open(redir[i]+1, O_CREAT|O_RDWR, 0666);
                    if(redir[i][0] == '<'){
                        dup2(fd, STDIN_FILENO);
                        close(fd);
                    }
                    else{
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                    }
                    free(redir[i]);
                    redir[i] = NULL;
                    ++i;
                }
                if(execvp(mtrx[row][0], mtrx[row]) == -1){
                    perror("");
                    return;
                }
            }
	    }
        ++row;
    }
    int status;
    if(cmds > 1){
        close(pipearr[0][WRITE_END]);
        close(pipearr[0][READ_END]);
    }
    //chiudere ciclicamente e freeare tutte le pipe?
	waitpid(pid, &status, 0);       //funziona cosi la wait o bisogna fare una per ogni figlio?
    //fare la free di matrix (qui o nel main)
}
