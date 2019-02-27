#include <stdlib.h>
#include <omp.h>
#include "omp_logs.h"

struct data{
    int x;
    int* sum;
};

void sum(void* d) {
    struct data* data = (struct data*) d;
    *(data->sum) += data->x;
}


int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage %s N\n", argv[0]);
        return 1;
    }
    int N = atoi(argv[1]);
    struct task_list* l = NULL;
    int s = 0;
    #pragma omp parallel for schedule(static) reduction (+:s)
    for (int j = 0; j < N; j++) {
        struct data d = {j, &s};
        log_task(&l, "Sum", j, omp_get_thread_num(), sum, (void*) &d);
    }
    tasks_to_svg(l, "for_static.svg");
    free_list(l);
    l = NULL;


    #pragma omp parallel for schedule(dynamic) reduction (+:s)
    for (int j = 0; j < N; j++) {
        struct data d = {j, &s};
        log_task(&l, "Sum", j, omp_get_thread_num(), sum, (void*) &d);

    }
    tasks_to_svg(l, "for_dynamic.svg");
    free_list(l);
    l = NULL;

    #pragma omp parallel for schedule(guided) reduction (+:s)
    for (int j = 0; j < N; j++) {
        struct data d = {j, &s};
        log_task(&l, "Sum", j, omp_get_thread_num(), sum, (void*) &d);

    }
    tasks_to_svg(l, "for_guided.svg");
    free_list(l);
    return 0;
}
