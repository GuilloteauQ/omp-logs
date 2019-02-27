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

/* [raw]
struct svg_file {
    FILE* f;
    int height;
    int width;
}; */

void svg_header(struct svg_file* s_f) {
    fprintf(s_f->f, "<svg width=\"%d\" height=\"%d\">\n", s_f->width, s_f->height);
}

void svg_footer(struct svg_file* s_f) {
    fprintf(s_f->f, "</svg>");
}


struct svg_file* new_svg_file(char* filename, int width, int height) {
    struct svg_file* s_f = malloc(sizeof(struct svg_file));

    s_f->f = fopen(filename, "w");
    s_f->height = height;
    s_f->width = width;

    svg_header(s_f);

    return s_f;
}

void close_svg(struct svg_file* s_f) {
    svg_footer(s_f);
    fclose(s_f->f);
    free(s_f);
}

void svg_line(struct svg_file* s_f, float x1, float y1, float x2, float y2, char* style) {
    fprintf(s_f->f, "<line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" style=\"%s\"/>\n", x1, y1, x2, y2, style);
}

void svg_rect(struct svg_file* s_f, float x, float y, float width, float height, char* color) {
    fprintf(s_f->f, "<rect x=\"%f\" y=\"%f\" width=\"%f\" height=\"%f\" fill=\"%s\" stroke=\"black\"/>\n", x, y, width, height, color);
}


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

int get_size(struct task_list* l) {
    if (l != NULL) {
        return 1 + get_size(l->next);
    } else {
        return 0;
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

double get_min_time(struct task_list* l) {
    double current_min = l->t->start_time;
    struct task_list* current = l;

    while (current != NULL) {
        if (current->t->start_time < current_min) {
            current_min = current->t->start_time;
        }
        current = current->next;
    }
    return current_min;
}

double remap_time_and_get_max_time(struct task_list*  l, double min_time) {
    double current_max = 0;
    struct task_list* current = l;

    while (current != NULL) {
        current->t->start_time = current->t->start_time - min_time;
        double challenger = current->t->start_time + current->t->cpu_time_used;
        if (challenger > current_max) {
            current_max = challenger;
        }
        current = current->next;
    }

    return current_max;
}

struct task_list** get_tasks_per_thread(struct task_list* l) {
    int threads_involved = omp_get_max_threads();
    struct task_list** tasks_per_thread = malloc(threads_involved * sizeof(struct task_list*));
    for (int i = 0; i < threads_involved; i++) {
        tasks_per_thread[i] = NULL;
    }

    struct task_list* current = l;

    while (current != NULL) {
        push(&(tasks_per_thread[current->t->thread_id]), current->t);
        current = current->next;
    }
    return tasks_per_thread;
}

void update_used_time(struct task_list* l) {
    int size = get_size(l);
    double* start_times = malloc(size * sizeof(double));
    double* used_times = malloc(size * sizeof(double));

    struct task_list* current = l;
    int i = 0;
    while (current != NULL) {
        start_times[i] = current->t->start_time;
        used_times[i] = current->t->cpu_time_used;
        i++;
        current = current->next;
    }

    current = l;
    i = 0;

    while (current != NULL) {
        for (int j = 0; j < size; j++) {
            if (i != j && start_times[i] < start_times[j] && start_times[j] < start_times[i] + used_times[i]) {
                current->t->cpu_time_used = start_times[j] - start_times[i];
            }
        }
        i++;
        current = current->next;
    }
}


// Maps a time in the SVG frame
float get_x_position(double time, double max_time, int width) {
    return (float) (time * (double) width) / ((float) max_time);
}

void thread_to_svg(struct task_list* l, struct svg_file* s_f, double max_time, float begin_y, float task_height, char* color) {
    update_used_time(l);
    // Draw the line for time
    svg_line(s_f, 0, begin_y, s_f->width, begin_y, "stroke:rgb(255,0,0)");

    struct task_list* current = l;

    while (current != NULL) {
        float x = get_x_position(current->t->start_time, max_time, s_f->width);
        float rect_width = get_x_position(current->t->cpu_time_used, max_time, s_f->width);
        svg_rect(s_f, x, begin_y - task_height / 2.0, rect_width, task_height, color);
        current = current->next;
    }
}

char* thread_color(int i) {
    switch (i) {
        case 0:
            return "red";
        case 1:
            return "blue";
        case 2:
            return "green";
        case 3:
            return "yellow";
        default:
            return thread_color(i % 4);
    }
}

void tasks_to_svg(struct task_list* l, char* filename) {
    int width = 2000;
    int height = 800;
    struct svg_file* s_f = new_svg_file(filename, width, height);
    int thread_pool_size = omp_get_max_threads();
    float h = height / (float) (thread_pool_size + 1);

    double max_time = remap_time_and_get_max_time(l, get_min_time(l));
    printf("Max time : %f\n", max_time);
    struct task_list** tasks_per_thread = get_tasks_per_thread(l);

    for (int i = 0; i < thread_pool_size; i++) {
        printf("Thread %d\n", i);
        thread_to_svg(tasks_per_thread[i], s_f, max_time, (i + 1) * h, 3 * h / 4, thread_color(i));
        print_list(tasks_per_thread[i]);
    }


    close_svg(s_f);
}
