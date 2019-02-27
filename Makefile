all: mergesort for_policies


mergesort: mergesort.o omp_logs.c
	gcc -fopenmp -o mergesort mergesort.o omp_logs.c

mergesort.o: mergesort.c
	gcc -c -O2 -fopenmp mergesort.c

for_policies: for_policies.o omp_logs.c
	gcc -fopenmp -o for_policies for_policies.o omp_logs.c

for_policies.o: for_policies.c
	gcc -c -O2 -fopenmp for_policies.c


clean:
	rm -f mergesort for_policies *.o
