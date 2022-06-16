#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include "worker.h"
#include "fileCache.h"
#include "server.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>


static Config _config;

// SERVER MAIN
int main (int argc, char *argv[]) {
    if (argc > 2) { fprintf(stderr, "Starting server: the only possible parameter for the application is the config file\n"); exit(EXIT_FAILURE);
    }
    int devNull;
    int saveOut;
    test_error(-1, saveOut = dup(STDOUT_FILENO), "Saving stdout descriptor");

    test_error(-1, devNull = open("/dev/null", O_WRONLY), "Opening dev/null"); // debug: send away stdout for a while
    test_error(-1, dup2(devNull, STDOUT_FILENO), "Redirecting stdout");

    // readd Config file
    if (argc == 2) readConfig (argv[1]);
    else readConfig (DEFAULT_CONFIG);

    // debug print - read inputted current _config
    fprintf(stdout, "Number of files: %u\nMemory size: %.2f MB\nThread number: %u\nSocket: %s\nLog file path: %s\n\n", _config.file_num, ((double) _config.cache_size)/1000000, _config.thread_num, _config.server_socket_name, _config.log_file_name);
    
    

    initCache(_config);

    // mockup allocation of worker threads
    pthread_t *workers;
    test_error(NULL, workers = (pthread_t *) malloc (_config.thread_num* sizeof(pthread_t)), "Allocating array of threads");

    pthread_mutex_t mutexVar;
    threadData * threadArgs;
    pthread_mutex_init(&mutexVar, NULL);
    
    for (int i = 0; i < _config.thread_num; i++) {
        char errmsg[40];
        snprintf(errmsg, 40, "Creating thread %d", i);
        test_error (NULL, threadArgs = (threadData*) malloc(sizeof(threadArgs)),  errmsg);
        threadArgs->threadId=i;
        test_error_isNot(0, errno = pthread_create(workers+i, NULL, &testThread, threadArgs), errmsg);
    }

    for (int i = 0; i < _config.thread_num; i++) {
        int status;
        test_error_isNot(0, pthread_join(workers[i], (void*) &status), "Joining back a thread");
        fprintf(stdout, "MAIN joined thread %d with value %d\n", i, status);
        fflush(stdout);
    }

    // debug reopening stdout;
    test_error(-1, dup2(saveOut, STDOUT_FILENO), "Re-redirecting stdout");
    
    // debug messages and such
    fprintf(stdout, "Main finished");
    mockupCheckMemory();
    
    fprintf(stdout, "\nMaking sockets...\n");    
    
    // Creating server socket
    int server_listener, client_socket;
    struct sockaddr_un socketAddress;

    strncpy(socketAddress.sun_path, _config.server_socket_name, UNIX_PATH_MAX);
    socketAddress.sun_family = AF_UNIX;
    
    unlink(socketAddress.sun_path); // in case the socket hasn't been cleaned or the file is already present or something
    errno=0; // we ignore errors from unlink, the notable ones should be found by socket and bind
    test_error(-1, server_listener = socket(AF_UNIX, SOCK_STREAM, 0), "Creating server socket");
    test_error(-1, bind(server_listener, (struct sockaddr *) &socketAddress,  sizeof(socketAddress)), "Binding server socket");
    test_error(-1, listen(server_listener, SOMAXCONN), "Server listen");


    fprintf(stdout, "Server socket ready to listen in address: %s as fd: %d!\nReady to accept!\n", socketAddress.sun_path, server_listener); // debug
    fflush(stdout);
    test_error(-1, client_socket = accept(server_listener, NULL, 0), "Accepting client");
    fprintf(stdout, "Accepted a client!\n");
    test_error(-1, unlink(socketAddress.sun_path), "Closing server socket");
    test_error(-1, close(server_listener), "Closing server socket");
    exit(EXIT_SUCCESS);
}






