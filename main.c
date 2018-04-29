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

#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include "libstack/stack.h"
#include "libfractal/fractal.h"

// TODO : insérer gitlog au répertoire à rendre

/* VARIABLES GLOBALES */

int d_option = 0; // 1 si "-d" est présent dans les paramètres, 0 sinon
int mt_option = 0; // 1 si "--maxthread" est présent dans les paramètres, 0 sinon
int maxthreads = 1; // nombre de threads de calcul maximal, 1 par défault (valeur minimale)

int files_to_read; // nombre de fichiers à lire pour extraire les infos des fractales
char file_out[64]; // nom du fichier de sortie finale

// variables modifiables
int mt_multiplier = 2; // facteur du nombre de slots dans un buffer (= mt_multiplier * maxthreads)

// création des protections du premier buffer
node_t *toCompute_buffer;
pthread_mutex_t toCompute_mutex;
sem_t toCompute_empty;
sem_t toCompute_full;
int toCompute_state; // 0 si encore utilisé, 1 si plus utilisé

// création des protections du deuxième buffer
node_t *computed_buffer;
pthread_mutex_t computed_mutex;
sem_t computed_empty;
sem_t computed_full;
int computed_state; // 0 si encore utilisé, 1 si plus utilisé

// protection des états des buffers
pthread_mutex_t buffer_states; // protection des états des buffers, pour toCompute_state et computed_state
pthread_mutex_t executing_states; // protection des états de l'exécution du programme, pour les variables de fin de programme :

// variables de fin de programme, 0==FALSE , 1==TRUE
int all_files_read = 0;
int all_fractals_calculated = 0;
int all_fractals_printed = 0;

/* DECLARATION DES FONCTIONS UTILISÉES */

void *file_reader(); // pour le thread producteur du buffer toCompute_buffer
void *fractal_calculator(); // pour le thread consommateur de computed_buffer er producteur de computed_buffer
void *fractal_printer(); // pour le thread consommateur de computed_buffer
int process_options(); // pour l'initialsiation des options
int initialise_buffer_protection(); // pour initiasliser les mutex et semaphores d'un buffer


/* FONCTIONS PRINCIPALES */

/**
 * TODO
 */
