CC = gcc
LD = gcc

CFLAGS = -Wall -Wextra -O2 -g -std=c99 -fopenmp
LDFLAGS = -L../ -lomp_logs -fopenmp

all: mergesort for_policies

mergesort: mergesort.o
	$(LD) $^ -o $@ $(LDFLAGS)

for_policies: for_policies.o
	$(LD) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f mergesort for_policies *.o

clean_svg:
	rm -f *.svg

super: clean clean_svg
