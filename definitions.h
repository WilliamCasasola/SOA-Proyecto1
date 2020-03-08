struct producer {

};

struct consumer {

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
} datum;

struct datum_buffer_pointer{
    struct datum metadata;
    struct message buffer[];
} foo;

