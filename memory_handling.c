#include "ubash.h"

void int_empty_array(int** arr, int dim){
    if(arr != NULL)
        for(int i=0; i<dim; ++i)
            arr[i] = NULL;
}

void char_empty_array(char ** arr, int dim){
    if(arr != NULL)
        for(int i=0; i<dim; ++i)
            arr[i] = NULL;
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
    while(mtrx[i] != NULL)
        free_dblptr(mtrx[i++]);
    free(mtrx);
    mtrx = NULL;
}