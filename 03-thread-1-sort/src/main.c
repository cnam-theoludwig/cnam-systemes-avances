/* single_thread.c */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <limits.h>

#ifndef SIZE
#define SIZE 10000000
#endif

int *tab;

double elapsed(struct timeval a, struct timeval b) {
    return (b.tv_sec - a.tv_sec) + (b.tv_usec - a.tv_usec) / 1e6;
}

int main(int argc, char** argv) {
    size_t i;
    tab = malloc((size_t)SIZE * sizeof(int));
    if (!tab) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    srand((unsigned)time(NULL));
    for (i = 0; i < SIZE; ++i) {
      tab[i] = rand();
    }

    struct timeval t0, t1;
    gettimeofday(&t0, NULL);

    int gmin = INT_MAX;
    int gmax = INT_MIN;
    for (i = 0; i < SIZE; ++i) {
        if (tab[i] < gmin) gmin = tab[i];
        if (tab[i] > gmax) gmax = tab[i];
    }

    gettimeofday(&t1, NULL);

    printf("Taille SIZE = %d\n", SIZE);
    printf("min = %d, max = %d\n", gmin, gmax);
    printf("Temps recherche (s) = %.6f\n", elapsed(t0, t1));

    free(tab);
    return EXIT_SUCCESS;
}
