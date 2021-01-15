#include "ubash.h"

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

char * getprompt(int status){
    char *cwd = getcwd(NULL, 0);                // Importante: usando getcwd in questo modo devi ricordarti di fare la FREE!!!
    if(cwd == NULL){
        perror("Couldn't get cwd");             //OJO on error errno is set
        return NULL;
    }
    char * prompt;
    char status_str[20];
    size_t p_size = strlen(cwd)+23;             //\033[0;34m (7) + size cwd + \033[0m (4) + ' ' (1) + \033[0;34m (7) + '$' (1) + ' ' (1) + \033[0m (4) + ' ' (1) + '\0' (1)
    if(status != 0){
        p_size += 13;                           //\033[0;31m (7) + \033[0m (4) + ' ' (1)
        sprintf(status_str, "%d", status);
        p_size += strlen(status_str);           //size status_str
    }
    if((prompt=malloc(p_size*sizeof(char))) == NULL){
        fprintf(stderr, "%s (errore di allocazione ottenendo il prompt)\n", strerror(errno));
        return NULL;
    }
    memset(prompt, '\0', p_size);
    strcat(prompt, "\033[0;34m");
    strcat(prompt, cwd);
    strcat(prompt, "\033[0m ");
    if(status != 0){
        strcat(prompt, "\033[0;31m");
        strcat(prompt, status_str);
        strcat(prompt, "\033[0m ");
    }
    strcat(prompt, "\033[0;34m");
    strcat(prompt, "$ ");
    strcat(prompt, "\033[0m ");
    free(cwd);
    return prompt;
}

char * getcommand(int status){
    char * prompt = getprompt(status);
    if(prompt == NULL)
        return NULL;
    char * cmd = readline(prompt);
    free(prompt);
    return cmd;
}

int exec_command(char *** mtrx){
    int cmds;
    int ** pipearr = NULL;
    if((cmds=cmds_num(mtrx)) > 1){
        if(xsafety_ctrl(((pipearr = malloc(cmds*sizeof(int*))) == NULL), NULL, -1, NULL, ERRORTYPE_ALLOC))
            return SUCCESS;
        int_empty_array(pipearr, cmds);
    }
    pid_t * pid_arr = NULL;
    if(xsafety_ctrl(((pid_arr = malloc(cmds*sizeof(pid_t))) == NULL), pipearr, cmds, NULL, ERRORTYPE_ALLOC))
        return SUCCESS;
    int index = 0;
    while(mtrx[index] != NULL){
        if((cmds > 1) && (mtrx[index+1] != NULL)){
            if(xsafety_ctrl(((pipearr[index] = malloc(2*sizeof(int))) == NULL), NULL, -1, NULL, ERRORTYPE_ALLOC))
                break;
            if(xsafety_ctrl((pipe(pipearr[index]) == -1), NULL, -1, NULL, strerror(errno)))
                break;
        }
        pid_arr[index] = fork();
        if(xsafety_ctrl((pid_arr[index] == -1), NULL, -1, NULL, strerror(errno)))               //ricordati che alla fine bisogna fare la close di pipearr e la free di pipearr/pid_arr
            break;
	    if(pid_arr[index] == 0){
            if(cmds > 1){
                if(index == 0){
                    close(pipearr[index][READ_END]);
                    if(dup2(pipearr[index][WRITE_END], STDOUT_FILENO) == -1)
		                return FAIL;
                    //do safe close
                    close(pipearr[index][WRITE_END]);
                }
                else if(mtrx[index+1] != NULL){
                    close_unused_pipes(pipearr, index-2);
                    close(pipearr[index-1][WRITE_END]);
                    if(dup2(pipearr[index-1][READ_END], STDIN_FILENO) == -1)
		                return FAIL;
                    close(pipearr[index-1][READ_END]);
                    close(pipearr[index][READ_END]);
                    if(dup2(pipearr[index][WRITE_END], STDOUT_FILENO) == -1)
		                return FAIL;
                    close(pipearr[index][WRITE_END]);
                }
                else{
                    close_unused_pipes(pipearr, index-2);
                    close(pipearr[index-1][WRITE_END]);
                    if(dup2(pipearr[index-1][READ_END], STDIN_FILENO) == -1)
		                return FAIL;
                    close(pipearr[index-1][READ_END]);
                }
            }
            char ** redir = getfirst_redir(mtrx[index]);
            int i = 0;
            if(redir != NULL){
                while(redir[i] != NULL){
                    int fd;
                    if(redir[i][0] == '<'){
                        if((fd = open(redir[i]+1, O_CREAT|O_RDONLY, 0777)) == -1)
                            return FAIL;
                        dup2(fd, STDIN_FILENO);
                        close(fd);
                    }
                    else{
                        if((fd = open(redir[i]+1, O_CREAT|O_WRONLY, 0777)) == -1)
                            return FAIL;
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                    }
                    free(redir[i]);
                    redir[i] = NULL;
                    ++i;
                }
            }
            if(execvp(mtrx[index][0], mtrx[index]) == -1)
                return FAIL;
	    }
        ++index;
    }
    if(cmds > 1)
        close_unused_pipes(pipearr, cmds-1);
    free_dblptr(pipearr);
    mywait(pid_arr, cmds);
    free(pid_arr);
}

//da modificare
int change_directory(char ** cmd){
    int rval = -1;
    if(cmd[1] != NULL){
        if(cmd[2] != NULL){
            fprintf(stderr, "%s\n", ERRORTYPE_CD);
            return rval;
        }
        if((cmd[1][0] != '<') && (cmd[1][0] != '>'))
            if(rval = chdir(cmd[1]) == -1)
                perror("");
        return rval;
    }
    if(rval = chdir("/home") == -1)
        perror("");
    return rval;
}
