#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <omp.h>
#include <pthread.h>
#include "omp_logs.h"

/* [raw]
struct task {
    char* label;
    int info;
    int thread_id;
    int parent_thread_id;
    // TIME
    double start_time;
    double cpu_time_used;
    struct task_list* children;
};*/

/* [raw]
struct task_cell {
    struct task* t;
    struct task_cell* next;
};*/


/* [raw]
typedef struct {
    pthread_mutex_t mutex;
    struct task_cell* head;
} task_list;*/

struct svg_file {
    FILE* f;
    int height;
    int width;
    int animated;
};

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// SVG
//
//

/* Write the header of the SVG file */
void svg_header(struct svg_file* s_f) {
    if (s_f->animated) {
        fprintf(s_f->f, "<?xml version=\"1.0\"?>\n<svg viewBox=\"0 0 %d %d\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n", s_f->width, s_f->height);
    } else {
        fprintf(s_f->f, "<svg viewBox=\"0 0 %d %d\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n", s_f->width, s_f->height);
    }
}

/*
 * Write the footer of the SVG file
 * with some JS code taken from https://github.com/wagnerf42/rayon-logs
 * to display the information about each task
 * when the mouse is over the task
 */
void svg_footer(struct svg_file* s_f) {
    if (s_f->animated) {
        fprintf(s_f->f, "<script>\n<![CDATA[\nvar tasks = document.getElementsByClassName('task');\nfor (var i = 0; i < tasks.length; i++) {\nvar tip = document.getElementById('tip_'+i);\ntip.style.display='none';\ntasks[i].tip = tip;\ntasks[i].addEventListener('mouseover', mouseOverEffect);\ntasks[i].addEventListener('mouseout', mouseOutEffect);}\n\nfunction mouseOverEffect() {\nthis.classList.add(\"task-highlight\");\nthis.tip.style.display='block';\n}\n\nfunction mouseOutEffect() {\nthis.classList.remove(\"task-highlight\");\nthis.tip.style.display='none';\n}\n]]>\n</script>\n<style>.task-highlight {fill: #ec008c;opacity: 1;}</style>\n</svg>");
    } else {
        fprintf(s_f->f, "</svg>");
    }
}


/* Return a new SVG structure */
struct svg_file* new_svg_file(char* filename, int width, int height, int animated) {
    struct svg_file* s_f = malloc(sizeof(struct svg_file));

    s_f->f = fopen(filename, "w");
    s_f->height = height;
    s_f->width = width;
    s_f->animated = animated;

    svg_header(s_f);

    return s_f;
}

/* Close the SVG file, but write the footer before */
void close_svg(struct svg_file* s_f) {
    svg_footer(s_f);
    fclose(s_f->f);
    free(s_f);
}

/* Draw a line in the SVG file */
void svg_line(struct svg_file* s_f, float x1, float y1, float x2, float y2, char* style) {
    fprintf(s_f->f, "<line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" style=\"%s\"/>\n", x1, y1, x2, y2, style);
}

/* Write some text in the SVG file */
void svg_text(struct svg_file* s_f, float x, float y, char* color, char* text) {
    fprintf(s_f->f, "<text x=\"%f\" y=\"%f\" fill=\"%s\">%s</text>\n", x, y, color, text);
}

/* Draw a rectangle in the SVG file */
void svg_rect(struct svg_file* s_f, float x, float y, float width, float height, char* color, struct task* task, int counter) {
    fprintf(s_f->f, "<rect class=\"task\" x=\"%f\" y=\"%f\" width=\"%f\" height=\"%f\" fill=\"%s\" stroke=\"black\"/>\n", x, y, width, height, color);
    if (s_f->animated) {
        fprintf(s_f->f, "<g id=\"tip_%d\">\n<rect x=\"%f\" y=\"%f\" width=\"200\" height=\"%f\" fill=\"white\" stoke=\"black\"/>\n<text x=\"%f\" y=\"%f\">[%s] Time: %d, Info: %d, From: %d\n</text></g>\n", counter, x, y - height/4.0, height/4.0, x, y - height/8.0,task->label, (int) task->cpu_time_used, task->info, task->parent_thread_id);
    }
}

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// TASKS
//
//

/* Return a new task */
struct task* new_task(char* label, int info, int thread_id, int parent_thread_id, double start_time, double cpu_time_used) {
    struct task* t = malloc(sizeof(struct task));

    t->label = label;
    t->info = info;
    t->thread_id = thread_id;
    t->parent_thread_id = parent_thread_id;
    t->start_time = start_time;
    t->cpu_time_used = cpu_time_used;
    t->children = NULL;

    return t;
}


/* Print the data inside the task */
void print_task(struct task* t) {
    printf("(%s):\n\tCalling Thread: %d\n\tParent Thread: %d\n\tInfo: %d\n\tStart Time: %f\n\tUsed Time CPU: %f\n",
            t->label,
            t->thread_id,
            t->parent_thread_id,
            t->info,
            t->start_time,
            t->cpu_time_used);
}

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// TASKS LIST
//
//


task_list* task_list_init() {
    task_list* l = malloc(sizeof(task_list));
    pthread_mutex_init(&(l->mutex), NULL);
    l->head = NULL;
    return l;
}

/* Return a new list with a single task inside */
struct task_cell* new_cell(struct task* t) {
    struct task_cell* l = malloc(sizeof(struct task_cell));
    l->t = t;
    l->next = NULL;
    return l;
}

/* Apply recursively a function to every task in the task list */
void apply_function(struct task_cell* l, void (*f)(struct task* t)) {
    if (l != NULL) {
        f(l->t);
        apply_function(l->next, f);
    }
}

/* Print the list */
void print_list(task_list* l) {
    apply_function(l->head, print_task);
}



