all:
	gcc main.c funcs.c -Wno-incompatible-pointer-types -lreadline
	#gcc test.c ub_funcs.c myString.c ub_auxiliaries.c ub_allocs.c -lreadline
	#gcc -Wall -pedantic -Werror test.c ub_funcs.c myString.c ub_auxiliaries.c ub_allocs.c -lreadline
#EXES= bash cd
#CC=gcc
#CFLAGS= -std=gnu11 -Wall -pedantic -Werror -fsanitize=address

#all: $(EXES)

#bash: bash.o cd.o
		#$(CC) $(CFLAGS) -o $@ $^

#bash.o: mybash.c
		#$(CC) $(CFLAGS) -o $@ -c $^

#cd.o: mycd.c
		#$(CC) $(CFLAGS) -o $@ -c $^

#clean:
		#rm -f *.o $()

#.PHONY: clean all