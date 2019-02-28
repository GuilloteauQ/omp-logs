all: mergesort for_policies


mergesort: mergesort.o
	gcc -fopenmp -o mergesort mergesort.o -L../ -lomp_logs

mergesort.o: mergesort.c
	gcc -c -O2 -fopenmp mergesort.c

for_policies: for_policies.o
	gcc -fopenmp -o for_policies for_policies.o -L../ -lomp_logs

for_policies.o: for_policies.c
	gcc -c -O2 -fopenmp for_policies.c


clean:
	rm -f mergesort for_policies *.o

clean_svg:
	rm -f *.svg

super: clean clean_svg