/*
 * Push a task in head of the list
 * We push in head because it does not really matter here
 */
void push_cell(struct task_cell** l, struct task* t) {
    struct task_cell* new = new_cell(t);
    if (*l == NULL) {
        *l = new;
    } else {
        new->next = *l;
        *l = new;
    }
}

void push(task_list* l, struct task* t) {
    pthread_mutex_lock(&(l->mutex));
    push_cell(&(l->head), t);
    pthread_mutex_unlock(&(l->mutex));
}

/* Return the size of the list */
int get_size_head(struct task_cell* l) {
    if (l != NULL) {
        return 1 + get_size_head(l->next);
    } else {
        return 0;
    }
}

int get_size(task_list* l) {
    return get_size_head(l->head);
}

/* Free recursively the list */
void free_cell(struct task_cell* l) {
    if (l != NULL) {
        free_cell(l->next);
        free(l->t);
        free(l);
    }
}

void free_list(task_list* l) {
    free_cell(l->head);
    free(l);
}

/*
 * Log the task
 * Compute the time it took to achieve the task
 * Push the new task to the task list
 */
void log_task(task_list** l, char* label, int size, int parent_thread,void (*f)(void* args), void* args) {
    int thread_id = omp_get_thread_num();
    clock_t start, end;
    double cpu_time_used;

    start = clock();
    f(args);
    end = clock();

    cpu_time_used = ((double) (end - start));
    if (*l == NULL) {
        *l = task_list_init();
    }
    push(*l, new_task(label, size, thread_id, parent_thread, (double) start, cpu_time_used));
}

/* Return the minimum starting time in the list */
double get_min_time(task_list* l) {
    double current_min = l->head->t->start_time;
    struct task_cell* current = l->head;

    while (current != NULL) {
        if (current->t->start_time < current_min) {
            current_min = current->t->start_time;
        }
        current = current->next;
    }
    return current_min;
}

/*
 * Substract the minimum starting time to every starting time
 * compute the max ending time, and return it
 */
double remap_time_and_get_max_time(task_list*  l, double min_time) {
    double current_max = 0;
    struct task_cell* current = l->head;

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

/*
 * From the list of all the tasks, split this list into
 * a list per thread, where each new list has only tasks
 * that the associated thread has done
 */
struct task_cell** get_tasks_per_thread(task_list* l) {
    int threads_involved = omp_get_max_threads();
    struct task_cell** tasks_per_thread = malloc(threads_involved * sizeof(struct task_cell*));
    for (int i = 0; i < threads_involved; i++) {
        tasks_per_thread[i] = NULL;
    }

    struct task_cell* current = l->head;

    while (current != NULL) {
        push_cell(&(tasks_per_thread[current->t->thread_id]), current->t);
        current = current->next;
    }
    return tasks_per_thread;
}

/*
 * In the case of recursive tasks, we can have a thread
 * starting a new task when it was still busy working on an other task
 * In this case we say that the previous task is done.
 * So we will update the time it actually used
 */
void update_used_time(struct task_cell* l) {
    int size = get_size_head(l);
    double* start_times = malloc(size * sizeof(double));
    double* used_times = malloc(size * sizeof(double));

    struct task_cell* current = l;
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
    free(start_times);
    free(used_times);
}


/* Maps a time in the SVG frame */
float get_x_position(double time, double max_time, float begin_x, float end_x) {
    return time * (end_x - begin_x) / ((float) max_time) + begin_x;
}

/* Draw all the content for a single thread */
void thread_to_svg(struct task_cell* l, struct svg_file* s_f, double max_time, float begin_x, float end_x, float begin_y, float task_height, char* color, int* counter, int thread_id) {
    update_used_time(l);
    struct task_cell* current = l;

    // Draw the line for time
    svg_line(s_f, begin_x, begin_y, end_x, begin_y, "stroke:rgb(0,0,0);stroke-width:3");
    // Write the name of the thread
    char* name = malloc(sizeof(char) * 9);
    sprintf(name, "Thread %d", thread_id);
    svg_text(s_f, (s_f->width - end_x)/2 + end_x, begin_y, "black", name);
    free(name);


    while (current != NULL) {
        float x = get_x_position(current->t->start_time, max_time, begin_x, end_x);
        float rect_width = get_x_position(current->t->cpu_time_used, max_time, begin_x, end_x);
        svg_rect(s_f, x, begin_y - task_height / 2.0, rect_width, task_height, color, current->t, *counter);
        current = current->next;
        (*counter)++;
    }
}

/* For each thread id, we assign a color */
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
        case 4:
            return "pink";
        case 5:
            return "orange";
        case 6:
            return "tan";
        case 7:
            return "aquamarine";
        default:
            return thread_color(i % 8);
    }
}


/*
 * Takes the list of all the tasks, and a filename,
 * creates a svg file
 * split the tasks per thread
 * and draw the tasks for every thread
 */
void tasks_to_svg(task_list* l, char* filename, int animated) {
    int width = 2000;
    int height = 800;
    struct svg_file* s_f = new_svg_file(filename, width, height, animated);
    int thread_pool_size = omp_get_max_threads();
    float h = height / (float) (thread_pool_size + 1);
    float begin_x = 0.0;
    float end_x = (float) width - 300.0;

    double max_time = remap_time_and_get_max_time(l, get_min_time(l));
    struct task_cell** tasks_per_thread = get_tasks_per_thread(l);
    int counter = 0;

    for (int i = 0; i < thread_pool_size; i++) {
        thread_to_svg(tasks_per_thread[i], s_f, max_time, begin_x, end_x, (i + 1) * h, 3 * h / 4, thread_color(i), &counter, i);
        free_cell(tasks_per_thread[i]);
    }


    close_svg(s_f);
    free(tasks_per_thread);
    free(l);
}