// READ Config file from path
void readConfig (char* path) { 
    int i = 0;

    // reading vars
    char buffer [UNIX_PATH_MAX +100]; 
    char key[16], strVal[UNIX_PATH_MAX];
    long long tempNum;
    FILE * inp;

    // error-checking vars
    unsigned int totRead;
    bool configured_Settings[5] = {false};
    bool configError = false;
    char tooMuch[1], // excessive gets strings
        strError[ERROR_MESSAGE_BUFFER_LENGTH]; // error message
    char *strtolExtra; // leftover from strtol

    //OPENING config file
    test_error(NULL, inp = fopen (path, "r"), "Opening config file");
    fprintf(stdout, "Opening config file: Success!\n"); //debug line

    // reading up to 5 lines
    while (i<5 && !feof (inp)) {
    	test_error(NULL, fgets(buffer, UNIX_PATH_MAX +100, inp), "Reading config file");

        if (!strcmp(buffer,"\n") || !strcmp(buffer,"\r\n") || buffer[0]=='\0') // read empty line, ignore
            continue;

    	totRead = sscanf(buffer, "%s %s %s", key, strVal, tooMuch);
        if (totRead != 2) { // each line must be KEY VALUE and only as such
            configError=true;
            strncpy(strError, "each line must contain no less and no more of 2 values", ERROR_MESSAGE_BUFFER_LENGTH);
            break;
        }

        // Read key -> value
        if (!strcmp(key, "FILE_NUM")) {

            if (configured_Settings[0]) { // number of file already defined
                configError=true;
                strncpy(strError, "FILE_NUM already defined", ERROR_MESSAGE_BUFFER_LENGTH);
                break;
            } else { // else check value of FILE_NUM key
                errno = 0;
                tempNum = strtol(strVal, &strtolExtra, 10);
                if (errno == ERANGE) {
                    configError=true;
                    strncpy(strError, "Out of bound number", ERROR_MESSAGE_BUFFER_LENGTH);
                    break;
                }

                if (strtolExtra[0]!='\0') { // leftover string > not really a integer, maybe not even a number
                    configError=true;
                    strncpy(strError, "Must be a number", ERROR_MESSAGE_BUFFER_LENGTH);
                    break;
                }

                if (tempNum <= 0 ) { // neg-or-zero value 
                    configError=true;
                    strncpy(strError, "Must be a positive integer", ERROR_MESSAGE_BUFFER_LENGTH);
                    break;
                }

                if (tempNum > FILE_UPPER_BOUND || tempNum > UINT_MAX) { // too many files or not an unsigned int
                    configError=true;
                    snprintf(strError, ERROR_MESSAGE_BUFFER_LENGTH, "FILE_NUM limit is %u", (unsigned int) FILE_UPPER_BOUND);
                    break;
                }
                
                // seems good
                _config.file_num = (unsigned int) tempNum;

                configured_Settings[0] = true;
            }
        }

        else if (!strcmp(key, "MEMORY")) {
            if (configured_Settings[1]) { // memory size already defined
                configError=true;
                strncpy(strError, "MEMORY already defined", ERROR_MESSAGE_BUFFER_LENGTH);
                break;
            } else { // check value of MEMORY key
                errno = 0;
                tempNum = strtol(strVal, &strtolExtra, 10);
                if (errno == ERANGE) {
                    configError=true;
                    strncpy(strError, "Out of bound number", ERROR_MESSAGE_BUFFER_LENGTH);
                    break;
                }

                if (tempNum <=0 ) { // neg-or-zero value 
                    configError=true;
                    strncpy(strError, "Must be a positive integer", ERROR_MESSAGE_BUFFER_LENGTH);
                    break;
                }
                
                if (!strcmp(strtolExtra, "K")) { // value in Kilobytes; might still be too big
                    if (tempNum < LONG_MAX/1000) // check overflow for weird behaviours
                        tempNum*= 1000;
                    else
                        tempNum = MEMORY_UPPER_BOUND+1;
                } else if (!strcmp(strtolExtra, "M")) { // else value in Megabytes; might still be too big
                    if (tempNum < LONG_MAX/1000000) // check overflow for weird behaviours
                        tempNum*= 1000000;
                    else
                        tempNum = MEMORY_UPPER_BOUND+1;
                } else if (strtolExtra[0] != '\0') { // else not a number
                    configError=true;
                    strncpy(strError, "Must be a number", ERROR_MESSAGE_BUFFER_LENGTH);
                    break;
                }

                if (tempNum > MEMORY_UPPER_BOUND || tempNum > UINT_MAX) { // too much memory required or not an unsigned int
                    configError=true;
                    snprintf(strError, ERROR_MESSAGE_BUFFER_LENGTH, "Memory size limit is %u byte", (unsigned int) MEMORY_UPPER_BOUND);
                    break;
                }
                
                // seems good
                _config.cache_size = (unsigned int) tempNum;

                configured_Settings[1] = true;
            }
        } 
        
        else if (!strcmp(key, "WORKERS_NUM")) {
            if (configured_Settings[2]) {
                configError=true;
                strncpy(strError, "WORKERS_NUM already defined", ERROR_MESSAGE_BUFFER_LENGTH);
                break;
            } else {
                errno = 0;
                tempNum = strtol(strVal, &strtolExtra, 10);
                if (errno == ERANGE) {
                    configError=true;
                    strncpy(strError, "Out of bound number", ERROR_MESSAGE_BUFFER_LENGTH);
                    break;
                }

                if (strtolExtra[0] != '\0') { // leftover string > not really a integer, maybe not even a number
                    configError=true;
                    strncpy(strError, "Must be a number", ERROR_MESSAGE_BUFFER_LENGTH);
                    break;
                }

                if (tempNum <=0 ) { // neg-or-zero value 
                    configError=true;
                    strncpy(strError, "Must be a positive integer", ERROR_MESSAGE_BUFFER_LENGTH);
                    break;
                }

                if (tempNum > WORKER_UPPER_BOUND || tempNum > UINT_MAX) { // too many worker threads or not an unsigned int
                    configError=true;
                    snprintf(strError, ERROR_MESSAGE_BUFFER_LENGTH, "WORKER_NUM limit is %u", (unsigned int) WORKER_UPPER_BOUND);
                    break;
                }
                
                // seems good
                _config.thread_num = (unsigned int) tempNum;

                configured_Settings[2] = true;
            }
        } 
        
        else if (!strcmp(key, "SOCKET")) {
            if (configured_Settings[3]) { // SOCKET name already defined
                configError=true;
                strncpy(strError, "SOCKET already defined", ERROR_MESSAGE_BUFFER_LENGTH);
                break;
            } else {
                if (strVal[0] == '\0') { // read empty string. Shouldn't happen - this is weird
                    configError=true;
                    strncpy(strError, "SOCKET cannot be an empty string", ERROR_MESSAGE_BUFFER_LENGTH);
                    break;
                }
                if (strlen(strVal) > (UNIX_PATH_MAX-5)) { // checks if the path length is alright for the socket standard
                    configError=true;
                    snprintf(strError, ERROR_MESSAGE_BUFFER_LENGTH, "The socket path must contain %d characters or less", (int) UNIX_PATH_MAX-5 );
                    break;
                }
                // is it needed to check if the string is valid? Probably not, probably it will be the SC call to return an error then

                // everything seems good, we can use the strVal read as socket name for the server
                snprintf(_config.server_socket_name, UNIX_PATH_MAX, "tmp/%s", strVal);

                configured_Settings[3] = true;
            }
        } 
        
        else if (!strcmp(key, "LOG")) {
            if (configured_Settings[4]) {
                configError=true;
                strncpy(strError, "LOG already defined", ERROR_MESSAGE_BUFFER_LENGTH);
                break;
            } else {
                if (strVal[0] == '\0') { // read empty string. Shouldn't happen - this is weird
                    configError=true;
                    strncpy(strError, "LOG name cannot be an empty string", ERROR_MESSAGE_BUFFER_LENGTH);
                    break;
                }
                if (strlen(strVal) > (UNIX_PATH_MAX-5)) { // checks if the path length is alright for the socket standard
                    configError=true;
                    snprintf(strError, ERROR_MESSAGE_BUFFER_LENGTH, "The log file name must contain %d characters or less", (int) UNIX_PATH_MAX-5 );
                    break;
                }
                // is it needed to check if the string is valid? Probably not, probably it will be the SC call to return an error then

                // everything seems good, we can use the strVal read as socket name for the server
                snprintf(_config.log_file_name, UNIX_PATH_MAX, "log/%s", strVal);

                configured_Settings[4] = true;
            }
        }
        
        else { // first line argument doesn't make sense
            configError=true;
            strncpy(strError, "KEY not recognized", ERROR_MESSAGE_BUFFER_LENGTH);
            break;
        }
        
    	i++;
    }

    /*if (!feof (inp)) { // WOULD THIS BE NECESSARY?
        configError=true;
        strncpy(strError, "too long but well read", ERROR_MESSAGE_BUFFER_LENGTH);
    }*/


    if (configError) { fprintf(stderr, "Improper config format: %s\tat Entry:%d\n", strError, i+1); exit(EXIT_FAILURE); }

    test_error_isNot(0, fclose(inp), "Closing config file");
    fprintf(stdout, "Read config file: Success!\nLeaving readConfig()\n"); //debug line
}
