#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>

/**
 * L'objectif de ce code est de mettre en place un thread :
 * ===
 * - réalisant l'affichage de données issues d'une structure lambda.
 * ===
 * Schématiquement :
 * ===
 *       Père
 *         ^
 *         |  
 * Thread1 | 
 */

/**
 * Structure lambda contenant des champs basiques.
 * (Ces champs n'ont aucun intérêt à par être utilisés pour l'exemple)
 * ===
 * codeA  : entier 
 * buffer : tableau de 10 caractères 
 */
typedef struct my_pthread_data {
  short int codeA;
  char buffer[32];
} pthread_data;


/**
 * Fonction exécutée par le thread
 */ 
void * my_thread_code(void * arg) {
  // Typage de la structure passée en paramètre
  pthread_data * data = (pthread_data *) arg;

  // Initialisation de la variable du code retour
  int * return_code = malloc(sizeof(int));
  if (return_code != NULL) {
    *return_code = 1664;
  }

  // Le code métier consiste à afficher les informations de la structure.
  printf("=~> Code executé par le thread\n"); 
  printf("* data.codeA  : %d\n", data->codeA);
  printf("* data.buffer : %s\n", data->buffer);
  printf("=~> Terminaison du thread\n");

  // Code retour du thread
  pthread_exit((void*)return_code);  
}


int main(int argc, char** argv) {
  // Variables
  pthread_t thread_id; 
  pthread_data my_data;
  int * my_return_code;
  // Initialisation de la structure de données avec des données lambda
  my_data.codeA = 42;
  
  memset(my_data.buffer, '\0', sizeof(char)*32);
  memcpy(my_data.buffer, "HELLO WORLD", sizeof(char)*11);

  // Initialisation du thread :
  // Les options de création du thread sont celles par défaut d'ou l'utilisation du la valeur NULL.
  // On positionne le pointeur sur fonction ou "callback" de la fonction my_thread_code.
  // On position le pointeur de la structure que l'on a initialisé.
  int ret =  pthread_create(&thread_id, NULL, &my_thread_code, &my_data);
 
  if (ret != 0) {
    perror("Impossible de créer le thread");
  }
  else {
    // Attente de terminaison du thread
    pthread_join(thread_id, (void**)&my_return_code);

    // Affichage du code retour du thread
    printf("Code retour du thread : %d\n", *my_return_code);

    free(my_return_code);
  }

  return 0;
}
