#ifndef SERVER_HEADER
#define SERVER_HEADER
    #define test_error(comp, sc, msg) \
        if ((sc) == (comp) ) { perror (msg); exit(EXIT_FAILURE); }
    #define test_error_isNot(comp, sc, msg) \
        if ((sc) != (comp) ) { perror (msg); exit(EXIT_FAILURE); }

    #define DEFAULT_CONFIG "config/config.txt"
    #define FILE_UPPER_BOUND 1000
    #define MEMORY_UPPER_BOUND 1000000000
    #define WORKER_UPPER_BOUND 64
    #define ERROR_MESSAGE_BUFFER_LENGTH 60
    #ifndef UNIX_PATH_MAX
        #define UNIX_PATH_MAX 108
    #endif

    typedef struct {
        unsigned int file_num;
        unsigned int cache_size;
        unsigned int thread_num;
        char server_socket_name[UNIX_PATH_MAX]; 
        char log_file_name[UNIX_PATH_MAX];
    } Config;
    static Config _config;

    void readConfig (char*);
#endif
