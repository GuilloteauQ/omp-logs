#ifndef _OMP_LOGS_H_

#define _OMP_LOGS_H_

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <omp.h>

struct task {
    char* label;
    int info;
    int thread_id;
    int parent_thread_id;
    double start_time;
    double cpu_time_used;
    struct task_list* children;
};

struct task_cell {
    struct task* t;
    struct task_cell* next;
};

typedef struct {
    pthread_mutex_t mutex;
    struct task_cell* head;
} task_list;


void log_task(task_list** l, char* label, int size, int parent_thread,void (*f)(void* args), void* args) ;

task_list* task_list_init() ;

void tasks_to_svg(task_list* l, char* filename, int animated) ;

#endif
