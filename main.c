#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <getopt.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include "libstack/stack.h"
#include "libfractal/fractal.h"

// TODO : insérer gitlog au répertoire à rendre
// TODO : quid si appel d'aileurs ? comment imprimer là ?
// TODO : négliger les fractales au nom doubles

/* VARIABLES GLOBALES */

int d_option = 0; // 1 si "-d" est présent dans les paramètres, 0 sinon
int mt_option = 0; // 1 si "--maxthread" est présent dans les paramètres, 0 sinon
int maxthreads = 1; // nombre de threads de calcul maximal, 1 par défault (valeur minimale)

int files_to_read; // nombre de fichiers à lire pour extraire les infos des fractales
char file_out[64]; // nom du fichier de sortie finale

// variables modifiables
int mt_multiplier = 2; // facteur du nombre de slots dans un buffer (= mt_multiplier * maxthreads)

// protections du premier buffer
node_t *toCompute_buffer;
pthread_mutex_t toCompute_mutex;
sem_t toCompute_empty;
sem_t toCompute_full;
int toCompute_state; // 0 si buffer est encore utilisé, 1 si buffer n'est plus utilisé

// protections du deuxième buffer
node_t *computed_buffer;
pthread_mutex_t computed_mutex;
sem_t computed_empty;
sem_t computed_full;
int computed_state; // 0 si buffer est encore utilisé, 1 si buffer n'est plus utilisé

// protection des états des buffers
pthread_mutex_t buffer_states; // protection des états des buffers (toCompute_state et computed_state)
pthread_mutex_t executing_states; // protection des états de l'exécution du programme, pour les variables de fin de programme TODO nécessaire si que dans main ?

// variables de fin de programme, 0==FALSE , 1==TRUE
int all_files_read = 0;
int all_fractals_calculated = 0;
int all_fractals_printed = 0;
int fractals_to_process = 0;
int fractals_to_calculate = 0;


/* DECLARATION DES FONCTIONS UTILISÉES */

void process_options(); // pour l'initialsiation des options
int initialise_buffer_protection(); // pour initiasliser les mutex et semaphores d'un buffer
void *file_reader(); // pour le thread producteur du buffer toCompute_buffer
void *fractal_calculator(); // pour le thread consommateur de computed_buffer et producteur de computed_buffer
void *fractal_printer(); // pour le thread consommateur de computed_buffer
int get_protected_variable(char variable[]); // retourne la valeur de la variable donnée en parmètre après un accès protégé


/* FONCTIONS PRINCIPALES */

/**
 * main : calcule et affiche des fractales retrouvées dans les fichiers passés en argument
 *
 * Exemple d'utilisation :
 *      ./main [-d] [--maxthreads n] fichier1 [fichierN] fichierOut
 *      -d : afficher toutes les fractales calculées
 *      --maxthreads X : vouloir utiliser X threads de calcul, sinon 1 par défault
 *      fichier* : fichier à partir duquel extraire les infos des fractales à calculer
 *      fichierOut : fichier dans lequel sortir la fractale à la plus haute moyenne
 *      peu importe l'ordre,  tant que fichierOut se retrouve en dernier
 *
 * @argc : nombre d'arguments reçus
 * @argv : vecteurs des arguments (strings) reçus
 * @return: 0 si tout s'est bien exécuté, autre sinon
 */
