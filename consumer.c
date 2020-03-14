//External includes
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


//Local includes
#include "definitions.h"

struct datum *datum_pointer;
struct consumer consumer;

//Initial point of execution
int main(int argc, char** argv){
   set_up(argc, argv);

}

// Initialize variables of the consumer struct
void set_up(int argc, char** argv){
    if (argc > 2 & argc < 4){
        consumer.pid = getpid();
        consumer.avg = apply_exponential_function(atof(argv[2]));
        consumer.buffer_name = argv[3];
    }
    else{
        printf("\n%s\n","Vehicle Speed must be a positive integer.");
        exit(EXIT_FAILURE);
    }
}

//Apply an exponential distribution
double apply_exponential_function(double avg){
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1- u) / avg;
}

void check_block(){

}

void consume(){


}

// Prints statistics of the consumer
void getStatistics(){
    printf("\n%s\n","Statistics of consumer with process id: %d\n  Total of messages processed: %d \n Total of time waiting: %d \n Total of time blocked: %d" , 
    consumer.pid, consumer.total_messages, consumer.total_time_waiting, consumer.total_time_blocked);
}