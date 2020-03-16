CC=gcc
CFLAGS=-pthread -lm -g
EXTRA_CFLAGS=-Wall -Wextra
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
	CFLAGS += -lrt
endif

all: creator finalizer consumer producer



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