int main(int argc, const char *argv[])
{
    // lecture des arguments
    if(argc < 3) { // nécessite au moins 3 arguments (nom de fonction, fichier d'entrée, fichier de sortie)
        fprintf(stderr, "Not enough arguments : at least 3 arguments needed in order to calculate fractals (function name, entry file, out file)\n"); // imprime le problème à la stderr
        exit(EXIT_FAILURE);
    } else {
        files_to_read = process_options(argc, argv); // traitement des arguments et options, récupère le nombre de fichiers à lire
    }

    // initialisation des mutex, semaphores et buffers
    int slots_in_buffer = ceil(maxthreads * mt_multiplier);
    if(initialise_buffer_protection(&toCompute_mutex, &toCompute_empty, &toCompute_full, slots_in_buffer)) { // pour toCompute_buffer
        // TODO : traitement d'erreur
    }
    if(initialise_buffer_protection(&computed_mutex,  &computed_empty,  &computed_full,  slots_in_buffer)) { // pour computed_buffer
        // TODO  : traitement d'erreur
    }
    if(pthread_mutex_init(&buffer_states, NULL) != 0) { // initialisation du mutex sur les états des buffers
        // TODO : traitement d'erreur
    }
    if(pthread_mutex_init(&executing_states, NULL) != 0) { // initialisation du mutex sur les états de l'exécution du programme
        // TODO : traitement d'erreur
    }

    // lancement des thread de lecture, un thraed par fichier à lire
    printf("Lancement des threads de lecture des fichiers\n");
    pthread_t reader_threads[files_to_read]; // threads de lecture de fichiers
    int j = 0; // emplacement dans [reader_threads]
    for(int i = 1 ; i<argc ; ++i) { // parcourt tous les arguments
        if(!strcmp(argv[i], "--maxthreads") || !strcmp(argv[i], "-d") || i == argc-1) { // option ou fichier de sortie finale
            if(!strcmp(argv[i], "--maxthreads")) {
                ++i; // passer argument de l'option maxthread
            }
            // ne rien faire
        } else { // fichier à lire
            char file_name[64];
            if(!strcmp(argv[i], "-")) { // nom du fichier à lire est l'entrée std
                printf("BESOIN D'UTILISER STD_IN\n"); // TODO : lecture de l'entrée std
            } else { // nom du fichier à lire est un fichier normal
                strcpy(file_name, argv[i]);
            }
            pthread_t new_thread; // création d'un nouveau thread
            if(pthread_create(&new_thread, NULL, file_reader, (void *) file_name)) { // initialisation du thread lecteur de fichier
                // TODO : traitement d'erreur
            }
            reader_threads[j] = new_thread; // thread mis dans le vecteur des lecteurs
            ++j; // passer à l'emplacement suivant dans le vecteur de threads
        }
    } // threads de lecture lancés et stockés dans [reader_threads]

    // lancement des threads de calcul
    printf("Lancement des threads de calcul des fractales\n");
    pthread_t calculating_threads[maxthreads]; // threads de calcul
    for(int j = 0 ; j < maxthreads ; ++j) {
        pthread_t new_thread; // création d'un nouveau thread de calcul
        if(pthread_create(&new_thread, NULL, fractal_calculator, NULL)) { // initialisation du thread de calcul
            // TODO : traitement d'erreur
        }
        calculating_threads[j] = new_thread; // thread mis dans le vecteur des calculateurs
    } // threads de calcul lancés et stockés dans [calculating_threads]


    // lancement du thread de sortie
    pthread_t printing_thread; // thread de sortie des fractales
    if(pthread_create(&printing_thread, NULL, fractal_printer, NULL)) { // initialisation du thread de sortie
        // TODO : traitement d'erreur
    }

    // attendre que tous les threads de lecture aient fini de lire tous les fichiers
    for(int i = 0 ; i < files_to_read ; ++i) {
        pthread_join(reader_threads[i], NULL);
    }
    pthread_mutex_lock(&executing_states);
    all_files_read = 1; // modifier l'état
    pthread_mutex_unlock(&executing_states);
    printf("Lecture des fichiers fini\n");

    // attendre que [toCompute_buffer] soit vidé
    pthread_mutex_lock(&toCompute_mutex);
    int length_toCompute = stack_length(toCompute_buffer);
    pthread_mutex_unlock(&toCompute_mutex);
    while(length_toCompute != 0) { // tant qu'il y a qqch dans toCompute_buffer
        pthread_mutex_lock(&toCompute_mutex);
        length_toCompute = stack_length(toCompute_buffer);
        pthread_mutex_unlock(&toCompute_mutex);
    }
    pthread_mutex_lock(&buffer_states);
    toCompute_state = 1; // toCopmute_buffer a fini d'être utilisé
    pthread_mutex_unlock(&buffer_states);
    printf("Plus aucune fractale en attente de calcul\n");

    // attendre que tous les threads de calcul aient fini de calculer toutes les fractales
    for(int i = 0 ; i < maxthreads ; ++i) {
        pthread_join(calculating_threads[i], NULL);
    }
    pthread_mutex_lock(&executing_states);
    all_fractals_calculated = 1; // modifier l'état
    pthread_mutex_unlock(&executing_states);
    printf("Calcul des fractales fini\n");

    // attendre que [computed_buffer] soit vidé
    pthread_mutex_lock(&computed_mutex);
    int length_computed = stack_length(computed_buffer);
    pthread_mutex_unlock(&computed_mutex);
    while(length_computed != 0) { // tant qu'il y a qqch dans computed_buffer
        pthread_mutex_lock(&computed_mutex);
        length_computed = stack_length(computed_buffer);
        pthread_mutex_unlock(&computed_mutex);
    }
    pthread_mutex_lock(&buffer_states);
    computed_state = 1; // computed_buffer a fini d'être utilisé
    pthread_mutex_unlock(&buffer_states);
    printf("Plus aucune fractale en attente de sortie\n");

    // attendre que le thread de sortie ait fini de sortir les fractales nécessaires
    pthread_join(printing_thread, NULL);
    pthread_mutex_lock(&executing_states);
    all_fractals_printed = 1; // modifier l'état
    pthread_mutex_unlock(&executing_states);
    printf("Sortie des fractales fini\n");

    stack_free(toCompute_buffer);
    stack_free(computed_buffer);

    printf("Programme exécuté correctement. EXIT\n");
    exit(EXIT_SUCCESS);
} // end main()

/**
 * TODO
 * producteur pour toCompute_buffer, lis dans un fichier dont le nom est donné en argument
 */
