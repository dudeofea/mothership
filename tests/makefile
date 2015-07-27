all:
	gcc -Wall -std=c99 -c ../engine.c
	gcc -Wall -std=c99 -c unit_tests.c
	gcc -Wall -std=gnu99 -c perf_tests.c
	gcc -o tests unit_tests.o engine.o -lcunit -lm
	gcc -o perf_tests perf_tests.o engine.o
	rm -f *.o
clean:
	rm -f *.o tests
