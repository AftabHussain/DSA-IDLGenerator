#include "stdio.h"
#include "stdlib.h"

int Global = 10;

typedef struct list {
// struct list* Next;
int Data;
}list;

void fun(int* X){
    (*X)+=Global;
}

 void accessF(struct list* L, void (*FP)(int*)){
    //  do{
         FP(&L->Data);
        //  L=L->Next;
    //  }while(L);
 }

 void passF(struct list* L){
      accessF (L,fun);
 }

int main(){
    struct list *X = malloc(sizeof(struct list));
    struct list *Y = alloca(sizeof(struct list));
    // passF(X);
}




