#ifndef WORKER_HEADER
#define WORKER_HEADER
    
    typedef struct {
        int threadId;
    } threadData;
    
    #define CO_BUFSIZE 100
    
    void putClient(int);
    
    void* testThread(void*);
    void* workerThread(void*);

#endif
