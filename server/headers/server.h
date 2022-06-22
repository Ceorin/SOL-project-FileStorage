#ifndef SERVER_HEADER
#define SERVER_HEADER
    #include "myUtils.h"
    #define DEFAULT_CONFIG "config/config.txt"

    #define FILE_UPPER_BOUND 1000
    #define MEMORY_UPPER_BOUND 1000000000
    #define WORKER_UPPER_BOUND 64
    #define ERROR_MESSAGE_BUFFER_LENGTH 60

    #define REQ_QSIZE 15

    typedef struct {
        unsigned int file_num;
        unsigned int cache_size;
        unsigned int thread_num;
        char server_socket_name[UNIX_PATH_MAX]; 
        char log_file_name[UNIX_PATH_MAX];
    } Config;

    extern Config _config;

    void readConfig (char*);
#endif
