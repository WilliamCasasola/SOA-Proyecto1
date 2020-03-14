#include <sys/mman.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <signal.h> 
#include "shared.h"


char* bufferName;
int alive;
int malicious;
double mean;
int consumed;
int metadataSize;
int semaphoresSize;
int bufferSize;
int totalSize;
struct stat smInfo;

void parseAndValidateParams();
void consume();
void finalize();
double expDist(double lambda);

int main(int argc, char** argv) {    
    parseAndValidateParams(argc, argv);
    metadataSize = sizeof(struct Metadata);
    semaphoresSize = sizeof(struct Semaphores);
    srand(time(NULL));
    consume();
    exit(EXIT_SUCCESS);
}

void parseAndValidateParams(int argc, char** argv){
    int withErrors = 0;
    mean = 0.25;
    consumed = 0;
    alive = 1;
    malicious = 0;
    if(argc >= 2){
        if(strcmp(argv[1], "-h") == 0){
            printf("\n%s\n","Application receives up to two parameters, with the first being mandatory:\n\t Buffer Name: char*\n\t Consumer Mean: +double\n");
            exit(EXIT_SUCCESS);
        }else{
            bufferName = argv[1];
        }
        if(argc == 3 ){
            mean = atof(argv[3]);
            if(mean <= 0){
                withErrors = 1;
                printf("\n%s\n","Consumer Mean must be a positive double.");
            }
        }
    }else{
        withErrors = 1;
    }
    if(withErrors){
        printf("\n%s\n\n","Parameters type does not match, please use -h to see usage.");
    } 
}

double expDist(double lambda) {
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1- u) / lambda;
}

void consume(){
    int sm;
    if((sm = shm_open(bufferName, O_RDWR, 0)) != -1){
        fstat(sm, &smInfo);
        void* map = mmap(0, smInfo.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, sm, 0);
        struct Metadata* metadata = ((struct Metadata*) map);
        bufferSize = metadata->bufferLength * sizeof(struct Message);
        totalSize = metadataSize + semaphoresSize + bufferSize;
        struct Semaphores* semaphores = ((struct Semaphores*) map) + metadataSize;
        void * buffer = ((void*) map) + metadataSize + semaphoresSize;
        int terminate = 0;
        sem_t * metadataS = sem_open(semaphores->metadata, O_RDWR);
        sem_t * consumeS = sem_open(semaphores->consume, O_RDWR);
        sem_wait(metadataS);
        terminate = metadata->terminate;
        if(!terminate){
            metadata->cCount++;
        }
        sem_post(metadataS);
        if(terminate){
            printf("The Shared Memory has been marked to terminate, can't create new Consumer\n");
            sem_close(metadataS);
            sem_close(consumeS);
            munmap(map, totalSize);
            close(sm);
        }else{
            int wait = 0;
            while(alive){
                sleep(expDist(mean));
                sem_wait(metadataS);
                wait = metadata->queued;
                sem_post(metadataS);
                sem_wait(consumeS);
                while(wait == 0){
                    raise(SIGSTOP);
                    sem_wait(metadataS);
                    wait = metadata->queued;
                    sem_post(metadataS);
                }
                printf("Consuming\n");
                struct Message* message = ((struct Message*) buffer) + ((metadata->cIndex % metadata->bufferLength) * sizeof(struct Message));
                printf("\n\nConsumer with id %i consumed a message from process with id %i\n\n", getpid(), message->pid);
                if(message->terminate || message->key == (getpid()%5)){
                    alive = 0;
                    if(message->key == (getpid()%5)){
                        malicious = 1;
                    }
                }
                metadata->cIndex++;
                metadata->queued--;
                consumed++;
                sem_post(consumeS);
                kill(-1, SIGCONT);
            }
            sem_close(metadataS);
            sem_close(consumeS);
            finalize();
            munmap(map, totalSize);
            close(sm);
        }
        
    }else{
        printf("\nCould not open shared memory with name %s.\n\n", bufferName);
        exit(EXIT_FAILURE);
    }
}

void finalize(){
    if(malicious){
        printf("\n\nConsumer with process is %i has received a malicious message, it will finalize. \n\tConsumed Messages: %i\n\n", getpid(), consumed);    
    }else{
        printf("\n\nConsumer with process is %i has been ordered to finalize. \n\tConsumed Messages: %i\n\n", getpid(), consumed);    
    }   
}


