// GRR20190379 João Pedro Kieras Oliveira
#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"

int item = 0;
task_t p1, p2, p3, c1, c2;
semaphore_t s_item, s_buffer, s_vaga;

void taskProd(void *arg){

    return;
}

void taskCons(void *arg){

    return;
}

int main(int argc, char *argv[]){
    printf("main: inicio\n");
    ppos_init();

    task_create(&p1, taskProd, "p1 produziu");
    task_create(&p2, taskProd, "p2 produziu");
    task_create(&p3, taskProd, "p3 produziu");
    task_create(&c1, taskCons, "                c1 consumiu");
    task_create(&c2, taskCons, "                c2 consumiu");

    sem_create(&s_item, 0);
    sem_create(&s_vaga, 0);
    sem_create(&s_buffer, 0);

    sem_destroy(&s_item);
    sem_destroy(&s_vaga);
    sem_destroy(&s_buffer);

    printf("main:fim");
    task_exit(0);
    exit(0);
}

