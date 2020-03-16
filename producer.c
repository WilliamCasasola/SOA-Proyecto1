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
struct producerConsumerStats *pStats;

void parseAndValidateParams();
void produce();
void finalize();
double expDist(double lambda);
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

void parseAndValidateParams(int argc, char **argv)
{
    int withErrors = 0;
    mean = 0.25;
    alive = 1;
    pStats = malloc(sizeof(struct producerConsumerStats));
    pStats->totalMessages = 0;
    pStats->timeWaiting = 0;
    pStats->timeBlocked = 0;
    pStats->timeSleeping = 0;

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
            mean = atof(argv[2]);
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

double expDist(double lambda)
{
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1 - u) / lambda;
}

void produce()
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
        sem_t *produceS = sem_open(lProduce, O_RDWR);
        if (errno)
        {
            printf("test");
        }
        int terminate = 0;

        //First Block
        double dif;
        time(&start);
        sem_wait(metadataS);
        time(&end);
        dif = difftime(end, start);
        pStats->timeBlocked += dif;

        terminate = metadata->terminate;
        if (!terminate)
        {
            metadata->pCount++;
        }
        sem_post(metadataS);

        if (terminate)
        {
            printf("The Shared Memory has been marked to terminate, can't create new Consumer\n");
            sem_close(metadataS);
            sem_close(produceS);
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
                pStats->timeSleeping += dif;

                //Second Block
                dif = 0;
                time(&start);
                sem_wait(metadataS);
                time(&end);
                dif = difftime(end, start);
                pStats->timeBlocked += dif;

                wait = (metadata->pIn > metadata->queued ) ? metadata->pIn : metadata->queued;
                metadata->pIn++;
                alive = !metadata->terminate;
                sem_post(metadataS);
                while (wait >= metadata->bufferLength)
                {
                    printf("\nBuffer full.\n");
                    //Waits

                    dif = 0;
                    time(&start);
                    raise(SIGSTOP);
                    time(&end);
                    dif = difftime(end, start);
                    pStats->timeWaiting += dif;

                    //Third Block
                    dif = 0;
                    time(&start);
                    sem_wait(metadataS);
                    time(&end);
                    dif = difftime(end, start);
                    pStats->timeBlocked += dif;

                    wait = metadata->queued;
                    alive = !metadata->terminate;
                    if (!alive)
                    {
                        wait = -1;
                    }
                    sem_post(metadataS);
                }

                if (alive)
                {
                    //Fourth Block
                    dif = 0;
                    time(&start);
                    sem_wait(produceS);
                    dif = difftime(end, start);
                    pStats->timeBlocked += dif;

                    struct Message *message = (struct Message *)((buffer) + ((metadata->pIndex % metadata->bufferLength) * sizeof(struct Message)));
                    message->pid = getpid();
                    message->datetime = time(NULL);
                    message->key = (rand() % 5);
                    message->terminate = 0;
                    metadata->pIndex++;
                    printf("\n\nProducer with id %i generated a message.\n\n", getpid());
                    pStats->totalMessages++;
                    sem_post(produceS);
                }
                //Fifth Block
                dif = 0;
                time(&start);
                sem_wait(metadataS);
                dif = difftime(end, start);
                pStats->timeBlocked += dif;

                if (alive){
                    metadata->queued++;
                }
                metadata->pIn--;
                sem_post(metadataS);
                kill(-1, SIGCONT);
            }

            //Fifth Block
            dif = 0;
            time(&start);
            sem_wait(metadataS);
            dif = difftime(end, start);
            pStats->timeBlocked += dif;

            metadata->pCount--;
            sem_post(metadataS);
            finalize();
            sem_close(metadataS);
            sem_close(produceS);
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
    printf("\n\nProducer with process id %i has been ordered to finalize. \n\tProduced Messages: %i\n\n", getpid(), pStats->totalMessages);
    getStatistics();
}

void getStatistics()
{
    double total = pStats->timeBlocked + pStats->timeWaiting + pStats->timeSleeping;
    printf("\n Statistics from producer with pid: %i\n Number of messages produced: %i\n Total time blocked(s): %lf\n Total time waiting(s): %lf\n Total time sleeping(s): %lf\n Total time without activity(s): %lf\n",
           getpid(), pStats->totalMessages, pStats->timeBlocked, pStats->timeWaiting, pStats->timeSleeping, total);
}