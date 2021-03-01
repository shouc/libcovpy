//
// Created by shou on 2/14/21.
//
#include "tracer.cpp"

int main(int argc){
start:
    while (1){
        int a;
        scanf("%d", &a);
        if (a == 0) return 0;
        if (a % 2== 1) break;
    }
    printf("ok");
    goto start;
}