#include "ubash.h"

//modificabile
bool add_pipe(int * ptr){
    return ((ptr = malloc(2*sizeof(int))) != NULL);
}

//safe se sz corretta
//modificabile
void close_unused_pipes(int ** pipearr, int sz){
    if((sz < 0) || (pipearr == NULL))
        return;
    if(pipearr[sz] == NULL)           
        close_unused_pipes(pipearr, --sz);
    else{
        close(pipearr[sz][READ_END]);
        close(pipearr[sz][WRITE_END]);
        close_unused_pipes(pipearr, --sz);
    }
}

int mywait(pid_t * pid_arr, int pidarr_sz){
    int error_status = 0;
    int status = 0;
    for(int i = 0; i < pidarr_sz; ++i){
        if(waitpid(pid_arr[i], &status, 0) == -1)
            return FAIL;
		if(!WIFEXITED(status)){
            fprintf(stderr, "\nERRORE: il comando non è andato a buon fine. (exit-status: %d)\n",status);
            error_status = status;
        }
		if(WIFSIGNALED(status)){
            fprintf(stderr, "\nERRORE: il processo numero \"%d\" ha ricevuto un segnale ed è terminato. (exit-status: %d)\n", i+1, status);
            error_status = status;
        }
    }
    if(status != error_status)
        return error_status;
    else
        return status;
}