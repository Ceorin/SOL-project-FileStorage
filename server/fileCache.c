#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "fileCache.h"
#include "server.h"

int initCache (Config con) { // TODO better of course
    if (_fileCache.fileArray != NULL || _fileCache.memory_in_use > 0 || _fileCache.file_num > 0) {
        // ALREADY INITIALIZED - ERROR!
        return -1;
    }
    printf("\ninitializing cache"); //debug
    _fileCache.memory_in_use = 0;
    _fileCache.file_num = 0;
    test_error(NULL, _fileCache.fileArray = (myFile*) malloc (con.file_num * sizeof(myFile)), "Creating cache"); // NOT 10 - SEE CONFIG
    test_error_isNot(0, pthread_mutex_init(&_fileCache.mutex, NULL), "Creating cache");
    // EVENTUAL OTHER INITs
    printf("\ncache initialized\n\n"); //debug
    return 0;
}

void mockupAccessCache (int name) {
    pthread_mutex_lock(&_fileCache.mutex); 
    _fileCache.memory_in_use++;
    fprintf(stdout, "Thread %d accessed cache\tMemory currently: %d\n", name, _fileCache.memory_in_use);
    pthread_mutex_unlock(&_fileCache.mutex);
}

void mockupCheckMemory () {
    pthread_mutex_lock(&_fileCache.mutex); 
    fprintf(stdout, "\nFileCache memory in use: %d\n", _fileCache.memory_in_use);
    pthread_mutex_unlock(&_fileCache.mutex);
    fflush (stdout);
}