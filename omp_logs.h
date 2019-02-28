#ifndef _OMP_LOGS_H_

#define _OMP_LOGS_H_

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
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


struct task_list {
    struct task* t;
    struct task_list* next;
};

void log_task(struct task_list** l, char* label, int size, int parent_thread,void (*f)(void* args), void* args) ;

void tasks_to_svg(struct task_list* l, char* filename) ;

#endif
