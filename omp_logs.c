#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include "omp_logs.h"

/* [raw]
struct task {
    char* label;
    int size;
    int thread_id;
    int parent_thread_id;
    // TIME
    double start_time;
    double cpu_time_used;
};*/

/* [raw]
struct task_list {
    struct task* t;
    struct task_list* next;
}; */



struct task* new_task(char* label, int size, int thread_id, int parent_thread_id, double start_time, double cpu_time_used) {
    struct task* t = malloc(sizeof(struct task));

    t->label = label;
    t->size = size;
    t->thread_id = thread_id;
    t->parent_thread_id = parent_thread_id;
    t->start_time = start_time;
    t->cpu_time_used = cpu_time_used;

    return t;
}


void print_task(struct task* t) {
    printf("(%s):\n\tCalling Thread: %d\n\tParent Thread: %d\n\tSize: %d\n\tStart Time: %f\n\tUsed Time CPU: %f\n",
            t->label,
            t->thread_id,
            t->parent_thread_id,
            t->size,
            t->start_time,
            t->cpu_time_used);
}

void apply_function(struct task_list* l, void (*f)(struct task* t)) {
    if (l != NULL) {
        f(l->t);
        apply_function(l->next, f);
    }
}

void print_list(struct task_list* l) {
    apply_function(l, print_task);
}


struct task_list* new_list(struct task* t) {
    struct task_list* l = malloc(sizeof(struct task_list));
    l->t = t;
    l->next = NULL;
    return l;
}

/* We push in head because it does not really matter here */
void push(struct task_list** l, struct task* t) {
    struct task_list* new_cell = new_list(t);
    if (*l == NULL) {
        *l = new_cell;
    } else {
        new_cell->next = *l;
        *l = new_cell;
    }
}

void free_list(struct task_list* l) {
    if (l != NULL) {
        free_list(l->next);
        free(l->t);
        free(l);
    }
}

void log_task(struct task_list** l, char* label, int size, int parent_thread,void (*f)(void* args), void* args) {
    int thread_id = omp_get_thread_num();
    clock_t start, end;
    double cpu_time_used;
    // Get time
    start = clock();
    f(args);
    end = clock();
    // Get time
    cpu_time_used = ((double) (end - start));// / CLOCKS_PER_SEC;
    push(l, new_task(label, size, thread_id, parent_thread, (double) start, cpu_time_used));
}

struct task_list** get_tasks_per_thread(struct task_list* l) {
    int threads_involved = omp_get_max_threads();
    struct task_list** tasks_per_thread = malloc(threads_involved * sizeof(struct task_list*));

    struct task_list* current = l;

    while (current != NULL) {
        push(&(tasks_per_thread[current->t->thread_id]), current->t);
        current = current->next;
    }
    return tasks_per_thread;
}