int main(int argc, const char *argv[])
{
    // lecture des arguments, initialisation de d_option, mt_option, maxthreads, files_to_read et file_out
    process_options(argc, argv);

    // initialisation des mutex, semaphores et buffers
    int slots_in_buffer = ceil(maxthreads * mt_multiplier); // nombre d'éléments max dans un buffer
    if(initialise_buffer_protection(&toCompute_mutex, &toCompute_empty, &toCompute_full, slots_in_buffer)) { // initialisation de toCompute_buffer
        fprintf(stderr, "Error at toCompute_buffer initialisation - Exiting main\n"); // imprime le problème à la stderr
        exit(EXIT_FAILURE);
    }
    if(initialise_buffer_protection(&computed_mutex,  &computed_empty,  &computed_full,  slots_in_buffer)) { // initialisation de computed_buffer
        fprintf(stderr, "Error at computed_buffer initialisation - Exiting main\n"); // imprime le problème à la stderr
        exit(EXIT_FAILURE);
    }
    if(pthread_mutex_init(&buffer_states, NULL) != 0) { // initialisation du mutex sur les états des buffers
        fprintf(stderr, "Error at \"buffer_states\" mutex initialisation - Exiting main\n"); // imprime le problème à la stderr
        exit(EXIT_FAILURE);
    }
    if(pthread_mutex_init(&executing_states, NULL) != 0) { // initialisation du mutex sur les états de l'exécution du programme
        fprintf(stderr, "Error at \"executing_states\" mutex initialisation - Exiting main\n"); // imprime le problème à la stderr
        exit(EXIT_FAILURE);
    }

    /* LANCEMENT DES THREADS */

    // lancement des thread de lecture, un thread par fichier à lire
    printf("Lancement des threads de lecture des fichiers\n");
    pthread_t reader_threads[files_to_read]; // vecteur contenant les threads de lecture de fichiers
    int j = 0; // emplacement dans [reader_threads]
    int i;
    for(i = 1 ; i<argc-1 ; ++i) { // parcourt tous les arguments sauf premier ("main") et dernier ("fichierOut")
        if(!strcmp(argv[i], "--maxthreads") || !strcmp(argv[i], "-d")) { // si option
            if(!strcmp(argv[i], "--maxthreads")) {
                ++i; // passe l'argument de l'option maxthread
            }
            // ne rien faire
        } else { // si fichier à lire
            char file_name[100];
            if(!strcmp(argv[i], "-")) { // fichier à lire est l'entrée std
                printf("BESOIN D'UTILISER STD_IN\n"); // TODO : lecture de l'entrée std
            } else { // fichier à lire est un fichier normal
                strcpy(file_name, argv[i]);
            }
            pthread_t new_thread; // création d'un nouveau thread
            if(pthread_create(&new_thread, NULL, file_reader, (void *) file_name)) { // initialisation du thread lecteur de fichier
                fprintf(stderr, "Error at \"file_reader\" thread creation - Exiting main\n"); // imprime le problème à la stderr
                exit(EXIT_FAILURE);
            }
            reader_threads[j] = new_thread; // nouveau thread mis dans le vecteur des lecteurs
            ++j; // passer à l'emplacement suivant dans le vecteur de threads
        }
    } // threads de lecture lancés et stockés dans [reader_threads]

    // lancement des threads de calcul
    printf("Lancement des threads de calcul des fractales\n");
    pthread_t calculating_threads[maxthreads]; // vecteur contenant les threads de calcul
    int k;
    for(k = 0 ; k < maxthreads ; ++k) {
        pthread_t new_thread; // création d'un nouveau thread de calcul
        if(pthread_create(&new_thread, NULL, fractal_calculator, NULL)) { // initialisation du thread de calcul
            fprintf(stderr, "Error at \"fractal_calculator\" thread creation - Exiting main\n"); // imprime le problème à la stderr
            exit(EXIT_FAILURE);
        }
        calculating_threads[k] = new_thread; // nouveau thread mis dans le vecteur des calculateurs
    } // threads de calcul lancés et stockés dans [calculating_threads]

    // lancement du thread de sortie
    pthread_t printing_thread; // création du thread de sortie des fractales
    if(pthread_create(&printing_thread, NULL, fractal_printer, NULL)) { // initialisation du thread de sortie
        fprintf(stderr, "Error at \"fractal_printer\" thread creation - Exiting main\n"); // imprime le problème à la stderr
        exit(EXIT_FAILURE);
    } // threads de sortie lancé

    /* ATTENTE DE L'AVENCÉE DES THREADS */

    // attendre que tous les threads de lecture aient fini de lire tous les fichiers
    int l;
    for(l = 0 ; l < files_to_read ; ++l) {
        pthread_join(reader_threads[l], NULL); // finir les threads de lecture, pas de valeur de retour attendue (attente sur pthread_join tant qu'un thread n'a pas retourné)
    }
    pthread_mutex_lock(&executing_states); // section critique
    all_files_read = 1; // modifier l'état
    pthread_mutex_unlock(&executing_states); // fin de section critique
    printf("Lecture des fichiers fini\n");

    // attendre que le thread de sortie ait fini de sortir les fractales nécessaires
    pthread_join(printing_thread, NULL); // finir le thread de sortie, pas de valeur de retour attendue (attente sur pthread_join tant que le thread n'a pas retourné)
    printf("Sortie des fractales fini\n");

    // annuler tous les threads de calcul
    int m;
    for(m = 0 ; m < maxthreads ; ++m) {
        pthread_join(calculating_threads[m], NULL); // finir les threads de calcul et terminer leur utilisation // TODO cancel ou join ??
    }
    printf("Threads de calcul terminés\n");

    // libération de toutes les variables enregistrées
    stack_free(toCompute_buffer);
    stack_free(computed_buffer);
    // TODO free autre chose ? mutex ? semaphores ?

    printf("Programme exécuté correctement. EXIT\n");
    exit(EXIT_SUCCESS);
} // end main()

