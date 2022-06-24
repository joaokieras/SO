#include "ppos.h"
#include <stdio.h>
#include <stdlib.h>

void ppos_init(){
    setvbuf(stdout, 0, _IONBF, 0);
    return;
}

int task_create(task_t *task, void (*start_func)(void *), void *arg){
    return 0;
}

void task_exit(int exit_code) {
    return;
}

int task_switch(task_t *task) {
    return 0;
}

int task_id(){
    return 0;
}

void task_suspend(task_t **queue){
    return;
}

void task_resume(task_t *task, task_t **queue){
    return;
}