#include <stdlib.h>
#include <stdio.h>
#include "fractal.h"

struct fractal *fractal_new(const char *name, int width, int height, double a, double b)
{
    struct fractal *new_frac = (struct fractal *) malloc(sizeof(struct fractal)); // alloue l'espace nécessaire pour la fractale
    if(new_frac == NULL) { // allocation échouée
        return NULL; // retourne NULL si erreur
    }

    // assigner le nom
    new_frac->name = (char *) malloc(sizeof(char) * 64); // alloue l'espace nécessaire pour le nom, pas plus que 64 caractères
    if(new_frac->name == NULL) { // allocation échouée
        // libère ce qui a déjà été alloué
        free(new_frac);

        return NULL; // retourne NULL si erreur
    }
    int i;
    for(i = 0 ; name[i] != '\0'; i++) { // copie du nom donné en argument dans la structure de la fractale
        new_frac->name[i] = name[i];
    }
    new_frac->name[i] = '\0';

    // assigner les autres variables
    new_frac->a = a;
    new_frac->b = b;
    new_frac->width = width;
    new_frac->height = height;

    new_frac->values = (int *) calloc(width * height, sizeof(int)); // alloue l'espace nécessaire pour les valeurs des pixels et l'initialise à 0
    if(new_frac->values == NULL) { // allocation échouée
        // libère ce qui a déjà été alloué
        free(new_frac->name);
        free(new_frac);

        return NULL; // retourne NULL si erreur
    }

    new_frac->total_value = 0;
    new_frac->average = 0;

    return new_frac;
}

void fractal_free(struct fractal *f)
{
    // libère les valeurs allouées ...
    free(f->values);
    free(f->name);
    // ... avant de libérer la structure
    free(f);
}

const char *fractal_get_name(const struct fractal *f)
{
    return f->name;
}

int fractal_get_value(const struct fractal *f, int x, int y)
{
    if(x <= 0 && x > f->width && y <= 0 && y > f->height) {
        printf("Tried to get value of fractal \"%s\" outside of limits : (%i,%i) - Returning error code", fractal_get_name(f), x, y);
        return -1; // retourne -1 si erreur : (x,y) en dehors de l'image
    }
    return f->values[x + (y * f->width)];
}

void fractal_set_value(struct fractal *f, int x, int y, int val)
{
    if(x < 0 && x >= f->width && y < 0 && y >= f->height) {
        printf("Tried to set value for fractal \"%s\" outside of limits : (%i,%i) - Not doing anything", fractal_get_name(f), x, y);
        return;
    }

    // actualise la valeur totale de la fractale en fonction de ce qu'il y avait avant en (x,y)
    f->total_value += (val - f->values[x + (y * f->width)]);

    // assigne la bonne valeur au pixel
    f->values[x + (y * f->width)] = val;
}

int fractal_get_total_value(const struct fractal *f)
{
    return f->total_value;
}

double fractal_get_average(const struct fractal *f)
{
    return f->average;
}

void fractal_compute_average(struct fractal *f)
{
    f->average = f->total_value / (f->width * f->height);
}

int fractal_get_width(const struct fractal *f)
{
    return f->width;
}

int fractal_get_height(const struct fractal *f)
{
    return f->height;
}

double fractal_get_a(const struct fractal *f)
{
    return f->a;
}

double fractal_get_b(const struct fractal *f)
{
    return f->b;
}
