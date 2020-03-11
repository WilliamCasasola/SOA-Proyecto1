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
    // reader_app();
    // writer_app();
    // reader_app();
    // writer_app();
    // writer_app();
    // reader_app();
    //terminate_app();
}

int init_app() {
    printf("------ Initializer ------\n");

    int buffer_length = 8;                                  // TODO: receive as argument
    int fd;                                                 // File Descriptor
    struct datum *datum_pointer;                            // Pointer to the section of memory that holds basic metadata
    struct datum_buffer *datum_buffer_pointer;      // Pointer to the section that holds all data
    long datum_size = sizeof(datum);
    long buffer_size = sizeof(message) * buffer_length;

    printf("---- System Page size: %ld\n", sysconf(_SC_PAGE_SIZE));
    printf("---- Metadata Size: %ld\n", datum_size);
    printf("---- Buffer Size: %ld\n", buffer_size);

    /*********************************************************************************************/
    // Create shared memory object
    /*********************************************************************************************/
    printf("-- shm_open()\n");

    // TODO: remove O_TRUNC, add O_EXCL
    fd = shm_open(buffer_name, O_CREAT|O_RDWR|O_TRUNC, S_IRWXO|S_IRWXG|S_IRWXU);

    if ( fd == -1 ) {
        perror("---- shm_open failure\n");
        return 1;
    }

    if((ftruncate(fd, datum_size + buffer_size)) == -1){
        perror("---- ftruncate failure\n");
        return 1;
    }

    /*********************************************************************************************/
    // Map memory object used to hold metadata
    /*********************************************************************************************/
    printf("-- mmap - Metadata\n");

    datum_pointer = get_datum(datum_size, fd);

    printf("---- Current data\n");
    printf("------- Buffer length: %i\n", datum_pointer->buffer_length);
    printf("------- Total Producers: %i\n", datum_pointer->totalProducers);
    printf("------- Total Consumers: %i\n", datum_pointer->totalConsumers);

    printf("-- Initializing counters\n");
    datum_pointer->buffer_length = buffer_length;
    datum_pointer->totalProducers = 5;
    datum_pointer->totalConsumers = 10;

    printf("---- Current data\n");
    printf("------- Buffer length: %i\n", datum_pointer->buffer_length);
    printf("------- Total Producers: %i\n", datum_pointer->totalProducers);
    printf("------- Total Consumers: %i\n", datum_pointer->totalConsumers);

    /*********************************************************************************************/
    // Map memory object used to hold metadata + buffer
    /*********************************************************************************************/
    printf("-- mmap - Metadata + Buffer\n");

    datum_buffer_pointer = get_datum_buffer(datum_size + buffer_size, fd);

    // TODO: Should we initialize to something specific?
    // struct message buffer[buffer_length];
    // for (int i = 0; i < buffer_length; i++)
    // {
    //     buffer[i] = (struct message) { i, i * 2, rand() % 4};
    // }
    for (int i = 0; i < buffer_length; i++)
    {
        datum_buffer_pointer->buffer[i] = (struct message) { .id=i, .datetime=i * 2, .key=rand() % 4};
    }

    printf("---- Messages in buffer\n");

    for (int i = 0; i < buffer_length; i++)
    {
        printf("------- Message - Id : %i, Datetime : %i, Key : %i\n", datum_buffer_pointer->buffer[i].id, datum_buffer_pointer->buffer[i].datetime, datum_buffer_pointer->buffer[i].key);
    }

    /*********************************************************************************************/
    // Terminate init/create process
    /*********************************************************************************************/
    printf("-- Terminate init \n");

    munmap(datum_pointer, datum_size);
    munmap(datum_buffer_pointer, datum_size + buffer_size);
    close(fd);

    printf("-- Initializer - END \n\n");
    return 0;
}

int reader_app(){
    printf("------ Reader ------\n");
    int fd;
    struct datum *datum_pointer;
    long datum_size = sizeof(datum);

    /*********************************************************************************************/
    // Access shared memory object
    /*********************************************************************************************/
    printf("-- shm_open()\n");

    fd = get_file_descriptor(buffer_name);

    /*********************************************************************************************/
    // Map memory object used to hold metadata
    /*********************************************************************************************/
    printf("-- mmap - Metadata \n");

    datum_pointer = get_datum(datum_size, fd);

    printf("-- Stored data\n");
    printf("---- Buffer Length: %i\n", datum_pointer->buffer_length);
    printf("---- Total Producers: %i\n", datum_pointer->totalProducers);
    printf("---- Total Consumers: %i\n", datum_pointer->totalConsumers);

    /*********************************************************************************************/
    // Map memory object used to hold metadata + buffer
    /*********************************************************************************************/
    printf("-- mmap - Metadata + Buffer \n");
    int buffer_length = datum_pointer->buffer_length;
    long buffer_size = sizeof(message) * buffer_length;

    struct datum_buffer *datum_buffer_pointer;
    datum_buffer_pointer = get_datum_buffer(datum_size + buffer_size, fd);

    printf("---- Read messages in buffer\n");

    for (int i = 0; i < buffer_length; i++)
    {
        printf("------ Message - Id : %i, Datetime : %i, Key : %i\n", datum_buffer_pointer->buffer[i].id, datum_buffer_pointer->buffer[i].datetime, datum_buffer_pointer->buffer[i].key);
    }

    /*********************************************************************************************/
    // Terminate read process
    /*********************************************************************************************/
    printf("-- Terminate reader \n");

    munmap(datum_pointer, datum_size);
    munmap(datum_buffer_pointer, datum_size + buffer_size);
    close(fd);

    printf("-- Reader - END \n\n");
    return 0;
}


int writer_app(){
    printf("------ Writer ------\n");
    int fd;
    struct datum *datum_pointer;
    long datum_size = sizeof(datum);

    /*********************************************************************************************/
    // Access shared memory object
    /*********************************************************************************************/
    printf("-- shm_open()\n");

    fd = get_file_descriptor(buffer_name);

    /*********************************************************************************************/
    // Map memory object used to hold metadata
    /*********************************************************************************************/
    printf("-- mmap - Metadata \n");

    datum_pointer = get_datum(datum_size, fd);

    printf("---- Increment counters\n");
    datum_pointer->totalProducers += 1;
    datum_pointer->totalConsumers += 1;

    /*********************************************************************************************/
    // Map memory object used to hold metadata + buffer
    /*********************************************************************************************/
    printf("-- mmap - Metadata + Buffer \n");
    int buffer_length = datum_pointer->buffer_length;
    long buffer_size = sizeof(message) * buffer_length;

    struct datum_buffer *datum_buffer_pointer;
    datum_buffer_pointer = get_datum_buffer(datum_size + buffer_size, fd);

    printf("---- Update messages in buffer\n");

    for (int i = 0; i < buffer_length; i++)
    {
        datum_buffer_pointer->buffer[i].datetime = datum_buffer_pointer->buffer[i].datetime + 1;
    }

    /*********************************************************************************************/
    // Terminate write process
    /*********************************************************************************************/
    printf("-- Terminate Writer\n");
    munmap(datum_pointer, datum_size);
    close(fd);

    printf("-- Writer - END \n\n");
    return 0;
}

int terminate_app() {
    printf("------ Finalizer ------\n");

    /*********************************************************************************************/
    // Realease shared memory object
    /*********************************************************************************************/
    printf("-- Terminate application \n");

    shm_unlink(buffer_name);

    return 0;
}