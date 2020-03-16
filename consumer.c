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
time_t start, end;

struct stat smInfo;
struct producerConsumerStats *cStats;

void parseAndValidateParams();
void consume();
void finalize();
double expDist(double lambda);
void getStatistics();

int main(int argc, char **argv)
{
    parseAndValidateParams(argc, argv);
    metadataSize = sizeof(struct Metadata);
    semaphoresSize = sizeof(struct Semaphores);
    srand(time(NULL));
    consume();
    exit(EXIT_SUCCESS);
}

void parseAndValidateParams(int argc, char **argv)
{
    int withErrors = 0;
    mean = 0.25;
    alive = 1;
    malicious = 0;
    cStats = malloc(sizeof(struct producerConsumerStats));
    cStats->totalMessages = 0;
    cStats->timeWaiting = 0;
    cStats->timeBlocked = 0;
    cStats->timeSleeping = 0;

    if (argc >= 2)
    {
        if (strcmp(argv[1], "-h") == 0)
        {
            printf("\n%s\n", "Application receives up to two parameters, with the first being mandatory:\n\t Buffer Name: char*\n\t Consumer Mean: +double\n");
            exit(EXIT_SUCCESS);
        }
        else
        {
            bufferName = argv[1];
        }
        if (argc == 3)
        {
            mean = atof(argv[2]);
            if (mean <= 0)
            {
                withErrors = 1;
                printf("\n%s\n", "Consumer Mean must be a positive double.");
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

double expDist(double lambda)
{
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1 - u) / lambda;
}

void consume()
{
    int sm;
    if ((sm = shm_open(bufferName, O_RDWR, 0)) != -1)
    {
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
        sem_t *consumeS = sem_open(lConsume, O_RDWR);

        int terminate = 0;

        //First Block
        double dif;
        time(&start);
        sem_wait(metadataS);
        time(&end);
        dif = difftime(end, start);
        cStats->timeBlocked += dif;

        terminate = metadata->terminate;
        if (!terminate)
        {
            metadata->cCount++;
        }
        sem_post(metadataS);
        if (terminate)
        {
            printf("The Shared Memory has been marked to terminate, can't create new Consumer\n");
            sem_close(metadataS);
            sem_close(consumeS);
            munmap(map, totalSize);
            close(sm);
        }
        else
        {
            int wait = 0;
            while (alive)
            {

                //Sleeps
                dif = 0;
                time(&start);
                sleep(expDist(mean));
                time(&end);
                dif = difftime(end, start);
                cStats->timeSleeping += dif;

                //Second Block
                dif = 0;
                time(&start);
                sem_wait(consumeS);
                sem_wait(metadataS);
                time(&end);
                dif = difftime(end, start);
                cStats->timeBlocked += dif;

                wait = metadata->queued;
                sem_post(metadataS);
                while (wait == 0)
                {
                    printf("\nBuffer empty.\n");

                    //Waits
                    dif = 0;
                    time(&start);
                    raise(SIGSTOP);
                    time(&end);
                    dif = difftime(end, start);
                    cStats->timeWaiting += dif;

                    //Third Block
                    dif = 0;
                    time(&start);
                    sem_wait(metadataS);
                    time(&end);
                    dif = difftime(end, start);
                    cStats->timeBlocked += dif;

                    wait = metadata->queued;
                    sem_post(metadataS);
                }
                struct Message *message = (struct Message *)((buffer) + ((metadata->cIndex % metadata->bufferLength) * sizeof(struct Message)));
                printf("\n\nConsumer with id %i consumed a message.\n\n", getpid());
                if (message->terminate || message->key == (getpid() % 5))
                {
                    alive = 0;
                    if (message->key == (getpid() % 5))
                    {
                        malicious = 1;
                    }
                }
                metadata->cIndex++;
                cStats->totalMessages++;
                sem_post(consumeS);

                //Fourth Block
                dif = 0;
                time(&start);
                sem_wait(metadataS);
                time(&end);
                dif = difftime(end, start);
                cStats->timeBlocked += dif;

                metadata->queued--;
                sem_post(metadataS);
                kill(-1, SIGCONT);
            }

            //Fifth Block
            dif = 0;
            time(&start);
            sem_wait(metadataS);
            time(&end);
            dif = difftime(end, start);
            cStats->timeBlocked += dif;

            metadata->cCount--;
            sem_post(metadataS);
            kill(-1, SIGCONT);
            finalize();
            sem_close(metadataS);
            sem_close(consumeS);
            munmap(map, totalSize);
            close(sm);
        }
    }
    else
    {
        printf("\nCould not open shared memory with name %s.\n\n", bufferName);
        exit(EXIT_FAILURE);
    }
}

void finalize()
{
    if (malicious)
    {
        printf("\n\nConsumer with process is %i has received a malicious message, it will finalize. \n\tConsumed Messages: %i\n\n", getpid(), cStats->totalMessages);
        getStatistics();
    }
    else
    {

        printf("\n\nConsumer with process is %i has been ordered to finalize. \n\tConsumed Messages: %i\n\n", getpid(), cStats->totalMessages);
        getStatistics();
    }
}

void getStatistics()
{
    double total = cStats->timeBlocked + cStats->timeWaiting + cStats->timeSleeping;
    printf("\n Statistics from consumer with pid: %i\n Number of messages consumed: %i\n Total time blocked(s): %lf\n Total time waiting(s): %lf\n Total time sleeping(s): %lf\n Total time without activity(s): %lf\n",
           getpid(), cStats->totalMessages, cStats->timeBlocked, cStats->timeWaiting, cStats->timeSleeping, total);
}
