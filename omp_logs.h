#ifndef _OMP_LOGS_H_

#define _OMP_LOGS_H_

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include "omp_logs.h"

struct task {
    char* label;
    int size;
    int thread_id;
    int parent_thread_id;
    // TIME
    double start_time;
    double cpu_time_used;
};


struct task_list {
    struct task* t;
    struct task_list* next;
};

struct task* new_task(char* label, int size, int thread_id, int parent_thread_id, double start_time, double cpu_time_used) ;

void print_task(struct task* t) ;

void apply_function(struct task_list* l, void (*f)(struct task* t)) ;

void print_list(struct task_list* l) ;

struct task_list* new_list(struct task* t) ;

void push(struct task_list** l, struct task* t) ;

void free_list(struct task_list* l) ;

void log_task(struct task_list** l, char* label, int size, int parent_thread,void (*f)(void* args), void* args) ;

struct task_list** get_tasks_per_thread(struct task_list* l) ;

#endif