#ifndef DEFINITIONS_H
#define DEFINITIONS_H

struct producer {

};

struct consumer {
    int pid;
    double avg;
    int total_messages;
    int total_time_blocked;
    int total_time_waiting;
    char* buffer_name;
    bool is_alive;
};

struct message {
    int id;
    int datetime;
    int key; // random 0-4
} message;

struct datum {
    int buffer_length;
    int totalProducers;
    int totalConsumers;
    char* buffer_sem;
    char* consumer_count_sem;
} datum;

struct datum_buffer {
    struct datum metadata;
    struct message buffer[];
} datum_buffer;



// To be used only by producers and consumers (NOT Creator)
int get_file_descriptor(char buffer_name[]);
struct datum *get_datum(long map_size, int fd);
struct datum_buffer *get_datum_buffer(long map_size, int fd);

#endif //DEFINITIONS_h
