#include "ubash.h"

int main(int argc, char *argv[]){
    char *** command = NULL;
    char * line;
    int status = 0;
    while(true){
        line = getcommand(status);
        command = parser(line);
        if(command == NULL)
            break;
        if((command[1] == NULL) && (command[0][0][0] == 'c') && (command[0][0][1] == 'd') && (command[0][0][2] == '\0'))
            status = change_directory(command[0]);
        else
            status = exec_command(command);
        free_mtrx(command);
    }
    exit(EXIT_SUCCESS);
}
