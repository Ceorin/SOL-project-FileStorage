#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "worker.h"
#include "server.h"
#include "fileCache.h"

void* testThread(void *arg) { // TODO redo this - this is mainly a mockup thing to test thread and mutex in c
    threadData td;
    td.threadId = (*(threadData*) arg).threadId;
    free(arg);
    static int testValue = 1;
    short int i = 0;
    int ret = -1;
    
    for (i = 0; i<15; i++) {
        pthread_mutex_lock(&testMutex);
            ret = testValue++;
        pthread_mutex_unlock(&testMutex);
        
        if (i%5 == 0) {
            fprintf(stdout, "Thread: %d - shared value was %d\n", td.threadId, ret);
            fprintf(stdout, "Thread %d tries accessing fileCache\n", td.threadId);
            mockupAccessCache(td.threadId);
            fprintf(stdout, "\n");
            fflush(stdout);
        }

    }

    pthread_exit((void*) ret);
}

//bop