/**
 * file_reader : fonction producteur pour toCompute_buffer
 *               lit dans un fichier et ajoute les fractales retrouvables dans ce fichier à la pile toCompute_buffer
 *
 * @file_to_read : nom du fichier à lire
 */
void *file_reader(char *file_to_read)
{
    printf("Lecture du fichier \"%s\"\n", file_to_read);

    // variables utilisées pour les fractales
    double a, b;
    int width, height;

    // ouverture du fichier
    FILE *file = fopen(file_to_read, "r");
    if(file == NULL) {
        fprintf(stderr, "Error at file \"%s\" opening - Exiting from file_reader\n", file_to_read); // imprime le problème à la stderr
        exit(EXIT_FAILURE);
    }

    // lecture des lignes du fichier
    char line[150]; // taille que peut avoir une ligne + marge
    while(fgets(line, sizeof(line), file) != NULL) { // parcourt les lignes tant qu'elles existent //TODO fscanf !!!
        if(strlen(line) < 2 || line[0] == '\n') { // passe les lignes vides // TODO ne marche pas quid pour les lignes vides ?
            // rien faire
        } else if(line[0] == '#') { // affiche les commentaires
            printf("Commentaire dans le fichier %s : %s", file_to_read, line);
        } else {
            printf("Ajout de la fractale suivante à la pile : %s", line);

            // lecture des attributs de la fractale
            char *name = strtok(line, " ");
            width = atoi(strtok(NULL, " "));
            height = atoi(strtok(NULL, " "));
            a = atof(strtok(NULL, " "));
            b = atof(strtok(NULL, " "));

            // création de la fractale
            fractal_t *new_fractal = fractal_new(name, width, height, a, b);
            if(new_fractal == NULL) { // si erreur à la création TODO : ou si double
                fprintf(stderr, "Error at fractal creation (returning NULL) - Ignoring fractal \"%s %i %i %f %f\"\n", name, width, height, a, b); // imprime le problème à la stderr
            } else { // si pas d'erreur à la création
                // ajout de la fractale dans [toCompute_buffer]
                sem_wait(&toCompute_empty); // attendre qu'un slot se libère
                pthread_mutex_lock(&toCompute_mutex); // section critique
                if (stack_push(&toCompute_buffer, new_fractal)) {
                    fprintf(stderr, "Error at pushing into stack - Exiting from file_reader\n"); // imprime le problème à la stderr
                    exit(EXIT_FAILURE);
                }
                pthread_mutex_unlock(&toCompute_mutex); // fin de section critique
                sem_post(&toCompute_full); // un slot rempli de plus

                // incrémentation du nombre de fractales a process
                pthread_mutex_lock(&executing_states); // section critique
                fractals_to_process++; // incrémente la valeur
                fractals_to_calculate++; // incrémente la valeur
                pthread_mutex_unlock(&executing_states); // fin de section critique
            }
        }
    }

    if(fclose(file)) { // fermer le document ouvert
        fprintf(stderr, "Error at file \"%s\" closing - Exiting from file_reader\n", file_to_read); // imprime le problème à la stderr
        exit(EXIT_FAILURE);
    }
    pthread_exit(NULL);
} // void *file_reader()

