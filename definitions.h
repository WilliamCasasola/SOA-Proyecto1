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
    int totalProducers;
    int totalConsumers;
} datum;
