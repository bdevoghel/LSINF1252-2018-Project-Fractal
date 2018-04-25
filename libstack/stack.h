#ifndef _STACK_H
#define _STACK_H

#include <stdlib.h>
#include "../libfractal/fractal.h"

/// Interface représentant une pile (un stack) stockant des pointeurs de fractales
// utilisé comme buffer entre les différents producteurs et consommateurs

/*
 * structure d'une node, élément d'une pile
 */
typedef struct node {
    struct fractal *fract; // pointeur de fractale
    struct node *next; // prochain élément du stack
} node_t;

/*
 * stack_length: retourne la taille de la pile
 *
 * @stack: une pile
 * @return: taille de la pile
 */
int stack_length(struct node *stack);

/*
 * stack_push: ajoute une fractale sur une pile
 *
 * @stack: une pile
 * @value: fractale à mettre sur la pile
 * @return: 0 si pas d'erreur, 1 sinon
 */
int stack_push(struct node **stack, struct fractal *fract);

/*
 * stack_pop: récupère la dernière fractale de une pile
 *
 * @stack: une pile
 * @return: la dernière fractale mise sur la pile, NULL si pile vide
 */
struct fractal *stack_pop(struct node **stack);

/*
 * stack_free: libère tous les éléments de la pile
 *
 * @stack: une pile
 */
void stack_free(struct node *stack);

#endif