/**
 * fractal_calculator : fonction consommateur de toCompute_buffer et producteur pour computed_buffer
 *                      calcule les pixels des fractales extraites de toCompute_buffer et remet la fractale calculée
 *                      dans computed_buffer
 */
void *fractal_calculator()
{
    fractal_t *toCompute_fractal; // fractale à extraire du buffer et à calculer avant de la remettre dans un autre buffer

    while(1) { // tourne indéfinimant, jusqu'à ce que le thread soit annulé depuis la main

        // extraire une fractale à calculer du [toCompute_buffer]
        sem_wait(&toCompute_full); // attente qu'un slot se remplisse

        // décrémentation du nombre de fractales a calculer
        pthread_mutex_lock(&executing_states); // section critique
        fractals_to_calculate--; // décrémente la valeur
        pthread_mutex_unlock(&executing_states); // fin de section critique

        pthread_mutex_lock(&toCompute_mutex); // section critique
        toCompute_fractal = stack_pop(&toCompute_buffer);
        if(toCompute_fractal == NULL) {
            fprintf(stderr, "Error at popping from stack - Exiting from fractal_calculator\n"); // imprime le problème à la stderr
            exit(EXIT_FAILURE);
        }
        pthread_mutex_unlock(&toCompute_mutex); // fin de section critique
        sem_post(&toCompute_empty); // un slot libre en plus

        // calculer les pixels de la fractale
        int i;
        int j;
        for (i = 0; i < fractal_get_width(toCompute_fractal) ; ++i) { // parcourt les abscisses
            for (j = 0; j < fractal_get_height(toCompute_fractal) ; ++j) { // parcourt les ordonnées
                fractal_compute_value(toCompute_fractal, i, j); // assigne la bonne valeur au pixel (i,j) TIME CONSUMING !
            }
        }

        // ajout de la fractale dans [computed_buffer]
        sem_wait(&computed_empty); // attendre qu'un slot se libère
        pthread_mutex_lock(&computed_mutex); // section critique
        if(stack_push(&computed_buffer, toCompute_fractal)) {
            fprintf(stderr, "Error at pushing into stack - Exiting from fractal_calculator\n"); // imprime le problème à la stderr
            exit(EXIT_FAILURE);
        }
        pthread_mutex_unlock(&computed_mutex); // fin de section critique
        sem_post(&computed_full); // un slot rempli de plus

        // affiche la fractale si option -d est présente
        if(d_option) {
            // ajouter .bmp au nom de la fractale et place ce string dans name_and_extention
            char name_and_extention[68]; // contiendra nouveau nom
            sprintf(name_and_extention, "%s.bmp", fractal_get_name(toCompute_fractal));

            // sortie de la fractale
            if(write_bitmap_sdl(toCompute_fractal, name_and_extention)) {
                fprintf(stderr, "Error at bitmap writing - Exiting from fractal_calculator\n"); // imprime le problème à la stderr
                exit(EXIT_FAILURE);
            }
            printf("Fractale \"%s\" extraite en tant que fichier .bmp\n", name_and_extention);
        }
    }
    /* SECTION JAMAIS ATTEINTE à cause de while(1) mais sinon :
     * printf("Sortie d'un thread de calcul\n");
     * pthread_exit(NULL);
     */
} // void *fractal_calculator()

/**
 * fractal_printer : fonction consommateur de computed_buffer
 *                   calcule les moyennes des fractales extraites de computed_buffer, garde temporairement les
 *                   fractales avec la plus haute moyenne
 *                   affiche toutes les fractales si option -d est activée et affiche d'offices les/la frastale(s) dont
 *                   la valeur moyenne est la plus haute
 */
