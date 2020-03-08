CC=gcc
CFLAGS=-pthread -lm -lrt
EXTRA_CFLAGS=-Wall -Wextra

main: main.c
	$(CC) main.c definitions.h -o main $(CFLAGS) $(EXTRA_CFLAGS)

clean :
	rm main
