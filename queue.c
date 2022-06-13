#include "queue.h"

int queue_append (queue_t **queue, queue_t *elem){
    // Verifica se queue/elem existem
    if(*queue == NULL)
        return -1;
    if(elem == NULL)
        return -1;
    // Verifica se não está em outra fila
    if(elem->prev != NULL || elem->next != NULL)
        return -1;
    // Insere no final da fila
    if(*queue == NULL){ // Primeira inserção
        *queue = elem;
        elem->prev = elem;
        elem->next = elem;
        return 0;
    }
    else { // Fila não-vazia
        queue_t *aux;
        aux = (*queue)->prev; // último elemento
        aux->next = elem;
        elem->next = *queue;
        elem->prev = aux;
        (*queue)->prev = elem;
        return 0;
    }
    return -1;
}

int queue_remove (queue_t **queue, queue_t *elem){
    // Verifica se queue/elem existem
    if(*queue == NULL)
        return -1;
    if(elem == NULL)
        return -1;
    // Verifica se não está em outra fila
    if(elem->prev != NULL || elem->next != NULL)
        return -1;
    // Fila não vazia
    if(queue == NULL)
        return -1;

    queue_t *aux = (*queue)->next;
    while(aux != *queue){
        if(aux == elem){
            if(aux->next == *queue){ // ultimo elemento da fila
                aux->prev->next = *queue;
                aux->next->prev = aux->prev;
                aux->next = NULL;
                aux->prev = NULL;
                return 0;
            }            
            else if(aux != *queue){ // caso genérico
                aux->prev->next = aux->next;
                aux->next->prev = aux->prev;
                aux->prev = NULL;
                aux->next = NULL;
                return 0;
            }
        }
        aux = aux->next;
    }
    if(aux == elem){ // inicio da fila
        *queue = aux->next;
        aux->prev->next = aux->next;
        aux->next->prev = aux->prev;
        aux->next = NULL;
        aux->prev = NULL;
        return 0;
    }
    return -1;
}

int queue_size (queue_t *queue){
    int cont = 0;
    queue_t *aux = queue->next;
    while(aux != queue){
        cont++;
        aux = aux->next;
    }
    cont++;
    return cont;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*) ){
    queue_t *aux = queue->next;
    while(aux != queue){
        print_elem(aux);
        aux = aux->next;
    }
    print_elem(aux);
    return;
}
