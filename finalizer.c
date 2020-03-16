#include <sys/mman.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <signal.h> 
#include "shared.h"


char* bufferName;
int metadataSize;
int semaphoresSize;
int bufferSize;
int totalSize;
struct stat smInfo;
char lConsume[10];
char lProduce[10];
char lMetadata[10];

void parseAndValidateParams();
void finalizePC();
void finalizeSM();

int main(int argc, char** argv) {    
    parseAndValidateParams(argc, argv);
    metadataSize = sizeof(struct Metadata);
    semaphoresSize = sizeof(struct Semaphores);
    finalizePC();
    finalizeSM();
    exit(EXIT_SUCCESS);
}

void parseAndValidateParams(int argc, char** argv){
    int withErrors = 0;
    if(argc == 2){
        if(strcmp(argv[1], "-h") == 0){
            printf("\n%s\n","Application receives 1 parameter:\n\t Buffer Name: char*\n");
            exit(EXIT_SUCCESS);
        }else{
            bufferName = argv[1];
        }
    }else{
        withErrors = 1;
    }
    if(withErrors){
        printf("\n%s\n\n","Parameters type does not match, please use -h to see usage.");
        exit(EXIT_FAILURE);
    }
}

void finalizeSM(){
    shm_unlink(bufferName);
    sem_unlink(lConsume);
    sem_unlink(lProduce);
    sem_unlink(lMetadata);
    printf("\n\nShared Memory with name \"%s\" has been destroyed.\n\n", bufferName);
}

void finalizePC(){
    int sm;
    if((sm = shm_open(bufferName, O_RDWR, 0)) != -1){
        fstat(sm, &smInfo);
        void* map = mmap(0, smInfo.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, sm, 0);
        struct Metadata* metadata = ((struct Metadata*) map);
        bufferSize = metadata->bufferLength * sizeof(struct Message);
        totalSize = metadataSize + semaphoresSize + bufferSize;
        void * buffer = ((void*) map) + metadataSize + semaphoresSize;
        struct Semaphores* semaphores = ((struct Semaphores*) map) + metadataSize;        
        strncpy(lConsume, semaphores->consume, 10);
        strncpy(lProduce, semaphores->produce, 10);
        strncpy(lMetadata, semaphores->metadata, 10);
        sem_t * metadataS = sem_open(lMetadata, O_RDWR);
        sem_t * produceS = sem_open(lProduce, O_RDWR);
        sem_wait(metadataS);
        metadata->terminate = 1;
        int pCount = metadata->pCount;
        sem_post(metadataS);
        while(pCount > 0){
            kill(-1, SIGCONT);
            sem_wait(metadataS);
            pCount = metadata->pCount;
            sem_post(metadataS);
        }
        int wait = 0;
        sem_wait(metadataS);
        int cCount = metadata->cCount;
        sem_post(metadataS);
        while(cCount > 0){
            sem_wait(metadataS);
            wait = metadata->pIn;
            metadata->pIn++;
            sem_post(metadataS);
            while (wait == metadata->bufferLength){
                sem_wait(metadataS);
                wait = metadata->queued;
                cCount = metadata->cCount;
                if(cCount == 0){
                    wait= -1;
                    sem_post(metadataS);
                }
                else{
                    sem_post(metadataS);
                    raise(SIGSTOP);
                }
            }
            if(cCount > 0){
                sem_wait(produceS);
                struct Message* message = (struct Message*) ((buffer) + ((metadata->pIndex % metadata->bufferLength) * sizeof(struct Message)));
                message->pid = getpid();
                message->datetime = time(NULL);
                message->key = -1;
                message->terminate = 1;            
                metadata->pIndex++;
                printf("\n\nFinalizer created a terminate message.\n\n");
                sem_post(produceS);
                sem_wait(metadataS);
                metadata->queued++;
                metadata->pIn--;
                cCount = metadata->cCount;
                sem_post(metadataS);
                kill(-1, SIGCONT);
            }
        }
        sem_close(metadataS);
        sem_close(produceS);
        munmap(map, totalSize);
        close(sm);
    }else{
        printf("\nCould not open shared memory with name %s.\n\n", bufferName);
        exit(EXIT_FAILURE);
    }
}


