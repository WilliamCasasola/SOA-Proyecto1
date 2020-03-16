#ifndef SHARED_H
#define SHARED_H

struct Message {
    pid_t pid;
    int datetime;
    int key;
    int terminate;
};

struct Metadata {	
	int bufferLength;
	int pCount;
	int cCount;
	int cIndex;
	int pIndex;
	int queued;
	int terminate;
};

struct Semaphores {
	char consume[10];
	char produce[10];
	char metadata[10];
};

struct producerConsumerStats
{
    int totalMessages;
    double timeBlocked;
    double timeWaiting;
	double timeSleeping;
};


#endif //SHARED_H