#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include<omp.h>
#include <x86intrin.h>

#include "omp_logs.h"


// The list where tasks will be pushed
struct task_list* l = NULL;

struct data {
    int size;
    int* T;
};

struct data* new_data(int size, int* T) {
    struct data* d = malloc(sizeof(struct data));
    d->size = size;
    d->T = T;
    return d;
}

// Every function that we want to log must be void f(void*)
void merge_data (void* d) {
    struct data* data = (struct data*) d;
    int* T = data->T;
    int size = data->size / 2;

    int i = 0 ;
    int j = size ;
    int k = 0 ;
    int *X = (int *) malloc (2 * size * sizeof(int)) ;
    while ((i < size) && (j < 2*size)) {
        if (T[i] < T [j]) {
            X [k] = T [i] ;
            i = i + 1 ;
        } else {
            X [k] = T [j] ;
            j = j + 1 ;
        }
        k = k + 1 ;
    }

    if (i < size) {
        for (; i < size; i++, k++) {
            X [k] = T [i] ;
        }
    } else {
        for (; j < 2*size; j++, k++) {
            X [k] = T [j] ;
        }
    }
    memcpy (data->T, X, 2*size*sizeof(int)) ;
    free (X) ;
}


// Every function that we want to log must be void f(void*)
void merge_sort_data(void* d) {
    struct data* data = (struct data*) d;
    int thread_id = omp_get_thread_num();
    {
        if (data->size >= 2) {
            int new_size = data->size / 2;
            struct data* d1 = new_data(new_size, data->T);
            struct data* d2 = new_data(new_size, data->T + new_size);
            #pragma omp task
            log_task(&l, "Sort", new_size, thread_id, merge_sort_data, d1);
            #pragma omp task
            log_task(&l, "Sort", new_size, thread_id, merge_sort_data, d2);
            #pragma omp taskwait
            #pragma omp task
            log_task(&l, "Merge", data->size, thread_id, merge_data, d);
        }

    }
}

int main (int argc, char **argv) {
    if (argc != 2) {
        fprintf (stderr, "mergesort N \n") ;
        exit (-1) ;
    }

    int N = 1 << (atoi(argv[1])) ;
    int* X = (int *) malloc (N * sizeof(int)) ;

    for (int i = 0 ; i < N ; i++) {
        X[i] = N - i ;
    }

    struct data* d = new_data(N, X);
    #pragma omp parallel
    {
    #pragma omp single
        // merge_sort_data(d);
        log_task(&l, "Start", N, omp_get_thread_num(), merge_sort_data, (void*) d);
    }
    tasks_to_svg(l, "mergesort.svg");

    free_list(l);
}