void *fractal_printer()
{
    fractal_t *computed_fractal; // fractale à extraire du buffer et à analyser
    fractal_t *temp_highest_fractal; // fractale ayant pour le moment la plus haute moyenne

    // tant qu'il y a des fractales à extraire */
    while(!get_protected_variable("all_files_read") || get_protected_variable("fractals_to_process") > 0) { // attente qu'un slot se remplisse tant qu'on attend des éléments dans [toCompute_buffer]
        // extraire une fractale calculée du [computed_buffer]
        sem_wait(&computed_full); // attente qu'un slot se remplisse
        pthread_mutex_lock(&computed_mutex); // section critique
        computed_fractal = stack_pop(&computed_buffer);
        if(computed_fractal == NULL) {
            fprintf(stderr, "Error at popping from stack - Exiting from fractal_printer\n"); // imprime le problème à la stderr
            exit(EXIT_FAILURE);
        }
        pthread_mutex_unlock(&computed_mutex); // fin de section critique
        sem_post(&computed_empty); // un slot libre en plus

        // calculer la moyenne et garder les plus grandes
        fractal_compute_average(computed_fractal);
        if(temp_highest_fractal == NULL) { // si la fractale ayant la plus haute moyenne n'existe pas encore
            temp_highest_fractal = computed_fractal;
        } else if(fractal_get_average(temp_highest_fractal) > fractal_get_average(computed_fractal)) {
            fractal_free(computed_fractal);
        } else if(fractal_get_average(temp_highest_fractal) < fractal_get_average(computed_fractal)) {
            fractal_free(temp_highest_fractal);
            temp_highest_fractal = computed_fractal;
        } else {
            printf("La fractale \"%s\" a la même moyenne que la fractale \"%s\". La première des deux est gardée.", fractal_get_name(temp_highest_fractal), fractal_get_name(computed_fractal));
            fractal_free(computed_fractal);
        }

        // décrémentation du nombre de fractales à process
        pthread_mutex_lock(&executing_states); // section critique
        fractals_to_process--; // décrémente la valeur
        pthread_mutex_unlock(&executing_states); // fin de section critique
    }

    // sortie de la fractale avec la plus grande moyenne
    printf("Création du fichier .bmp avec la fractale ayant la plus grande moyenne : \"%s\"\n", fractal_get_name(highest_fractal));

    // ajouter .bmp au nom du fichier TODO sauf si .bmp déjà présent
    char name_and_extention[68]; // contiendra nouveau nom
    sprintf(name_and_extention, "%s.bmp", file_out);

    // sortie de la fractale
    if(write_bitmap_sdl(temp_highest_fractal, name_and_extention)) {
        fprintf(stderr, "Error at bitmap writing - Exiting from fractal_printer\n"); // imprime le problème à la stderr
        exit(EXIT_FAILURE);
    }

    pthread_exit(NULL);
} // void *fractal_printer()


/* SOUS_FONCTIONS D'AIDE */

/**
 * process_options : initialise d_option, mt_option, maxthreads, files_to_read et file_out en fonction arguments reçus lors de l'appel à la main
 *
 * @argc : nombre d'arguments reçus avec la main
 * @argv : vecteurs des arguments (strings) reçus avec la main
 */
void process_options(int argc, char *argv[])
{
    if(argc < 3) { // nécessite au moins 3 arguments (nom de fonction, fichier d'entrée, fichier de sortie)
        fprintf(stderr, "Not enough arguments : at least 3 arguments needed in order to calculate fractals (function name, entry file, out file)\n"); // imprime le problème à la stderr
        exit(EXIT_FAILURE);
    }

    // utilisation de getopt()
    int opt= 0;
    static struct option long_options[] = { // pour les longues options
            {"maxthreads", required_argument, 0, 'm'}, // l'option m attend un int en argument
            {0,            0,                 0, 0  }}; // ligne vide obligatoire
    int long_index =0;
    while((opt = getopt_long(argc, argv, "dm:", long_options, &long_index )) != -1) { // voir man getopt
        switch(opt) {
            case 'd' : // -d présent
                d_option = 1;
                break;
            case 'm' : // -maxthreads présent
                mt_option = 1;
                maxthreads = atoi(optarg);
                break;
            default : // option inconnue
                fprintf(stderr, "Unknown argument detected - Exiting from process_options\n");
                exit(EXIT_FAILURE);
        }
    }

    // copie du nom du fichier de sortie dans la variable file_out
    strcpy(file_out, argv[argc-1]);

    // calcul du nombre de fichiers à lire
    files_to_read = argc - 2 - d_option - mt_option*2;

    // vérification de validité des arguments
    if(files_to_read < 1) {
        fprintf(stderr, "At least one file to be read needed - Exiting from process_options\n"); // imprime le problème à la stderr
        exit(EXIT_FAILURE);
    }

    // affiche messages d'info sur les options
    if(d_option) {
        printf("Option -d présente : une image sera générée pour chaque fractale calculée\n");
    } else {
        printf("Option -d pas présente : une image de la fractale ayant la plus haute moyenne sera générée dans le fichier %s\n", file_out);
    }
    if(mt_option) {
        if(maxthreads == 1) {
            printf("Option --maxthreads présente : %i thread de calcul sera utlisé\n", maxthreads);
        } else if(maxthreads <= 0) {
            fprintf(stderr, "Not enough calculation threads - Exiting from process_optios\n"); // imprime le problème à la stderr
            exit(EXIT_FAILURE);
        } else {
            printf("Option --maxthreads présente : %i threads de calcul seront utlisés\n", maxthreads);
        }
    } else {
        printf("Option --maxthreads pas présente : le nombre par défault de %i thread de calcul sera utlisé\n", maxthreads);
    }
} // flags d'options sont mis à jour et nous connaissons [files_to_read] et [file_out]

