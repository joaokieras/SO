// GRR20190379 João Pedro Kieras Oliveira
#include "ppos.h"
#include <stdio.h>
#include <stdlib.h>

#define DEBUG
// Task Main e contador de IDs devem ser declarados globalmente
// para podermos utilizá-los em qualquer função
task_t taskMain;
int numID = 0;

void ppos_init()
{
    setvbuf(stdout, 0, _IONBF, 0);
    // Arruma ponteiros e ID da task Main
    taskMain.prev = NULL;
    taskMain.next = NULL;
    taskMain.id = numID;

#ifdef DEBUG
    printf("ppos_init: criada task main id: %d\n", taskMain.id);
    printf("ppos_init: sistema inicializado\n");
#endif

    return;
}

int task_create(task_t *task, void (*start_func)(void *), void *arg)
{
    char *stack;
    stack = malloc(STACKSIZE);
    // Verifica se alocação funcionou
    if (stack == NULL)
        return -1;
    // Idêntico a contexts.c
    task->context.uc_stack.ss_sp = stack;
    task->context.uc_stack.ss_size = STACKSIZE;
    task->context.uc_stack.ss_flags = 0;
    task->context.uc_link = 0;
    makecontext(&(task->context), (void *)(*start_func), 1, arg);
    getcontext(&(task->context));
    // Arruma ponteiros e ID da nova task
    task->next = NULL;
    task->prev = NULL;
    task->id = ++numID;

#ifdef DEBUG
    printf("task_create: criada task com id: %d\n", task->id);
#endif

    return 0;
}

void task_exit(int exit_code)
{
    return;
}

int task_switch(task_t *task)
{
    return 0;
}

int task_id()
{
    return 0;
}

void task_suspend(task_t **queue)
{
    return;
}

void task_resume(task_t *task, task_t **queue)
{
    return;
}