#include <pthread.h>
#include <sys/types.h>
#include "myUtils.h"

// TODO everything

int openConnection(const char* sockname, int msec, const struct timespec abstime) {
    return -1;
}

int closeConnection(const char* sockname) {
    return -1;
}

int openFile(const char* pathname, int flags) {
    return -1;
}

int readFile(const char* pathname, void** buf, size_t* size) {
    return -1;
}

int readNFiles(int N, const char* dirname) {
    return -1;
}

int writeFile(const char* pathname, const char* dirname) {
    return -1;
}
int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname) {
    return -1;
}

int lockFile(const char* pathname) {
    return -1;
}

int unlockFile(const char* pathname) {
    return -1;
}

int closeFile(const char* pathname) {
    return -1;
}

int removeFile(const char* pathname) {
    return -1;
}