#include "ubash.h"

int token_counter(const char const * str, char delim){
    int tok_num = 0;
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

bool double_occurrence(char* str, char occ, size_t str_sz){
    for(int i = 0; i < (str_sz - 1); ++i){
        if((str[i] == occ) && (str[i+1] == occ))
            return true;
    }
    return false;
}

bool line_semantic_error(char* line){
    if(line == NULL)                                            //questo controllo andrebbe fatto nel main
        return true;
    if(line[0] == '\0'){                                        //nel caso in cui non abbiamo inserito nessun comando, non stampiamo niente(vedere readline)
        return true;
    }
    size_t line_size = strlen(line);
    if(double_occurrence(line, '|', line_size)){
        fprintf(stderr, "%s\n", ERRORTYPE_LINE);
        return true; 
    } 
    if((line[0] == '|') || (line[line_size - 1] == '|')){
        fprintf(stderr, "%s\n", ERRORTYPE_LINE);
        return true;
    }
        
    return false;
}

// se restituisce false, RICORDIAMOCI di fare la mega FREE
bool arg_semantic_error(char* arg, int arg_index, char *** tofree1, char ** tofree2){
    switch(arg_index){
        case 0:
            //controllo argomenti == stringa vuota ( cioe ________| o |_|)
            if(arg == NULL){
                fprintf(stderr, "%s\n", ERRORTYPE_LINE);
                free_mtrx(tofree1);
                free_dblptr(tofree2);
                return true;
            }
        default:
            //controllo redirezioni senza file attaccati 
            if(strlen(arg) == 1){
                if((arg[0] == '<') || (arg[0] == '>')){
                    fprintf(stderr, "%s\n", ERRORTYPE_REDIR1);
                    free_mtrx(tofree1);
                    free_dblptr(tofree2);
                    return true;
                }
            }
    }
    return false;
}

char * envvar_filter(char * arg, size_t arg_sz){
    if((arg[0] == '$') && (arg_sz > 2)){
        char * env_var = getenv(arg+1);
        return env_var;
    }
    return arg;
}

char*** parser(char* line){
    if(line_semantic_error(line))
        return NULL;
    
    char*** mtrx = NULL;

    int cmds_num = token_counter(line, '|');
    size_t rowsto_alloc = cmds_num+1;
    if(psafety_ctrl(((mtrx=malloc(rowsto_alloc*sizeof(char*))) == NULL), NULL, NULL, ERRORTYPE_ALLOC))
        return NULL;
    char_empty_array(mtrx, rowsto_alloc);
    char* cmd;
    char* cmd_context;
    int cmd_index = 0;
    for(cmd=strtok_r(line, "|", &cmd_context); cmd!=NULL; cmd=strtok_r(NULL, "|", &cmd_context)){
        int args_num = token_counter(cmd, ' ');
        size_t colsto_alloc = args_num+1;
        if(psafety_ctrl(((mtrx[cmd_index]=malloc(colsto_alloc*sizeof(char**))) == NULL), mtrx, NULL, ERRORTYPE_ALLOC))
            return NULL;
        char_empty_array(mtrx[cmd_index], colsto_alloc);
        char** redir_arr = NULL;
        int redir_arr_sz = 0;
        int input_num = 0;
        int output_num = 0;
        char* arg;
        char* arg_context;
        int arg_index = 0;
        arg=strtok_r(cmd, " ", &arg_context);
        while(true){
            if((arg == NULL) && arg_index)
                break;
            if(arg_semantic_error(arg, arg_index, mtrx, redir_arr))
                return NULL;
            size_t arg_sz = strlen(arg) + 1;
            if((arg[0] == '<' ) || (arg[0] == '>' )){
                if(arg[0] == '<')
                    ++input_num;
                else
                    ++output_num;
                //controlli mancanti da fare nella exec_command
                if(psafety_ctrl(((input_num > 1) || (output_num > 1)), mtrx, redir_arr, ERRORTYPE_REDIR2))
                    return NULL;
                int torealloc = redir_arr_sz + 2;
                if(psafety_ctrl(((redir_arr=realloc(redir_arr, torealloc*sizeof(char*))) == NULL), mtrx, redir_arr, ERRORTYPE_ALLOC))
                    return NULL;
                redir_arr[++redir_arr_sz] = NULL;
                if(psafety_ctrl(((redir_arr[redir_arr_sz-1] = malloc(arg_sz)) == NULL), mtrx, redir_arr, ERRORTYPE_ALLOC))
                    return NULL;
                strncpy(redir_arr[redir_arr_sz-1], arg, arg_sz);
            }
            else{
                char * toadd;
                if(psafety_ctrl(((toadd=envvar_filter(arg, arg_sz))==NULL), mtrx, redir_arr, ERRORTYPE_ENVVAR))
                    return NULL;
                arg_sz = strlen(toadd)+1;
                if(psafety_ctrl(((mtrx[cmd_index][arg_index] = malloc(arg_sz)) == NULL), mtrx, redir_arr, ERRORTYPE_ALLOC))
                    return NULL;
                strncpy(mtrx[cmd_index][arg_index], toadd, arg_sz);
                ++arg_index;
            }
            arg=strtok_r(NULL, " ", &arg_context);
        }
        //controlli di correttezza per redirezioni
        if(cmds_num > 1){
            if(psafety_ctrl(((cmd_index == 0) && (output_num > 0)), mtrx, redir_arr, ERRORTYPE_REDIR3))
                return NULL;
            if(psafety_ctrl(((cmd_index == cmds_num-1) && (input_num > 0)), mtrx, redir_arr, ERRORTYPE_REDIR3))
                return NULL;
            if(psafety_ctrl(((cmd_index > 0) && (cmd_index < cmds_num-1) && ((input_num > 0) || (output_num > 0))), mtrx, redir_arr, ERRORTYPE_REDIR3))
                return NULL;
        }
        for(int i = 0; arg_index+i < args_num; ++i){
            size_t tocopy_sz = strlen(redir_arr[i]) + 1;
            if(psafety_ctrl(((mtrx[cmd_index][arg_index+i] = malloc(tocopy_sz*sizeof(char))) == NULL), mtrx, redir_arr, ERRORTYPE_ALLOC))
                return NULL;
            strncpy(mtrx[cmd_index][arg_index+i], redir_arr[i], tocopy_sz);
        }
        free_dblptr(redir_arr);
        ++cmd_index;
    }
    return mtrx;
}