CC=gcc
CFLAGS=-pthread -lm -lrt
EXTRA_CFLAGS=-Wall -Wextra

main: main.c
	$(CC) main.c definitions.h -o main $(CFLAGS) $(EXTRA_CFLAGS)

creator_2: creator_2.c
	$(CC) creator_2.c definitions.h -o creator_2 $(CFLAGS) $(EXTRA_CFLAGS)

creator: creator.c
	$(CC) creator.c definitions.h -o creator $(CFLAGS) $(EXTRA_CFLAGS)

clean :
	rm main