void *file_reader(void *file_to_read)
{
    char *filename = (char *)file_to_read;
    printf("Fichier \"%s\" en train d'être lu\n", filename);

    // variables utilisées pour les fractales
    double a, b;
    int width, height;

    // ouverture du fichier
    FILE *file = fopen(filename, "r");
    if(file == NULL) {
        exit(EXIT_FAILURE);// TODO : traitement d'erreur
    }

    // lecture des lignes du fichier
    char line[150]; // taille que peut avoir une ligne + marge
    while(fgets(line, sizeof(line), file) != NULL) { // parcourt les lignes tant qu'elles existent
        if(line[0] == '#') { // affiche les commentaires TODO :  ne traite pas encore les lignes vides !!
            printf("Dans le fichier %s        : %s", filename, line);
        } else {
            printf("Ajout de la fractale suivante à la pile : %s", line);

            // lecture des attributs de la fractale
            char *name = strtok(line, " ");
            width = atoi(strtok(NULL, " "));
            height = atoi(strtok(NULL, " "));
            a = atof(strtok(NULL, " "));
            b = atof(strtok(NULL, " "));

            // ajout de la fractale dans [toCompute_buffer]
            sem_wait(&toCompute_empty); // attendre qu'un slot se libère
            pthread_mutex_lock(&toCompute_mutex); // section critique
            if(stack_push(&toCompute_buffer, fractal_new(name, width, height, a, b))) { // TODO quid si fractal_new retourne NULL ?
                // TODO : traitement d'erreur
            }
            pthread_mutex_unlock(&toCompute_mutex);	// fin de section critique
            sem_post(&toCompute_full); // un slot rempli de plus
        }
    }

    if(fclose(file)) { // fermer le document ouvert
        // TODO : traitement d'erreur
    }
    pthread_exit(NULL);
} // void *file_reader()

/**
 * TODO
 * consommateur de toCompute_buffer, producteur de computed_buffer, calcule les pixels des fractales
 */
void *fractal_calculator()
{
    fractal_t *toCompute_fractal; // fractale à extraire du buffer et à calculer avant de la remettre dans un autre buffer

    // regarder s'il y a des fractales à calculer
    pthread_mutex_lock(&buffer_states);
    int finished = toCompute_state;
    pthread_mutex_unlock(&buffer_states);
    while(!finished) {

        // extraire une fractale à calculer du [toCompute_buffer]
        sem_wait(&toCompute_full); // attente qu'un slot se remplisse
        pthread_mutex_lock(&toCompute_mutex); // section critique
        toCompute_fractal = stack_pop(&toCompute_buffer);
        if (toCompute_fractal == NULL) { // TODO quid si fractal_new retourne NULL ?
            // TODO : traitement d'erreur
        }
        pthread_mutex_unlock(&toCompute_mutex); // fin de section critique
        sem_post(&toCompute_empty); // un slot libre en plus

        // calculer les pixels de la fractale
        for (int i = 0; i < fractal_get_width(toCompute_fractal) ; ++i) { // parcourt les abscisses
            for (int j = 0; j < fractal_get_height(toCompute_fractal) ; ++j) { // parcourt les ordonnées
                fractal_compute_value(toCompute_fractal, i, j);
            }
        }

        // ajout de la fractale dans [computed_buffer]
        sem_wait(&computed_empty); // attendre qu'un slot se libère
        pthread_mutex_lock(&computed_mutex); // section critique
        if (stack_push(&computed_buffer, toCompute_fractal)) {
            // TODO : traitement d'erreur
        }
        pthread_mutex_unlock(&computed_mutex);    // fin de section critique
        sem_post(&computed_full); // un slot rempli de plus

        // regarder s'il reste des fractales à calculer
        pthread_mutex_lock(&buffer_states);
        finished = toCompute_state;
        pthread_mutex_unlock(&buffer_states);
    }
    pthread_exit(NULL);
} // void *fractal_calculator()

/**
 * TODO
 */
