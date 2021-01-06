#include "ubash.h"

int main(int argc, char *argv[]){
    char a[]={"ps aux -lhr | grep $ Andrefrox | okay all'right"};
    char*** to_parse = parser(a);
    int cmd_index = 0;

    if(to_parse != NULL){
        while(to_parse[cmd_index] != NULL){
            int arg_index = 0;
            printf("cmd %d\n", cmd_index);
            while(to_parse[cmd_index][arg_index] != NULL){
                printf("%s\n", to_parse[cmd_index][arg_index]);
                ++arg_index;
            }
            ++cmd_index;
        }
    }
    free_mtrx(to_parse);
}