/**
 * initialise_buffer_protection : initialse le mutex et les sémaphores utiles à la protection de buffers
 *
 * @mutex : mutex pour bloquer l'accès au buffer
 * @empty : sémaphore comptant le nombre de places libres dans le buffer
 * @full : sémaphore comptant le nombre de places occupées dans le buffer
 * @slots_in_buffer : nombre de places à prévoir dans le buffer
 * @return : 0 si tout est correctement initialisé, 1 si erreur
 */
int initialise_buffer_protection(pthread_mutex_t *mutex, sem_t *empty, sem_t *full, int slots_in_buffer)
{
    if(pthread_mutex_init(mutex, NULL) != 0) { // initialisation du mutex
        fprintf(stderr, "Error at mutex initialisation\n"); // imprime le problème à la stderr
        return 1;
    }
    if(sem_init(empty, 0 , slots_in_buffer) != 0) { // empty compte le nombre de slots vides dans un buffer, commence à [slots_in_buffer]
        fprintf(stderr, "Error at semaphore initialisation\n"); // imprime le problème à la stderr
        return 1;
    }
    if(sem_init(full, 0 , 0) != 0) { // full compte le nombre de slots remplis dans un buffer, commence à 0
        fprintf(stderr, "Error at semaphore initialisation\n"); // imprime le problème à la stderr
        return 1;
    }

    return 0; //successful
}

/**
 * get_protected_variable : retourne la valeur de la variable donnée en parmètre après un accès protégé
 *
 * @variable : nom (string) de la variable à accéder
 * @return : valeur de la variable accédée, quitte si erreur
 */
int get_protected_variable(char variable[])
{
    if(!strcmp(variable, "all_files_read")) { // si la variable à accéder est [all_files_read]
        pthread_mutex_lock(&executing_states); // section critique
        int to_return = all_files_read; // récupérer valeur de l'état
        pthread_mutex_unlock(&executing_states); // fin de section critique
        return to_return;
    }
    if(!strcmp(variable, "all_fractals_calculated")) { // si la variable à accéder est [all_fractals_calculated]
        pthread_mutex_lock(&executing_states); // section critique
        int to_return = all_fractals_calculated; // récupérer valeur de l'état
        pthread_mutex_unlock(&executing_states); // fin de section critique
        return to_return;
    }
    if(!strcmp(variable, "fractals_to_process")) { // si la variable à accéder est [fractals_to_process]
        pthread_mutex_lock(&executing_states); // section critique
        int to_return = fractals_to_process; // récupérer valeur de l'état
        pthread_mutex_unlock(&executing_states); // fin de section critique
        return to_return;
    }
    if(!strcmp(variable, "fractals_to_calculate")) { // si la variable à accéder est [fractals_to_calculate]
        pthread_mutex_lock(&executing_states); // section critique
        int to_return = fractals_to_calculate; // récupérer valeur de l'état
        pthread_mutex_unlock(&executing_states); // fin de section critique
        return to_return;
    }

    fprintf(stderr, "Trying to access unknown or untreated variable - Exiting from get_protected_variable\n"); // imprime le problème à la stderr
    exit(EXIT_FAILURE);
}
