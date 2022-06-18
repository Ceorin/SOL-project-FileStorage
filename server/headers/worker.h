#ifndef WORKER_HEADER
#define WORKER_HEADER
    static pthread_mutex_t testMutex = PTHREAD_MUTEX_INITIALIZER;
    
    typedef struct {
        int threadId;
    } threadData;
    
    #define CO_BUFSIZE 100
    
    
    
    void* testThread(void*);
    void* workerThread(void*);

#endif
