CC=gcc
CFLAGS=-pthread -lm -lrt -g
EXTRA_CFLAGS=-Wall -Wextra
UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	CCFLAGS += -lrt
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
