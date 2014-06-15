CC=gcc
CFLAGS=-Wall -std=c99

all:
	$(CC) $(CFLAGS) -c engine.c -o engine.o
	$(CC) $(CFLAGS) -c effects.c -o effects.o
	$(CC) $(CFLAGS) -c motherGUI.c -o motherGUI.o
	$(CC) $(CFLAGS) -c mothership.c -o mothership.o
	$(CC) $(CFLAGS) -o mothership *.o -lm -lfftw3 -lncurses -ljack
	rm -rf *.o
old:
	$(CC) $(CFLAGS) -c effects.c -o effects.o
	$(CC) $(CFLAGS) -c jack_client.c -o jack_client.o -ljack -lfftw3 -lm
	$(CC) $(CFLAGS) -c ncurses.c -o ncurses.o -lncurses
	$(CC) $(CFLAGS) -c engine.c -o engine.o
	#$(CC) $(CFLAGS) -c mothership.c -o mothership.o
	#$(CC) $(CFLAGS) -o engine_test mothership.o engine.o effects.o -lm -lfftw3 -lncurses
	$(CC) $(CFLAGS) -o jack_client jack_client.o engine.o effects.o ncurses.o -ljack -lfftw3 -lm -lncurses
	rm -rf *.o
run:
	jackd -P80 -p16 -t2000 -d alsa -d hw:0 -p 1024 -n 2 -r 44100 -s &
clean:
	rm -rf *.o jack_client.o jack_client
