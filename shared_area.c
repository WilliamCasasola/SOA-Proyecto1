#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include "definitions.h"

int get_file_descriptor(char buffer_name[]) {
    int fd = shm_open(buffer_name, O_RDWR, S_IRWXO|S_IRWXG|S_IRWXU);

    if ( fd == -1 ) {
        perror("---- shm_open failure");
        //return 1;
    }

    return fd;
};

struct datum *get_datum(long datum_size, int fd) {
    struct datum *datum_pointer = (struct datum *)mmap(0, datum_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (datum_pointer == MAP_FAILED) {
        perror("---- mmap failed");
        // TODO
        // return 1;
        // exit(0);
    }

    return datum_pointer;
};


struct datum_buffer *get_datum_buffer(long map_size, int fd) {
    struct datum_buffer *datum_buffer_pointer = (struct datum_buffer *)mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (datum_buffer_pointer == MAP_FAILED) {
        perror("---- mmap failed");
        // TODO
        // return 1;
        // exit(0);
    }

    return datum_buffer_pointer;
};


