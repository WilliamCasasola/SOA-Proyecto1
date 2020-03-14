CC=gcc
CFLAGS=-pthread -lm
EXTRA_CFLAGS=-Wall -Wextra

# TODO: Add other commands
all: creator finalizer consumer

creator: creator.c
	$(CC) creator.c -o creator $(CFLAGS) $(EXTRA_CFLAGS)

consumer: consumer.c
	$(CC) consumer.c -o consumer $(CFLAGS) $(EXTRA_CFLAGS)

producer: producer.c
	$(CC) producer.c -o producer $(CFLAGS) $(EXTRA_CFLAGS)

finalizer: finalizer.c 
	$(CC) finalizer.c -o finalizer $(CFLAGS) $(EXTRA_CFLAGS)

clean :
	rm creator consumer producer finalizer
