#ifndef _OMP_LOGS_H_

#define _OMP_LOGS_H_

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <omp.h>
#include "omp_logs.h"

struct task {
    char* label;
    int info;
    int thread_id;
    int parent_thread_id;
    // TIME
    double start_time;
    double cpu_time_used;
    struct task_list* children;
};


struct task_list {
    struct task* t;
    struct task_list* next;
};


struct svg_file {
    FILE* f;
    int height;
    int width;
};

void svg_header(struct svg_file* s_f) ;

void svg_footer(struct svg_file* s_f) ;

struct svg_file* new_svg_file(char* filename, int width, int height) ;

void close_svg(struct svg_file* s_f) ;

void svg_line(struct svg_file* s_f, float x1, float y1, float x2, float y2, char* style) ;

void svg_text(struct svg_file* s_f, float x, float y, char* color, char* text) ;

void svg_rect(struct svg_file* s_f, float x, float y, float width, float height, char* color, struct task* task, int counter) ;

struct task* new_task(char* label, int info, int thread_id, int parent_thread_id, double start_time, double cpu_time_used) ;

void print_task(struct task* t) ;

void apply_function(struct task_list* l, void (*f)(struct task* t)) ;

void print_list(struct task_list* l) ;

struct task_list* new_list(struct task* t) ;

void push(struct task_list** l, struct task* t) ;

int get_size(struct task_list* l) ;

void free_list(struct task_list* l) ;

void log_task(struct task_list** l, char* label, int size, int parent_thread,void (*f)(void* args), void* args) ;

double get_min_time(struct task_list* l) ;

double remap_time_and_get_max_time(struct task_list*  l, double min_time) ;

struct task_list** get_tasks_per_thread(struct task_list* l) ;

void update_used_time(struct task_list* l) ;

float get_x_position(double time, double max_time, float begin_x, float end_x) ;

void thread_to_svg(struct task_list* l, struct svg_file* s_f, double max_time, float begin_x, float end_x, float begin_y, float task_height, char* color, int* counter) ;

char* thread_color(int i) ;

void tasks_to_svg(struct task_list* l, char* filename) ;

#endif