#include <sys/mman.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "definitions.h"

char buffer_name[] = "project_1";

int init_app();
int reader_app();
int writer_app();
int terminate_app();

int main(int argc, char** argv) {
    init_app();
    reader_app();
    writer_app();
    writer_app();
    writer_app();
    writer_app();
    reader_app();
    terminate_app();
}

int init_app() {
    printf("- Initializer -\n");

    int fd;
    struct datum counters, *counters_pointer;
    long page_size = sizeof(counters);

    printf("-- System Page size: %ld\n", sysconf(_SC_PAGE_SIZE));
    printf("-- App Page Size: %ld\n", page_size);

    /*******************************/
    printf("-- shm_open()\n");

    fd = shm_open(buffer_name, O_CREAT|O_RDWR|O_TRUNC, S_IRWXO|S_IRWXG|S_IRWXU);

    if ( fd == -1 ) {
        perror("--- shm_open failure\n");
        return 1;
    }

    if((ftruncate(fd, page_size)) == -1){
        perror("--- ftruncate failure\n");
        return 1;
    }

    /*******************************/
    printf("-- mmap\n");

    counters_pointer = (struct datum *)mmap(0, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (counters_pointer == MAP_FAILED) {
        perror("--- mmap failed");
        return 1;
    }

    printf("-- Current data\n");
    printf("--- Total Producers: %i\n", counters_pointer->totalProducers);
    printf("--- Total Consumers: %i\n", counters_pointer->totalConsumers);

    printf("-- Initializing counters\n");
    // doesn't update the nmap
    // counters = (struct datum){ .totalProducers=5, .totalConsumers=4};
    // counters_pointer = &counters;
    counters_pointer->totalProducers = 5;
    counters_pointer->totalConsumers = 10;

    printf("-- Stored data\n");
    printf("--- Total Producers: %i\n", counters_pointer->totalProducers);
    printf("--- Total Consumers: %i\n", counters_pointer->totalConsumers);

    printf("-- Terminate init \n");
    munmap(counters_pointer, page_size);
    close(fd);

    printf("-- Initializer - END \n\n");
    return 0;
}

int reader_app(){
    printf("- Reader -\n");
    int fd;
    struct datum counters, *counters_pointer;
    long page_size = sizeof(counters);

    /*******************************/
    printf("-- shm_open()\n");

    fd = shm_open(buffer_name, O_RDWR, S_IRWXO|S_IRWXG|S_IRWXU);

    if ( fd == -1 ) {
        perror("--- shm_open failure");
        return 1;
    }

    /*******************************/
    printf("-- mmap - Read stored data \n");

    counters_pointer = (struct datum *)mmap(0, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (counters_pointer == MAP_FAILED) {
        perror("--- mmap failed");
        return 1;
    }

    printf("-- Stored data\n");
    printf("--- Total Producers: %i\n", counters_pointer->totalProducers);
    printf("--- Total Consumers: %i\n", counters_pointer->totalConsumers);

    printf("-- Terminate reader \n");
    munmap(counters_pointer, page_size);
    close(fd);

    printf("-- Reader - END \n\n");
    return 0;
}


int writer_app(){
    printf("- Writer -\n");
    int fd;
    struct datum counters, *counters_pointer;
    long page_size = sizeof(counters);

    /*******************************/
    printf("-- shm_open()\n");

    fd = shm_open(buffer_name, O_RDWR, S_IRWXO|S_IRWXG|S_IRWXU);

    if ( fd == -1 ) {
        perror("--- shm_open failure");
        return 1;
    }

    /*******************************/
    printf("-- mmap - Read stored data \n");

    counters_pointer = (struct datum *)mmap(0, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (counters_pointer == MAP_FAILED) {
        perror("--- mmap failed");
        return 1;
    }

    printf("-- Increment counters\n");
    counters_pointer->totalProducers = counters_pointer->totalProducers + 1;
    counters_pointer->totalConsumers = counters_pointer->totalConsumers + 1;

    printf("-- Terminate Writer\n");
    munmap(counters_pointer, page_size);
    close(fd);

    printf("-- Writer - END \n\n");
    return 0;
}

int terminate_app() {
    shm_unlink(buffer_name);
    return 0;
}