# OMP-Logs
Logging lib for OpenMP

## Compilation

You can compile the lib by doing ```make```

Then, you can use this lib from anywhere.
The builds will look like:
```
gcc file.c -o file.o -L/path -lomp_logs
```
where ```path``` is the path to the directory where you built the lib.

## Usage

### 1) Import the header file

```c
#include "path/to/the/file/omp_logs.h"
```

### 2) Create a empty list of tasks

This list needs to be accessible by every call of the function ```log_task```.

You can put the list as a global variable to be sure.

```c
struct task_list* list = NULL;
```

### 3) Log some tasks !

The function to log a task is:

```c
void log_task(struct task_list** l, char* label, int info, int parent_id, void (*f)(void* args), void* args)
```
 * ```l``` is the list of tasks that we created just before
 * ```label``` is the label that you could give to the task
 * ```info``` is a placeholder for some info that you could want to have (i.e. size of an array, index, ...)
 * ```parent_id``` is the id of the thread that is the parent of the thread running the task. You can get this id by doing ```omp_get_thread_num()``` before creating the task
 * ```f``` is the function that represents the task. It **must** be of type ```void (*f)(void* args)```
 * ```args``` is the argument(s) of ```f```
 
 I invite you to look at the examples ```mergesort.c``` and ```for_policies.c```
 
 ### 4) See your logs !
 
 When all the parallel stuff is done, you can get the logs in a ```svg``` file:
 
 ```c
 tasks_to_svg(l, "filename.svg");
 ```
 Even if this is a ```svg``` file, you'll have to open it with your browser.
 
 Here is an example (For the full experience: click on the image, and then, right click -> "View Image"):
 
 ![example](https://github.com/GuilloteauQ/omp-logs/blob/master/mergesort.svg)
 
 Every line represents a thread across time.
 
 Every rectangle is a task.
 
 You can move your mouse on it to see some info:
 
  * The ```label``` of the task
  * The time it took to execute the task
  * The ```info``` of the task
 
 
## Disclaimer

**This work is highly inspired from** [wagnerf42's rayon-logs](https://github.com/wagnerf42/rayon-logs)
 
