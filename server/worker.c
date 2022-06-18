#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "worker.h"
#include "server.h"
#include "fileCache.h"

int ClientList[REQ_QSIZE];
int next = -1;
static pthread_mutex_t mutex_ClientList = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t empty_ClientList = PTHREAD_COND_INITIALIZER;
static pthread_cond_t full_ClientList = PTHREAD_COND_INITIALIZER;

void putClient(int fd) {
    pthread_mutex_lock(&mutex_ClientList);
    while (next >= REQ_QSIZE) {
        fprintf (stdout, "Waiting for list not to be full\n");
        pthread_cond_wait(&full_ClientList,&mutex_ClientList);
    }
    next++;
    ClientList[next] = fd;
    pthread_cond_signal(&empty_ClientList);
    pthread_mutex_unlock(&mutex_ClientList);
}

int getClient() {
    int newFD = -1;
    pthread_mutex_lock(&mutex_ClientList);
    while (next < 0) {
        fprintf(stdout, "Waiting for list not to be empty\n");
        pthread_cond_wait(&empty_ClientList,&mutex_ClientList);
    }
    newFD = ClientList[next];
    next--;
    pthread_cond_signal(&full_ClientList);
    pthread_mutex_unlock(&mutex_ClientList);
    return newFD;
}

void* testThread(void *arg) { // TODO redo this - this is mainly a mockup thing to test thread and mutex in c
    threadData td;
    td.threadId = (*(threadData*) arg).threadId;
    free(arg);
    static int testValue = 1;
    short int i = 0;
    int ret = -1;
    /*
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

    }*/

    for ( i = 0; i < 5; i++) {
        if (td.threadId % 2 == 0) {
            int ret = i+td.threadId;
            fprintf(stdout, "Thread %d producing value %d\n", td.threadId, ret);
            putClient(ret);
            fprintf (stdout, "Thread %d produced successully\n", td.threadId);
        } else {
            fprintf(stdout, "Thread %d tries to get a value\n", td.threadId);
            ret = getClient();
            fprintf(stdout, "Thread %d consumed value %d\n", td.threadId, ret);
        }

        fflush(stdout);
    }

    pthread_exit((void*) td.threadId );
}

//bop

