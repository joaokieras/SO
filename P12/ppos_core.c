// GRR20190379 João Pedro Kieras Oliveira
#include "ppos.h"
#include "queue.h"
#include <signal.h>
#include <string.h>
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
void task_sleep(int t);
void awakeTasks();
int sem_create(semaphore_t *s, int value);
int sem_down(semaphore_t *s);
int sem_up(semaphore_t *s);
int sem_destroy(semaphore_t *s);
void enter_cs(int *lock);
void leave_cs(int *lock);
int mqueue_create(mqueue_t *queue, int max_msgs, int msg_size);
int mqueue_destroy(mqueue_t *queue);
int mqueue_send(mqueue_t *queue, void *msg);
int mqueue_recv(mqueue_t *queue, void *msg);
int mqueue_msgs(mqueue_t *queue);

// Task Main, contador de IDs e  contador de userTask devem ser 
// declarados globalmente para podermos utilizá-los em qualquer função
// taskAtual será um ponteiro para a task executando no momento
// queueReady será a fila de tarefas prontas

task_t taskMain, *taskAtual, taskDispatcher, *queueReady, *queueSleeping = NULL;
int numID = 0, userTask = 0, contTimer = 20, lock = 0;
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
        awakeTasks();
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
    if(queueReady == NULL)
        return NULL;

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

void awakeTasks() {
    if(queueSleeping == NULL)
        return;
    
    task_t *aux = queueSleeping;
    int cont = queue_size((queue_t *) queueSleeping);
    while(cont > 0){
        if(timerSys >= aux->timeSleep){
            task_resume(aux, &queueSleeping);
            aux = queueSleeping;
        }
        else
            aux = aux->next;
        cont--;
    }
    return;
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
    task->preemptable = 0;
    task_suspend(&(task->queueSuspended));
    task->preemptable = 1;
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
    if(queue == NULL || task == NULL)
        return;

    task->preemptable = 0;
    queue_remove((queue_t **) queue, (queue_t*) task);
    task->status = PRONTA;
    queue_append((queue_t **) &queueReady, (queue_t *) task);
    task->preemptable = 1;
    return;
}

void task_sleep(int t){
    taskAtual->preemptable = 0;
    queue_remove((queue_t **) &queueReady, (queue_t*) taskAtual);
    taskAtual->timeSleep = timerSys + t;
    queue_append((queue_t **) &queueSleeping, (queue_t *) taskAtual);
    taskAtual->preemptable = 1;
    task_yield();
}
void enter_cs(int *lock){
    // atomic OR (Intel macro for GCC)
    while (__sync_fetch_and_or (lock, 1)) ;   // busy waiting
}

void leave_cs(int *lock){
    (*lock) = 0;
}

int sem_create(semaphore_t *s, int value){
    if(s == NULL)
        return -1;
    s->cont = value;
    s->queueSem = NULL;
    return 0;
}

int sem_down(semaphore_t *s){
    if(s == NULL)
        return -1;

    enter_cs(&lock);
    s->cont--;
    if(s->cont < 0){
        leave_cs(&lock);
        task_suspend(&(s->queueSem));
    }
    else
        leave_cs(&lock);

    return 0;
}

int sem_up(semaphore_t *s){
    enter_cs(&lock);
    s->cont++;
    task_t *primeiro = s->queueSem;
    task_resume(primeiro, &(s->queueSem));
    leave_cs(&lock);
    return 0;
}

int sem_destroy(semaphore_t *s){
    if(s == NULL)
        return -1;
    while(s->queueSem != NULL)
        sem_up(s);
    return 0;
}

int mqueue_create(mqueue_t *queue, int max_msgs, int msg_size){
    if(queue == NULL)
        return -1;
    queue->queueMsg = NULL;
    queue->maxSize = max_msgs;
    queue->msgSize = msg_size;
    queue->numMsgs = 0;
    queue->terminada = 0;

    sem_create(&(queue->sem_item), 0);
    sem_create(&(queue->sem_vaga), max_msgs);
    sem_create(&(queue->sem_queue), 1);
    return 0;
}

int mqueue_destroy(mqueue_t *queue){
    if(queue == NULL)
        return -1;

    mnodo_t *aux = queue->queueMsg;
    while(aux != NULL){
        queue_remove((queue_t **)&(queue->queueMsg), (queue_t *)aux);
        free(aux->msg);
        free(aux);
        aux = queue->queueMsg;
    }

    sem_destroy(&(queue->sem_item));
    sem_destroy(&(queue->sem_vaga));
    sem_destroy(&(queue->sem_queue));
    queue->terminada = 1;
    return 0;
}

int mqueue_send(mqueue_t *queue, void *msg){
    if(queue == NULL || msg == NULL)
        return -1;
    if(queue->terminada == 1)
        return -1;
    sem_down(&(queue->sem_vaga));
    sem_down(&(queue->sem_queue));

    // Insere na fila
    mnodo_t *aux = malloc(sizeof(mnodo_t));
    if(aux == NULL){
        printf("Erro na alocação\n");
        exit(1);
    }
    aux->msg = malloc(sizeof(msg));
    if(aux == NULL){
        printf("Erro na alocação\n");
        exit(1);
    }
    aux->prev = NULL;
    aux->next = NULL;
    memcpy(aux->msg, msg, queue->msgSize);
    queue_append((queue_t **)&(queue->queueMsg), (queue_t *)aux);
    queue->numMsgs++;

    sem_up(&(queue->sem_queue));
    sem_up(&(queue->sem_item));
    return 0;
}

int mqueue_recv(mqueue_t *queue, void *msg){
    if(queue == NULL || msg == NULL)
        return -1;
    if(queue->terminada == 1)
        return -1;
    sem_down(&(queue->sem_item));
    sem_down(&(queue->sem_queue));

    // Remove da fila
    if(queue->queueMsg == NULL)
        return -1;
    mnodo_t *first = queue->queueMsg;
    memcpy(msg, first->msg, queue->msgSize);
    queue_remove((queue_t **)&(queue->queueMsg), (queue_t *)first);
    queue->numMsgs--;
    free(first->msg);
    free(first);

    sem_up(&(queue->sem_queue));
    sem_up(&(queue->sem_vaga));
    return 0;
}

int mqueue_msgs(mqueue_t *queue){
    if(queue == NULL)
        return -1;
    return queue->numMsgs;
}