#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <limits.h>
#include <pthread.h>

#ifndef SIZE
#define SIZE 10000000
#endif

int *tab;

typedef struct {
    int tid;
    size_t start, end;
} thread_arg_t;

int gmin = INT_MAX;
int gmax = INT_MIN;
pthread_mutex_t gmutex = PTHREAD_MUTEX_INITIALIZER;

double elapsed(struct timeval a, struct timeval b) {
    return (b.tv_sec - a.tv_sec) + (b.tv_usec - a.tv_usec) / 1e6;
}

void *thread_func(void *arg) {
    thread_arg_t *t = (thread_arg_t*)arg;
    int lmin = INT_MAX, lmax = INT_MIN;
    for (size_t i = t->start; i < t->end; ++i) {
        int v = tab[i];
        if (v < lmin) lmin = v;
        if (v > lmax) lmax = v;
    }

    pthread_mutex_lock(&gmutex);

    if (lmin < gmin) {
      gmin = lmin;
    }

    if (lmax > gmax) {
      gmax = lmax;
    }

    pthread_mutex_unlock(&gmutex);

    return NULL;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int nthreads = atoi(argv[1]);
    if (nthreads <= 0) nthreads = 1;

    tab = malloc((size_t)SIZE * sizeof(int));
    if (!tab) {
      perror("malloc");
      return EXIT_FAILURE;
    }

    srand((unsigned)time(NULL));
    for (size_t i = 0; i < SIZE; ++i) {
      tab[i] = rand();
    }

    pthread_t *threads = malloc(nthreads * sizeof(pthread_t));
    thread_arg_t *args = malloc(nthreads * sizeof(thread_arg_t));
    if (!threads || !args) {
      perror("alloc");
      return EXIT_FAILURE;
    }

    size_t base = SIZE / nthreads;
    size_t rem = SIZE % nthreads;
    size_t offset = 0;

    struct timeval t0, t1;
    gettimeofday(&t0, NULL);

    for (int t = 0; t < nthreads; ++t) {
        size_t chunk = base + (t < (int)rem ? 1 : 0);
        args[t].tid = t;
        args[t].start = offset;
        args[t].end = offset + chunk;
        pthread_create(&threads[t], NULL, thread_func, &args[t]);
        offset += chunk;
    }

    int gmin = INT_MAX, gmax = INT_MIN;
    for (int t = 0; t < nthreads; ++t) {
        pthread_join(threads[t], NULL);
    }

    gettimeofday(&t1, NULL);

    printf("SIZE=%d, threads=%d\n", SIZE, nthreads);
    printf("min=%d, max=%d\n", gmin, gmax);
    printf("Temps recherche (s) = %.6f\n", elapsed(t0, t1));

    free(threads);
    free(args);
    free(tab);
    return EXIT_SUCCESS;
}
