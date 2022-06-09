#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "worker.h"

void* testThread(void *arg) {
    threadData td;
    td.threadId = (*(threadData*) arg).threadId;
    free(arg);
    static int testValue = 1;
    short int i = 0;
    int ret = -1;
    
    for (i = 0; i<15; i++) {
        pthread_mutex_lock(&testMutex);
            ret = testValue;
            fprintf(stdout, "Thread: %d - shared value %d\n", td.threadId, testValue++);
        pthread_mutex_unlock(&testMutex);
    }

    pthread_exit((void*) ret);
}