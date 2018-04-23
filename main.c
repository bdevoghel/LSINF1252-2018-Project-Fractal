/**
 * Exemple d'utilisation :
 *      ./main [-d] [--maxthreads n] fichier1 fichierN [fichierOut]
 *      -d : afficher toutes les fractales calculées
 *      --maxthreads X : vouloir utiliser X threads de calcul, sinon 1 par défault
 *      fichier* : fichier à partir duquel extraire les infos des fractales à calculer
 *      fichierOut : fichier dans lequel sortir la fractale à la plus haute moyenne
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <pthread.h>
#include <semaphore.h>

#include "libstack/stack.h"
#include "libfractal/fractal.h"

/* VARIABLES GLOBALES */

int d_option = 0; // 1 si "-d" est présent dans les paramètres, 0 sinon
int mt_option = 0; // 1 si "--maxthread" est présent dans les paramètres, 0 sinon
int maxthreads = 1; // nombre de threads de calcul maximal, 1 par défault (valeur minimale)

int files_to_read; // nombre de fichiers à lire pour extraire les infos des fractales
char *file_out; // nom du fichier de sortie finale

pthread_t reader_threads; // threads de lecture des fichiers
pthread_t calculating_threads; // threads de calcul

// variables modifiables
int mt_multiplier = 2; // facteur du nombre de slots dans un buffer (= mt_multiplier * maxthreads)

// creation et protection du premier buffer
node_t toCompute_buffer;
pthread_mutex_t toCompute_mutex;
sem_t toCompute_empty;
sem_t toCompute_full;

// creation et protection du deuxième buffer
node_t computed_buffer;
pthread_mutex_t computed_mutex;
sem_t computed_empty;
sem_t computed_full;

// variables de fin de programme
int all_files_read = 0;


/* FONCTIONS PRINCIPALES */

/**
 * TODO
 */
int main(int argc, const char *argv[])
{
    // lecture des arguments
    if(argc < 3) { // nécessite au moins 3 arguments (nom de fonction, fichier d'entrée, fichier de sortie / option "-d")
        printf(stderr, "ERROR MESSAGE"); // imprime le problème à la stderr TODO : message d'erreur "Il n'y a pas assez d'arguments donnés, vous devez au moins donner un fichier contenant les fractales et un fichier de sortie!\n"
        exit(EXIT_FAILURE);

    } else {
        files_to_read = process_options(argc, argv); // traitement des arguments et options, récupère le nombre de fichiers à lire
    }

    // initialisation des mutex et semaphores des buffers
    int slots_in_buffer = ceil(maxthreads * mt_multiplier);
    initialise_buffer_protection(&toCompute_mutex, &toCompute_empty, &toCompute_full, slots_in_buffer); // pour toCompute_buffer
    initialise_buffer_protection(&computed_mutex,  &computed_empty,  &computed_full,  slots_in_buffer); // pour computed_buffer

    // lancement des thread de lecture, un thraed par fichier à lire
    reader_threads[files_to_read]; // threads de lecture
    int j = 0; // emplacement dans [reader_threads]
    for(int i = 1 ; i<argc ; i++) { // parcourt tous les arguments
        if(!strcmp(argv[i], "--maxthreads") || !strcmp(argv[i], "-d") || (!d_option && i = argc-1)) { // paramètre ou fichier de sortie finale
            // ne rien faire
        } else { // fichier à lire
            if(!strcmp(argv[i], "-")) { // nom du fichier à lire est l'entrée std
                char *file_to_read = 0;//TODO : lecture de l'entrée std
            } else { // nom du fichier à lire est un fichier normal
                char *file_to_read = argv[i];
            }
            pthread new_thread; // création d'un nouveau thread
            reader_threads[j] = new_thread; // thread mis dans le vecteur des lecteurs
            if(pthread_create(&new_thread, NULL, *file_reader, (void *) file_to_read)) { // initialisation du thread lecteur de fichier
                //TODO : traitement d'erreur à la création du thread
            }
            j++; // passer à l'emplacement suivant dans le vecteur de threads
        }
    }

    // lancement des threads de calcul
    calculating_threads[maxthreads]; // threads de calcul
    for(int j = 0 ; j < maxthreads ; j++) {
        pthread_t new_thread; // création d'un nouveau thread de calcul
        calculating_threads[j] = new_thread; // thread mis dans le vecteur des calculateurs
        if(pthread_create(&new_thread, NULL, *fractal_calculator, NULL)) { // initialisation du thread de calcul
            //TODO : traitement d'erreur à la création du thread
        }
    }


    // TODO : créer le(s) thread de sortie

    // TODO : récupérer la plus grande fractale


    // attendre que tous les threads de lecture ont fini de lire tous les fichiers
    for(int i = 0 ; i < files_to_read ; i++) {
        phtread_join(reader_threads[i], NULL);
    }
    all_files_read = 1;


    // TODO comment détecter que toutes les fractes ont été calculées et que les threads de calcul ont fini leur boulot ?


    stack_free(toCompute_buffer);
    stack_free(computed_buffer);

    return 0;
}

