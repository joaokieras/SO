// GRR20190379 João Pedro Kieras Oliveira
#include "ppos.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

#define DEBUG

void scheduler();
void print_elem();

// Task Main e contador de IDs devem ser declarados globalmente
// para podermos utilizá-los em qualquer função
// taskAtual será um ponteiro para a task executando no momento

task_t taskMain, *taskAtual, taskDispatcher, *queueTask;
int numID = 0;

void ppos_init()
{
    setvbuf(stdout, 0, _IONBF, 0);

    // Arruma ponteiros e ID da task Main
    taskMain.prev = NULL;
    taskMain.next = NULL;
    taskMain.id = numID;
    taskMain.status = RODANDO;

    taskAtual = &taskMain;

    task_create(&taskDispatcher, scheduler, NULL);

    #ifdef DEBUG
    //printf("ppos_init: criada task main id: %d\n", taskMain.id);
    //printf("ppos_init: sistema inicializado\n");
    #endif

    return;
}

int task_create(task_t *task, void (*start_func)(void *), void *arg)
{
    char *stack;
    stack = malloc(STACKSIZE);
    if (stack == NULL)
        return -1;

    // Idêntico a contexts.c
    getcontext(&(task->context));
    task->context.uc_stack.ss_sp = stack;
    task->context.uc_stack.ss_size = STACKSIZE;
    task->context.uc_stack.ss_flags = 0;
    task->context.uc_link = 0;
    makecontext(&(task->context), (void *)(*start_func), 1, arg);

    // Arruma ponteiros e ID da nova task
    task->next = NULL;
    task->prev = NULL;
    task->id = ++numID;
    task->status = NOVA;
    queue_append((queue_t **) &queueTask, (queue_t*) task);
    //queue_print("Fila de tarefas", (queue_t*) queueTask, print_elem);

    #ifdef DEBUG
    printf("task_create: criada task com id: %d\n", task->id);
    #endif

    return 0;
}

void task_exit(int exit_code)
{
    #ifdef DEBUG
    //printf("task_exit: tarefa %d sendo encerrada\n", taskAtual->id);
    #endif

    task_t *aux = taskAtual;
    taskAtual = &taskMain;
    swapcontext(&(aux->context), &(taskAtual->context));
    return;
}

int task_switch(task_t *task)
{
    #ifdef DEBUG
    //printf("task_switch: mudando do contexto %d -> %d\n", taskAtual->id, task->id);
    #endif

    task_t *aux = taskAtual;
    taskAtual = task;
    swapcontext(&(aux->context), &(taskAtual->context));
    return 0;
}

int task_id()
{
    return taskAtual->id;
}

void task_yield(){

}

void scheduler(){

}

void print_elem (void *ptr)
{
   task_t *elem = ptr ;

   if (!elem)
      return ;

   elem->prev ? printf ("%d", elem->prev->id) : printf ("*") ;
   printf ("<%d>", elem->id) ;
   elem->next ? printf ("%d", elem->next->id) : printf ("*") ;
}

void task_suspend(task_t **queue)
{
    return;
}

void task_resume(task_t *task, task_t **queue)
{
    return;
}