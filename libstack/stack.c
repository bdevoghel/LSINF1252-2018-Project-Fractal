#include <stdlib.h>
#include "stack.h"

int stack_length(node_t *stack)
{
    int length = 0;
    node_t *temporary = stack; // crée une copie

    while(temporary != NULL){
        temporary = temporary->next; // avance dans la copie
        length++;
    }

    return length;
}

int stack_push(node_t **stack, fractal_t *fract)
{
    node_t *new_head= (node_t *) malloc(sizeof(node_t)); // alloue l'espace nécessaire pour la nouvelle tête
    if (new_head == NULL) { // allocation échouée
        return 1; // retourne 1 si erreur
    }
    // place le nouvel élément sur la pile
    new_head->next = *stack;
    new_head->fract = fract;
    *stack = new_head;

    return 0; // réussite
}

struct fractal *stack_pop(node_t **stack)
{
    node_t *head = *stack;
    fractal_t *head_value = head->fract; // extraire le pointeur de la fractale de la tête de la pile
    *stack = head->next; // assigner nouvelle tête de pile
    free(head); // libère la node dont la valeur à été extraite

    return head_value;
}

void stack_free(node_t *stack)
{
    while (stack != NULL) {
        node_t *head = stack;
        stack = head->next;
        free(head);
    }
    // TODO : aussi libérer la pile-même ?? free(stack);
}
