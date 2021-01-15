#include "ubash.h"

bool psafety_ctrl(bool val, char *** mtrx, char ** redir_arr, char * error_msg){
    if(val){
        fprintf(stderr, "%s\n", error_msg);
        free_mtrx(mtrx);
        free_dblptr(redir_arr);
        return true;
    }
    return false;
}

bool xsafety_ctrl(bool val, int ** pipearr, int pipearr_sz, pid_t * pid_arr, char * error_msg){
    if(val){
        fprintf(stderr, "%s\n", error_msg);
        close_unused_pipes(pipearr, pipearr_sz);
        free_dblptr(pipearr);
        free(pid_arr);
        return true;
    }
    return false;
}