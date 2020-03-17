#include <sys/mman.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <signal.h>
#include <math.h>
#include <time.h> 
#include <string.h>
#include "shared.h"


char* bufferName;
int bufferLength = 10;
int metadataSize;
int semaphoresSize;
int bufferSize;
int totalSize;
char lConsume[10];
char lProduce[10];
char lMetadata[10];

void parseAndValidateParams();
void create();
void randStr(char *dest);

int main(int argc, char** argv) {    
    parseAndValidateParams(argc, argv);
    metadataSize = sizeof(struct Metadata);
    semaphoresSize = sizeof(struct Semaphores);
    bufferSize = bufferLength * sizeof(struct Message);
    totalSize = metadataSize + semaphoresSize + bufferSize;
    srand(time(NULL));
    create();
    exit(EXIT_SUCCESS);
}

void parseAndValidateParams(int argc, char** argv){
    int withErrors = 0;
    if(argc >= 2){
        if(strcmp(argv[1], "-h") == 0){
            printf("\n%s\n","Application receives up to two parameters, with the first being mandatory:\n\t Buffer Name: char*\n\t Buffer Length: +int\n");
            exit(EXIT_SUCCESS);
        }else{
            bufferName = argv[1];
        }
        if(argc == 3 ){
            bufferLength = atoi(argv[2]);
            if(bufferLength <= 0){
                withErrors = 1;
                printf("\n%s\n","Buffer Length must be a positive integer.");
            }
        }
    }else{
        withErrors = 1;
    }
    if(withErrors){
        printf("\n%s\n\n","Parameters type does not match, please use -h to see usage.");
        exit(EXIT_FAILURE);
    }
}

void randStr(char *dest) {
    char charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int i = 10;
    while (i-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}

void create(){
    int sm = shm_open(bufferName, O_CREAT|O_RDWR|O_TRUNC|O_EXCL, S_IRWXO|S_IRWXG|S_IRWXU);
    if(errno == EEXIST){
        printf("\n%s\n\n","The Buffer Name already exists");
        exit(EXIT_FAILURE);
    }else{
        if((ftruncate(sm, totalSize)) == -1){
            perror("---- ftruncate failure\n");
            close(sm);
            shm_unlink(bufferName);
            exit(EXIT_FAILURE);
        }else{
            void* map = mmap(0, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, sm, 0);
            struct Metadata* metadata = (struct Metadata*) (map);
            metadata->bufferLength = bufferLength;
            metadata->pCount = 0;
            metadata->cCount = 0;
            metadata->cIndex = 0;
            metadata->pIndex = 0;
            metadata->queued = 0;
            metadata->terminate = 0;
            struct Semaphores* semaphores = (struct Semaphores*) (map) + metadataSize;
            randStr(semaphores->consume);
            randStr(semaphores->produce);
            randStr(semaphores->metadata);
            strncpy(lConsume, semaphores->consume, 10);
            strncpy(lProduce, semaphores->produce, 10);
            strncpy(lMetadata, semaphores->metadata, 10);
            sem_t* comsumeS = sem_open(lConsume, O_CREAT, S_IRWXO|S_IRWXG|S_IRWXU, 1);
            sem_close(comsumeS);
            sem_t* produceS = sem_open(lProduce, O_CREAT, S_IRWXO|S_IRWXG|S_IRWXU, 1);
            sem_close(produceS);
            sem_t* metadataS = sem_open(lMetadata, O_CREAT, S_IRWXO|S_IRWXG|S_IRWXU, 1);
            sem_close(metadataS);
            munmap(map, totalSize);
            close(sm);
            printf("Shared Memory with name \"%s\" has been created with Buffer Length of %i\n", bufferName, bufferLength);
        }
    }
}