/**
 * TODO
 * producteur pour toCompute_buffer, lis dans un fichier dont le nom est donné en argument
 */
void *file_reader(void *file_to_read)
{
    char *filename = (char *)file_to_read;

    // TODO

    return NULL;
}

/**
 * TODO
 * consommateur de toCompute_buffer, producteur de computed_buffer, calcule les pixels des fractales
 */
void *fractal_calculator()
{
    // TODO
    // créer dans fractal.c et fractal.h la fonction fractal_compute

    return NULL;
}

/* SOUS_FONCTIONS D'AIDE */

/**
 * process_options : initialise d_option, mt_option, maxthreads et file_out qui sont retrouvés dans les
 *                   arguments d'appel à la main
 *
 * @argc : nombre d'arguments reçus avec la main
 * @argv : vecteurs des arguments (strings) reçus avec la main
 * @return: nombre de fichiers à lire
 */
int process_options(int argc, char *argv)
{
    for(int i = 1 ; i<argc ; i++) { // parcourt tous les arguments
        if(!strcmp(argv[i], "--maxthreads")) { // paramètre du nombre de threads de calcul
            mt_option = 1;
            i++; // passer à l'argument correspondant à l'option
            maxthreads = atoi(argv[i]);
        } else if(!strcmp(argv[i], "-d")) { // paramètre d'affichage des fractales
            d_option = 1;
        }
    }

    // print messages d'info sur les options
    if(d_option) {
        printf("Option -d présente : une image sera générée pour chaque fractale calculée\n");
    } else {
        // copie du nom du fichier de sortie dans la variable file_out
        char *file_name[strlen(argv[argc-1])];
        strcpy(file_name, argv[i]);
        file_out = file_name;

        printf("Option -d pas présente : une image de la fractale ayant la plus haute moyenne sera générée dans le fichier %s\n", file_out);
    }
    if(mt_option) {
        printf("Option --maxthreads présente : %i threads de calcul seront utlisés\n", maxthreads);
    } else {
        printf("Option --maxthreads pas présente : le nombre par défault de %i thread de calcul sera utlisé\n", maxthreads);
    }

    return argc - 1 - d_option*2 - mt_option*2; // retourne le nombre de fichiers à lire
}

/**
 * initialise_buffer_protection : initialse le mutex et les sémaphores utiles à la protection de buffers
 *
 * @mutex : mutex pour bloquer l'accès au buffer
 * @empty : sémaphore comptant le nombre de places libres dans le buffer
 * @full : sémaphore comptant le nombre de places occupées dans le buffer
 * @slots_in_buffer : nomre de places à prévoir dans le buffer
 */
void initialise_buffer_protection(pthread_mutex_t *mutex, sem_t *empty, sem_t *full, int slots_in_buffer)
{
    if(pthread_mutex_init(mutex, NULL) != 0) { // initialisation du mutex
        //error TODO : traitement d'erreur
    }
    if(sem_init(empty, 0 , slots_in_buffer) != 0) { // empty compte le nombre de slots vides dans un buffer, commence à [slots_in_buffer]
        //error TODO : traitement d'erreur
    }
    if(sem_init(full, 0 , 0) != 0) { // full compte le nombre de slots remplis dans un buffer, commence à 0
        //error TODO : traitement d'erreur
    }
}