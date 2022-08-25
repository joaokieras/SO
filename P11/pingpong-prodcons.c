// GRR20190379 João Pedro Kieras Oliveira
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ppos.h"
#include "queue.h"

#define TAM_BUFFER 5

int item = 0;
int timeSleep = 1000;
int queueItems[TAM_BUFFER] = {};
task_t p1, p2, p3, c1, c2;
semaphore_t s_item, s_buffer, s_vaga;

void insere_item_buffer(int i);
int retira_item_buffer();
void print_fila();

void taskProd(void *arg){
    while(1){
        task_sleep(timeSleep);
        item = (random () % 99);
        sem_down(&s_vaga);
        sem_down(&s_buffer);
        insere_item_buffer(item);
        printf("%s %d\n", (char*) arg, item);
        sem_up(&s_buffer);
        sem_up(&s_item);
    }
    task_exit(0);
}

void taskCons(void *arg){
    int item_retirado = 0;
    while(1){
        sem_down(&s_item);
        sem_down(&s_buffer);
        item_retirado = retira_item_buffer();
        sem_up(&s_buffer);
        sem_up(&s_vaga);
        printf("%s %d\n", (char*) arg, item_retirado);
        task_sleep(timeSleep);
    }
    task_exit(0);
}

int main(int argc, char *argv[]){
    printf("main: inicio\n");
    ppos_init();

    // Cria semáforos
    sem_create(&s_item, 0);
    sem_create(&s_vaga, TAM_BUFFER);
    sem_create(&s_buffer, TAM_BUFFER);

    // Cria tarefas
    task_create(&p1, taskProd, "p1 produziu");
    task_create(&p2, taskProd, "p2 produziu");
    task_create(&p3, taskProd, "p3 produziu");
    task_create(&c1, taskCons, "                c1 consumiu");
    task_create(&c2, taskCons, "                c2 consumiu");

    task_join(&p3);

    // Destrói semáforos
    sem_destroy(&s_item);
    sem_destroy(&s_vaga);
    sem_destroy(&s_buffer);

    printf("main: fim\n");
    task_exit(0);
    exit(0);
}

void insere_item_buffer(int num){
    int i;
    for(i = 0;i < TAM_BUFFER;i++){
        if(queueItems[i] == 0){
            queueItems[i] = num;
            //print_fila();
            return;
        }
    }
       
    return;
}

int retira_item_buffer(){
    int num = queueItems[0];
    for(int i = 0;i < TAM_BUFFER - 1;i++){
        queueItems[i] = queueItems[i + 1];
    }
    queueItems[TAM_BUFFER - 1] = 0;
    return num;
}

void print_fila(){
    printf("Fila: ");
    for(int i = 0;i < TAM_BUFFER;i++)
        printf("%d ", queueItems[i]);
    printf("\n");
}