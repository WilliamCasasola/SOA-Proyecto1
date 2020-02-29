/*
 1. Se deben mantener varios buffers? En el enunciado se menciona que se debe especificar el nombre
    entonces, ¿se debe dar la posibilidad de crear varios?
 2. El buffer es el mmap?
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/mman.h>

#include "definitions.h"
#include "creator.h"

// TODO: read from terminal
double mean;
int producerCount;
int consumerCount;

struct message buffer[10];
struct producer producers[10];
struct consumer comsumers[10];

pthread_mutex_t pushMessage;
pthread_mutex_t takeMessage;
pthread_mutex_t addConsumer;
pthread_mutex_t addProducer;

pthread_cond_t bufferBlocked;
pthread_cond_t bufferEmpty;

/*
TODO:
    - Read instructions to create consumer/producer
    - Invoque creator
*/
int main(int argc, char** argv) {
    printf("%s\n","Main");
}
