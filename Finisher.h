#ifndef FINISHER_H_
#define FINISHER_H_
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

int consumersCounter;
int producersCounter;
int totalMemory;
int memoryRelased;
int usedMemory;
int flagToCancel;
pthread_cond_t flagChanged;

char cancelMessage[] = "cancelAll";

void StopConsumers();
void StopProducers();
void SendEndMessageToConsumers();
void CreateSpecialKeyForConsumer();
void AtivateFlagToStopProducers();
void FreeBuffer();
void ShowSpecs();

#endif