all: lib

omp_logs.o: omp_logs.c
	gcc -O -c omp_logs.c omp_logs.h -fopenmp

lib: omp_logs.o
	ar rcs libomp_logs.a omp_logs.o
	ranlib libomp_logs.a

clean:
	rm -f *.o *.a *.gch
