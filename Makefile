CC=gcc
CFLAGS=-pthread -lm -lrt
EXTRA_CFLAGS=-Wall -Wextra

# TODO: Add other commands
all: creator

creator: creator.c shared_area.c
	$(CC) creator.c shared_area.c -o creator $(CFLAGS) $(EXTRA_CFLAGS)

consumer: consumer.c shared_area.c
	$(CC) consumer.c shared_area.c -o consumer $(CFLAGS) $(EXTRA_CFLAGS)

producer: producer.c shared_area.c
	$(CC) producer.c shared_area.c -o producer $(CFLAGS) $(EXTRA_CFLAGS)

finalizer: finalizer.c shared_area.c
	$(CC) finalizer.c shared_area.c -o finalizer $(CFLAGS) $(EXTRA_CFLAGS)

clean :
	rm creator consumer producer
