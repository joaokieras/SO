// GRR20190379 João Pedro Kieras Oliveira
#include "queue.h"
#include <stdio.h>

int queue_append(queue_t **queue, queue_t *elem){
    // Testa fila
    if(queue == NULL){
        fprintf(stderr, "###(Append) Erro: a fila não existe\n");
        return -1;
    }
    // Testa elemento
    if(elem == NULL){
        fprintf(stderr, "###(Append) Erro: o elemento não existe\n");
        return -1;
    }
    // Testa elemento em outra fila
    if(elem->prev != NULL || elem->next != NULL){
        fprintf(stderr, "###(Append) Erro: o elemento já existe em outra fila\n");
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
        queue_t *aux = (*queue)->prev;
        aux->next = elem;
        elem->next = *queue;
        elem->prev = aux;
        (*queue)->prev = elem;
        return 0;
    }
    return -1;
}

int queue_remove(queue_t **queue, queue_t *elem){
    if(queue == NULL){
        fprintf(stderr, "###(Remove) Erro: a fila não existe\n");
        return -1;
    }
    // Testa fila vazia
    if(*queue == NULL){
        fprintf(stderr, "###(Remove) Erro: a fila está vazia\n");
        return -1;
    }

    if(elem == NULL){
        fprintf(stderr, "###(Remove) Erro: o elemento não existe\n");
        return -1;
    }
    // Testa elemento nulo
    if(elem->prev == NULL || elem->next == NULL){
        fprintf(stderr, "###(Remove) Erro: o elemento não pertence a nenhuma fila\n");
        return -1;
    }

    // Testa se elemento existe na fila
    queue_t *aux = (*queue)->next;
    int pertence = 0;
    while(!pertence && aux != *queue){
        if(elem == aux){
            pertence = 1;
            break;
        }
        aux = aux->next;
    } 
    if(aux == elem) // "último" elemento, no caso o primeiro da fila, já que a iteração começa em queue->next
        pertence = 1;

    if(!pertence){
       fprintf(stderr, "###(Remove) Erro: o elemento não pertence a fila\n");
        return -1; 
    }  
    // Fila com um único elemento
    if(queue_size(*queue) == 1){
        *queue = NULL;
        elem->prev = NULL;
        elem->next = NULL;
        return 0;
    }
    else{
        //Caso o elemento seja o primeiro da fila, o próximo será o novo primeiro elem.
        if(*queue == elem)
            *queue = elem->next;
        elem->next->prev = elem->prev;
        elem->prev->next = elem->next;
        elem->prev = NULL;
        elem->next = NULL;
        return 0;
    }
    return -1;
}

int queue_size(queue_t *queue){
    if(queue == NULL)
        return 0;
    
    int cont = 0;
    queue_t *aux = queue->next;
    while(aux != queue){
        cont++;
        aux = aux->next;
    }
    // Conta o primeiro elemento que não participou da iteração
    cont++;
    return cont;
}

void queue_print(char *name, queue_t *queue, void print_elem (void*) ){
    int tam = queue_size(queue);
    printf("%s: [", name);
    if(tam == 0){ // Fila vazia
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
    // "último" elemento
    if(tam != 0)
        print_elem(aux->prev);

    printf("]\n");
    return;
}
