#include "queue.h"
#include <stdio.h>

int queue_append(queue_t **queue, queue_t *elem){
    // Testa fila
    if(queue == NULL){
        fprintf(stderr, "### Error: queue doesn't exists\n");
        return -1;
    }
    // Testa elemento
    if(elem == NULL){
        fprintf(stderr, "### Error: element doesn't exists\n");
        return -1;
    }
    // Testa elemento em outra fila
    if(elem->prev != NULL || elem->next != NULL){
        fprintf(stderr, "### Error: element alredy exists in another queue\n");
        return -1;
    }
    // Fila vazia 
    if(*queue == NULL){ 
        *queue = elem;
        (*queue)->prev = elem;
        (*queue)->next = elem;
        return 0;
    }
    else{ // Fila não vazia - insere no final 
        queue_t *last = (*queue)->prev;
        last->next = elem;
        elem->next = *queue;
        elem->prev = last;
        (*queue)->prev = elem;
        return 0;
    }
    return -1;
}

int queue_remove(queue_t **queue, queue_t *elem){
    if(queue == NULL){
        fprintf(stderr, "### Error: queue doesn't exists\n");
        return -1;
    }
    // Testa fila vazia
    if(*queue == NULL){
        fprintf(stderr, "### Error: queue is empty\n");
        return -1;
    }

    if(elem == NULL){
        fprintf(stderr, "### Error: element doesn't exists\n");
        return -1;
    }

    if(elem->prev == NULL || elem->next == NULL){
        fprintf(stderr, "### Error: element doesn't belong to any queue\n");
        return -1;
    }

    // Testa se elemento existe na fila
    queue_t *aux = (*queue)->next;
    int pertence = 0;
    while(!pertence && aux != *queue){
        if(elem == aux){
            pertence = 1;
            aux = aux->prev;
        }
        aux = aux->next;
    } 
    // "último" elemento, no caso o primeiro da fila, já que a iteração começa em queue->next
    if(aux == elem)
        pertence = 1;

    if(!pertence){
       fprintf(stderr, "### Error: element doesn't belong to the queue\n");
        return -1; 
    }  
    // Único elemento
    if(queue_size(*queue) == 1)
        *queue = NULL;
    else{
        // Primeiro elemento
        if(*queue == elem)
            *queue = elem->next;

        elem->next->prev = elem->prev;
        elem->prev->next = elem->next;
    }

    elem->prev = NULL;
    elem->next = NULL;

    return 0;
}

int queue_size (queue_t *queue){
    int cont = 0;
    if(queue == NULL)
        return cont;
    
    queue_t *aux = queue->next;
    while(aux != queue){
        cont++;
        aux = aux->next;
    }
    cont++;
    return cont;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*) ){
    int tam = queue_size(queue);
    printf("%s [", name);
    // Fila vazia
    if(tam == 0){
        printf("]\n");
        return;
    }
    // A iteração começa em queue->next mas o print 
    // deve ser feito no anterior para ficar na ordem certa
    queue_t *aux = queue->next;
    while(aux != queue && tam != 0){
        print_elem(aux->prev);
        printf(" ");
        aux = aux->next;
        tam--;
    }
    // último elemento
    if(tam != 0)
        print_elem(aux->prev);

    printf("]\n");
}
