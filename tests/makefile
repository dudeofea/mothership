all:
	gcc -Wall -std=c99 -c ../engine.c
	gcc -Wall -std=c99 -c unit_tests.c
	gcc -o tests unit_tests.o engine.o -lcunit -lm
	rm -f *.o
clean:
	rm -f *.o tests
