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
#include <stdbool.h>

#include "shared.h"

char *bufferName;
int alive;
int malicious;
int metadataSize;
int semaphoresSize;
int bufferSize;
int totalSize;
char lConsume[10];
char lProduce[10];
char lMetadata[10];
double mean;
clock_t before;
clock_t difference;

struct stat smInfo;
struct producerConsumerStats *pStats;

void parseAndValidateParams();
void produce();
double expDist(double lambda);
bool killProducer(int sm, void *map, sem_t *produceS, struct Metadata *metadata, sem_t *metadataS, bool);
void getStatistics();

int main(int argc, char **argv)
{
    parseAndValidateParams(argc, argv);
    metadataSize = sizeof(struct Metadata);
    semaphoresSize = sizeof(struct Semaphores);    
    srand(time(NULL));
    produce();
    exit(EXIT_SUCCESS);
}

//TODO: Same method as consumer
void parseAndValidateParams(int argc, char **argv)
{
    int withErrors = 0;
    mean = 0.25;
    pStats = malloc(sizeof(struct producerConsumerStats));
    pStats->totalMessages = 0;
    pStats->timeWaiting = 0;
    pStats->timeBlocked = 0;    
    alive = 1;
    malicious = 0;
    if (argc >= 2)
    {
        if (strcmp(argv[1], "-h") == 0)
        {
            printf("\n%s\n", "Application receives up to two parameters, with the first being mandatory:\n\t Buffer Name: char*\n\t Producer Mean: +double\n");
            exit(EXIT_SUCCESS);
        }
        else
        {
            bufferName = argv[1];
        }
        if (argc == 3)
        {
            mean = atof(argv[3]);
            if (mean <= 0)
            {
                withErrors = 1;
                printf("\n%s\n", "Producer Mean must be a positive double.");
            }
        }
    }
    else
    {
        withErrors = 1;
    }
    if (withErrors)
    {
        printf("\n%s\n\n", "Parameters type does not match, please use -h to see usage.");
    }
}

//TODO: Same method as consumer
double expDist(double lambda)
{
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1 - u) / lambda;
}

bool killProducer(int sm, void *map, sem_t *produceS, struct Metadata *metadata, sem_t *metadataS, bool increaseCount)
{
   
    before = clock();

    //Validates the terminate variable for killing producer
    int terminate = 0;
    sem_wait(metadataS);
    terminate = metadata->terminate;
    sem_post(metadataS);
    difference = clock() - before;
    pStats->timeBlocked += difference * 1000 / CLOCKS_PER_SEC;
    if (!terminate)
    {
        if(increaseCount){
            metadata->pCount++;
        }        
        return false;
    }
    else
    {
        sem_wait(metadataS);
        if (metadata->pCount > 0){
            metadata->pCount--;
        }
        sem_post(metadataS);
        printf("This producer:  %i  is going to die.\n", getpid());
        getStatistics();
        sem_close(metadataS);
        sem_close(produceS);
        munmap(map, totalSize);
        close(sm);
        return true;
    }
}

void produce()
{
    int sm;
    if ((sm = shm_open(bufferName, O_RDWR, 0)) != -1)
    {
        // Open memory map to corresponding structs and set up semaphores
        fstat(sm, &smInfo);
        void *map = mmap(0, smInfo.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, sm, 0);
        struct Metadata *metadata = ((struct Metadata *)map);
        bufferSize = metadata->bufferLength * sizeof(struct Message);
        totalSize = metadataSize + semaphoresSize + bufferSize;
        struct Semaphores *semaphores = ((struct Semaphores *)map) + metadataSize;
        strncpy(lConsume, semaphores->consume, 10);
        strncpy(lProduce, semaphores->produce, 10);
        strncpy(lMetadata, semaphores->metadata, 10);
        void *buffer = ((void *)map) + metadataSize + semaphoresSize;
        sem_t *metadataS = sem_open(lMetadata, O_RDWR);
        sem_t *produceS = sem_open(lProduce, O_RDWR);

        //Initialize stats for the current producer
        if (!killProducer(sm, map, produceS, metadata, metadataS, true))
        {
            int wait = 0;
            while (alive)
            {
                before = clock();
                sleep(expDist(mean));
                difference = clock() - before;
                pStats->timeWaiting +=  ((double)difference)/CLOCKS_PER_SEC;
                printf("time: %lf ", pStats->timeWaiting);

                sem_wait(metadataS);
                wait = metadata->queued;
                sem_post(metadataS);
                sem_wait(produceS);
                while (wait >= metadata->bufferLength)
                {
                    raise(SIGSTOP);
                    sem_wait(metadataS);
                    wait = metadata->queued;
                    sem_post(metadataS);
                }

                printf("Producing\n");
                struct Message *message = ((struct Message *)buffer) + ((metadata->pIndex % metadata->bufferLength) * sizeof(struct Message));
                message->pid = (rand() % (9999 + 1 - 0)) + 0;
                message->datetime = time(NULL);
                message->key = -1;
                message->terminate = 1;
                metadata->pIndex++;
                metadata->queued++;

                sem_wait(metadataS);
                printf("\n Producer with id %i produce a message from process with id %i\n Quantity of producers alive: %i\n Quantity of consumers alive: %i \n\n",
                 getpid(), message->pid, metadata->pCount, metadata->cCount);
                metadata->queued++;
                sem_post(metadataS);

                pStats->totalMessages++;
                sem_post(produceS);
                kill(-1, SIGCONT);
                alive = !killProducer(sm, map, produceS, metadata, metadataS, false);
            }            
        }
    }
    else
    {
        printf("\nCould not open shared memory with name %s.\n\n", bufferName);
        exit(EXIT_FAILURE);
    }
}

void getStatistics(){
    printf("\n Statistics from producer with pid: %i\n  Number of messages produced: %i\n Total time blocked(ms): %lf\n Total time waiting(ms): %lf", 
    getpid(), pStats->totalMessages, pStats->timeBlocked, pStats->timeWaiting);
}