void *fractal_printer()
{
    fractal_t *computed_fractal; // fractale à extraire du buffer et à analyser
    node_t *highest_fractal_stack; // pile contenant la/les fractale(s) avec la plus haute valeur moyenne
    fractal_t *temp_highest_fractal; // fractale utilisé lors de la comparaison de moyennes

    // regarder s'il y a des fractales à extraire
    pthread_mutex_lock(&buffer_states);
    int finished = computed_state;
    pthread_mutex_unlock(&buffer_states);
    while(!finished) { // tant qu'il y a des fractales à extraire

        // extraire une fractale calculée du [computed_buffer]
        sem_wait(&computed_full); // attente qu'un slot se remplisse
        pthread_mutex_lock(&computed_mutex); // section critique
        computed_fractal = stack_pop(&computed_buffer);
        if(computed_fractal == NULL) {
            // TODO : traitement d'erreur
        }
        pthread_mutex_unlock(&computed_mutex); // fin de section critique
        sem_post(&computed_empty); // un slot libre en plus

        // affiche la fractale si option -d est présente
        if(d_option) {
            // ajouter .bmp au nom de la fractale
            char name_and_extention[68];
            strcpy(name_and_extention, fractal_get_name(computed_fractal));
            strcat(name_and_extention, ".bmp");

            if(write_bitmap_sdl(computed_fractal, name_and_extention)) {
                // TODO : traitement d'erreur
            }
            printf("Fractale \"%s\" extraite sous forme .bmp\n", name_and_extention);
        }

        // calculer la moyenne et garder les plus grandes
        fractal_compute_average(computed_fractal);
        if(highest_fractal_stack == NULL) {
            stack_push(&highest_fractal_stack, computed_fractal);
        } else {
            temp_highest_fractal = stack_pop(&highest_fractal_stack); // retire la fractale à la plus haute moyenne de la pile
            if(fractal_get_average(temp_highest_fractal) > fractal_get_average(computed_fractal)) {
                stack_push(&highest_fractal_stack, temp_highest_fractal); // remet à sa place
                fractal_free(computed_fractal);
            } else if(fractal_get_average(temp_highest_fractal) == fractal_get_average(computed_fractal)) { // si même moyenne
                stack_push(&highest_fractal_stack, temp_highest_fractal); // remet à sa place ...
                stack_push(&highest_fractal_stack, computed_fractal); // ... et ajoute le nouveau qui a la même moyenne
            } else {
                stack_free(highest_fractal_stack); // libère tous les anciens meilleurs
                stack_push(&highest_fractal_stack, computed_fractal); // met le nouveau meilleur sur la pile
            }
        }

/*printf("TEST 1\n");
        fractal_compute_average(computed_fractal);
printf("TEST 11\n");
        if(fractal_get_average(highest_fractal) < fractal_get_average(computed_fractal)) {
printf("TEST 12\n");
            fractal_t *temp = highest_fractal;
            highest_fractal = computed_fractal;
            fractal_free(temp);
        } else {
            fractal_free(computed_fractal);
        }
printf("TEST 2\n");*/

        // regarder s'il reste des fractales à extraire
        pthread_mutex_lock(&buffer_states);
        finished = computed_state;
        pthread_mutex_unlock(&buffer_states);
    }

    // sortie des/de la fractale(s) avec la plus grande moyenne
    fractal_t *highest_fractal = stack_pop(&highest_fractal_stack);
    while(highest_fractal != NULL) {
        printf("Création du fichier BMP avec la fractale ayant la plus grande valeur de moyenne : \"%s\"\n", fractal_get_name(highest_fractal));
        if(write_bitmap_sdl(highest_fractal, file_out)) {
            // TODO : traitement d'erreur
        }
        fractal_free(highest_fractal);
        highest_fractal = stack_pop(&highest_fractal_stack);
    }
    stack_free(highest_fractal_stack);

    pthread_exit(NULL);
} // void *fractal_printer()


/* SOUS_FONCTIONS D'AIDE */

/**
 * process_options : initialise d_option, mt_option, maxthreads et file_out qui sont retrouvés dans les
 *                   arguments d'appel à la main
 *
 * @argc : nombre d'arguments reçus avec la main
 * @argv : vecteurs des arguments (strings) reçus avec la main
 * @return: nombre de fichiers à lire
 */
int process_options(int argc, char *argv[])
{
    for(int i = 1 ; i<argc ; ++i) { // parcourt tous les arguments
        if(!strcmp(argv[i], "--maxthreads")) { // paramètre du nombre de threads de calcul
            mt_option = 1;
            ++i; // passer à l'argument correspondant à l'option
            maxthreads = atoi(argv[i]);
        } else if(!strcmp(argv[i], "-d")) { // paramètre d'affichage des fractales
            d_option = 1;
        }
    }

    //TODO error handeling sur arguments (pas bon nombre ....)

    // copie du nom du fichier de sortie dans la variable file_out
    strcpy(file_out, argv[argc-1]);

    // print messages d'info sur les options
    if(d_option) {
        printf("Option -d présente : une image sera générée pour chaque fractale calculée\n");
    } else {
        printf("Option -d pas présente : une image de la fractale ayant la plus haute moyenne sera générée dans le fichier %s\n", file_out);
    }
    if(mt_option) {
        if(maxthreads == 1) {
            printf("Option --maxthreads présente : %i thread de calcul sera utlisé\n", maxthreads);
        } else {
            printf("Option --maxthreads présente : %i threads de calcul seront utlisés\n", maxthreads);
        }
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
 * @return : 0 si tout correctement initialisé, 1 si erreur
 */
int initialise_buffer_protection(pthread_mutex_t *mutex, sem_t *empty, sem_t *full, int slots_in_buffer)
{
    if(pthread_mutex_init(mutex, NULL) != 0) { // initialisation du mutex
        // TODO : traitement d'erreur
    }
    if(sem_init(empty, 0 , slots_in_buffer) != 0) { // empty compte le nombre de slots vides dans un buffer, commence à [slots_in_buffer]
        // TODO : traitement d'erreur
    }
    if(sem_init(full, 0 , 0) != 0) { // full compte le nombre de slots remplis dans un buffer, commence à 0
        // TODO : traitement d'erreur
    }

    return 1; //successful
}
