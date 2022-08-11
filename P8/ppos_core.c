// GRR20190379 João Pedro Kieras Oliveira
#include "ppos.h"
#include "queue.h"
#include <signal.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

#define DEBUG

void dispatcher();
void print_elem();
void task_setprio(task_t *task, int prio);
int task_getprio(task_t *task);
task_t *scheduler();
void taskAging(int id);
task_t *findNextTask();
void tratadorSinal();
unsigned int systime();
int task_join(task_t *task);

// Task Main, contador de IDs e  contador de userTask devem ser 
// declarados globalmente para podermos utilizá-los em qualquer função
// taskAtual será um ponteiro para a task executando no momento
// queueReady será a fila de tarefas prontas

task_t taskMain, *taskAtual, taskDispatcher, *queueReady, *queueSleeping;
int numID = 0, userTask = 0, contTimer = 20;
unsigned long timerSys = 0;
struct itimerval timer;
struct sigaction action;

void ppos_init()
{
    setvbuf(stdout, 0, _IONBF, 0);

    // Arruma ponteiros e ID da task Main
    taskMain.prev = NULL;
    taskMain.next = NULL;
    taskMain.id = numID;
    taskMain.status = EXECUTANDO;
    taskMain.preemptable = 1;
    queue_append((queue_t **) &queueReady, (queue_t*) &taskMain);
    userTask++;
    taskAtual = &taskMain;
    // Timer
    action.sa_handler = tratadorSinal;
    sigemptyset (&action.sa_mask);
    action.sa_flags = 0;
    if(sigaction (SIGALRM, &action, 0) < 0){
        perror ("Erro em sigaction: ") ;
        exit (1) ;
    }

    timer.it_value.tv_usec = 1000;      // primeiro disparo, em micro-segundos
    timer.it_interval.tv_usec = 1000;   // disparos subsequentes, em micro-segundos
    if(setitimer (ITIMER_REAL, &timer, 0) < 0){
        perror ("Erro em setitimer: ") ;
        exit (1) ;
    }

    task_create(&taskDispatcher, dispatcher, NULL);

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

    getcontext(&(task->context));
    task->context.uc_stack.ss_sp = stack;
    task->context.uc_stack.ss_size = STACKSIZE;
    task->context.uc_stack.ss_flags = 0;
    task->context.uc_link = 0;
    makecontext(&(task->context), (void *)(*start_func), 1, arg);

    task->next = NULL;
    task->prev = NULL;
    task->id = ++numID;
    task->status = PRONTA;
    task->pe = 0;
    task->pd = task->pe;
    task->preemptable = 0;
    task->timeExec = timerSys;
    task->timeProc = 0;
    task->numActivation = 0;

    // Na fila não devem ser inseridas as tasks Main nem Dispatcher
    if(task->id > 1){
        userTask++;
        task->preemptable = 1;
        queue_append((queue_t **) &queueReady, (queue_t*) task);
    }

    #ifdef DEBUG
    //printf("task_create: criada task com id: %d\n", task->id);
    //queue_print("Fila de tarefas", (queue_t*) queueReady, print_elem);
    #endif

    return 0;
}

void task_exit(int exit_code)
{
    #ifdef DEBUG
    //printf("task_exit: tarefa %d sendo encerrada\n", taskAtual->id);
    #endif

    task_t *aux = taskAtual;
    
    userTask--;
    aux->exitCode = exit_code;
    aux->status = TERMINADA;
    aux->timeExec = timerSys - aux->timeExec;

    while(aux->queueSuspended != NULL)
        task_resume(aux->queueSuspended, &aux->queueSuspended);

    printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", aux->id, aux->timeExec, aux->timeProc, aux->numActivation);

    if(aux == &taskDispatcher){
        free(taskDispatcher.context.uc_stack.ss_sp);
        taskAtual = &taskMain;
    } 
    else
        taskAtual = &taskDispatcher;

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
    taskAtual->status = PRONTA;
    taskDispatcher.numActivation++;
    task_switch(&taskDispatcher);
    return;
}

void dispatcher(){
    while(userTask > 0){
        task_t *nextTask = scheduler();
        if(nextTask != NULL){
            contTimer = 20;
            int tIni = timerSys;
            task_switch(nextTask);
            int tEnd = timerSys;
            nextTask->timeProc += tEnd - tIni;
            switch(nextTask->status){
                case PRONTA:
                    break; 
                case EXECUTANDO:
                    break;               
                case TERMINADA:
                    queue_remove((queue_t **) &queueReady, (queue_t *) nextTask);
                    free(nextTask->context.uc_stack.ss_sp);
                    break;
            }
        }
        #ifdef DEBUG
        //queue_print("Fila de tarefas", (queue_t*) queueReady, print_elem);
        #endif
    }
    task_exit(0);
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

void task_setprio(task_t *task, int prio){
    if(task != NULL){
        task->pe = prio;
        task->pd = task->pe;
    }
    else{
        taskAtual->pe = prio;
        taskAtual->pd = taskAtual->pe;
    }
    return;
}

int task_getprio(task_t *task){
    if(task != NULL)
        return task->pe;
    return taskAtual->pe;
}

task_t *scheduler(){
    task_t *aux;
    aux = findNextTask();
    taskAging(aux->id);
    aux->status = EXECUTANDO;
    aux->pd = aux->pe;
    aux->numActivation++;
    return aux;
}

task_t *findNextTask(){
    task_t *aux = queueReady;
    task_t *percorre = queueReady->next;
    while(aux != percorre){
        if(aux->pd > percorre->pd)
            aux = percorre;
        percorre = percorre->next;
    }
    return aux;
}

void taskAging(int id){
    task_t *aux = queueReady;
    int cont = queue_size((queue_t *) queueReady);
    while(cont > 0){
        if(aux->id != id)
            aux->pd--;
        aux = aux->next;
        cont--;
    }
    return;
}

void tratadorSinal(){
    timerSys++;
    contTimer--;
    if(taskAtual->preemptable == 1 && contTimer == 0)
        task_yield();
    return;
}

unsigned int systime(){
    return timerSys;
}

int task_join(task_t *task){
    if(task == NULL || task->status == TERMINADA)
        return -1;
    task_suspend(&(task->queueSuspended));
    return task->exitCode;
}

void task_suspend(task_t **queue)
{
    queue_remove((queue_t **) &queueReady, (queue_t*) taskAtual);
    taskAtual->status = SUSPENSA;
    queue_append((queue_t **) queue, (queue_t *) taskAtual);
    task_yield();
    return;
}

void task_resume(task_t *task, task_t **queue)
{
    if(*queue == NULL)
        return;
    queue_remove((queue_t **) queue, (queue_t*) task);
    task->status = PRONTA;
    queue_append((queue_t **) &queueReady, (queue_t *) task);
    return;
}