#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <signal.h>

#include "worker.h"
#include "fileCache.h"
#include "server.h"
#include "myUtils.h"

Config _config;
//Setting signals TODO
static void handler_int_quit (int signum) {
    write(STDOUT_FILENO, "Received SIGINT OR QUIT\n", 25);
    _exit(EXIT_SUCCESS);
}

static void handler_hup (int signum) {
    write(STDOUT_FILENO, "Received SIGHUP\n", 17);

    _exit(EXIT_SUCCESS);
}

// SERVER MAIN
int main (int argc, char *argv[]) {
    if (argc > 2) { fprintf(stderr, "Starting server: the only possible parameter for the application is the config file\n"); exit(EXIT_FAILURE);
    }
    // Setting signals TODO
    struct sigaction sig1, sig2;
    sigset_t sigMask;

    memset(&sig1, 0, sizeof(sig1));
    memset(&sig2, 0, sizeof(sig2));

    // setting full mask to install handlers
    test_error(-1, sigfillset(&sigMask), "Filling signal mask");
    test_error_isNot(0, errno = pthread_sigmask(SIG_SETMASK, &sigMask, NULL), "Setting full mask");

    sig1.sa_handler = handler_int_quit;
    test_error(-1, sigaction(SIGINT, &sig1, NULL), "Setting new SIGINT handler");
    test_error(-1, sigaction(SIGQUIT, &sig1, NULL), "Setting new SIGQUIT handler");

    sig2.sa_handler = handler_hup;
    test_error(-1, sigaction(SIGHUP, &sig2, NULL), "Setting new SIGHUP handler");
    // handlers set, undoing mask
    test_error(-1, sigemptyset(&sigMask), "Emptying mask");
    test_error_isNot(0, errno = pthread_sigmask(SIG_SETMASK, &sigMask, NULL), "Removing full mask");

    test_error(-1, sigaddset(&sigMask, SIGINT), "Adding SIGINT to mask");
    test_error(-1, sigaddset(&sigMask, SIGQUIT), "Adding SIGINT to mask");
    test_error(-1, sigaddset(&sigMask, SIGHUP), "Adding SIGINT to mask");

      

    // readd Config file
    if (argc == 2) readConfig (argv[1]);
    else readConfig (DEFAULT_CONFIG);

    // logFile
    int logFile;
    test_error(-1, logFile = open(_config.log_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644), "Opening log file");
    test_error(-1, dup2(logFile, STDOUT_FILENO), "Redirecting stdout");
    
    // Worker threads
    pthread_t *workers;
    test_error(NULL, workers = (pthread_t *) calloc (_config.thread_num, sizeof(pthread_t)), "Allocating array of threads");
    
    // Pipe from workers to main
    int workersPipe[2];
    test_error(-1, pipe2(workersPipe, O_NONBLOCK), "Creating pipe");
   
    // debug print - read inputted current _config fprintf(stdout, "Number of files: %u\nMemory size: %.2f MB\nThread number: %u\nSocket: %s\nLog file path: %s\n\n", _config.file_num, ((double) _config.cache_size)/1000000, _config.thread_num, _config.server_socket_name, _config.log_file_name);
    
    // not really mockup but needs better implementation once the cache is developed
    initCache(_config);

    // Allocation of worker threads [including mockup test of Pipe]   
    //pthread_attr_t workersAttr;
    //test_error_isNot(0, errno = pthread_attr_init(&workersAttr), "Thread attribute init");
    //test_error_isNot(0, errno = pthread_attr_setdetachstate(&workersAttr, PTHREAD_CREATE_DETACHED), "Thread attribute init");
      
    for (int i = 0; i < _config.thread_num; i++) {
        test_error_isNot(0, errno = pthread_create(workers+i, NULL, &workerThread, (void*) &workersPipe[1]), "Creating a worker thread");
        test_error_isNot(0, errno = pthread_detach(workers[i]), "Detaching thread");
    }
    // mockup test of Pipe with the workers
    int readB;
    char buf[CO_BUFSIZE];


       
    
    // Creating server socket
    fprintf(stdout, "\nMaking sockets...\n"); 
    int server_listener, client_socket;
    struct sockaddr_un socketAddress;

    char communication_Buffer[CO_BUFSIZE]=""; // Comunication buffer

    struct pollfd* pFDs;
    test_error(NULL, pFDs = (struct pollfd *) calloc ((50+_config.thread_num*3), sizeof(struct pollfd)), "Allocating poll structure");
    short int nFDs = 2; // Server listener; Read-end of Main pipe
    short int tmpSize = 0, pollRes=0;

    // Creating listener...
    strncpy(socketAddress.sun_path, _config.server_socket_name, UNIX_PATH_MAX);
    socketAddress.sun_family = AF_UNIX;
    
    unlink(socketAddress.sun_path); // in case the socket hasn't been cleaned or the file is already present or something
    errno=0; // we ignore errors from unlink, the notable ones should be found by socket and bind
    test_error(-1, server_listener = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0), "Creating server socket");
    // server socket non-blocking => accepted connections will be non-blocking too!
    
    test_error(-1, bind(server_listener, (struct sockaddr *) &socketAddress,  sizeof(socketAddress)), "Binding server socket");
    test_error(-1, listen(server_listener, SOMAXCONN), "Server listen");



    // sort of debug
    fprintf(stdout, "Server socket ready to listen in address: %s as fd: %d!\nReady to accept!\n", socketAddress.sun_path, server_listener); // debug
    fflush(stdout);


    // setting up poll
    memset(pFDs, 0, sizeof(pFDs)); // probably not necessary but in case it might save stuff. Also helps not setting i-th revents to 0 manually

    pFDs[0].fd = server_listener;
    pFDs[0].events = POLLIN;

    pFDs[1].fd = workersPipe[0];
    pFDs[1].events = POLLIN;
    
    bool done = false;
    while (!done) {
        sleep(2);
        fprintf(stdout, "\nWaiting on poll...\n");
        fflush(stdout);
        test_error(-1, pollRes = poll (pFDs, nFDs, 8000), "Poll failed");

        if (pollRes == 0) {
            fprintf(stdout, "Timed out!\n");
            break;
        }

        tmpSize = nFDs;
        for (short int i = 0; i < tmpSize; i++) {
            if (pFDs[i].revents==0)
                continue; // nothing to do here
            if (!(pFDs[i].revents & POLLIN)) { //error rn
                fprintf(stdout, "Idk what this request from %d is %d\n", pFDs[i].fd, pFDs[i].revents);
            }
            
            if (pFDs[i].fd == server_listener)   {
                // Listening socket -> accept!
                fprintf(stdout, "Listening socket reading\n");

                do {
                    client_socket = accept(server_listener, NULL, 0);
                    if (client_socket < 0) {
                        if (errno != EWOULDBLOCK) {// not break => actual error    
                            perror("Accepting clients");
                            exit(EXIT_FAILURE);
                        } else 
                            errno = 0;
                    } else {
                        fprintf(stdout, "Accepted client on fd %d\n", client_socket);
                        
                        pFDs[nFDs].fd = client_socket;
                        pFDs[nFDs].events = POLLIN;
                        nFDs++;
                    }
                } while (client_socket >= 0);
            } else if (pFDs[i].fd == workersPipe[0]) {
                // Reading from worker pipe FD coming back
                fprintf(stdout, "Reading worker pipe\n");
                int FDread = -1;
                do { 
                    readB = read(pFDs[i].fd, &FDread, sizeof(int));
                    if (readB == -1) {
                        if (errno == EWOULDBLOCK)
                            errno = 0;
                        else {
                            perror("Reading from pipe");
                            exit(EXIT_FAILURE);
                        }
                    } else {
                        fprintf(stdout, "Worker gave back fd:%d\n", FDread);

                        if (FDread != -1) { // if it's a valid FD, start tracking it again!
                            pFDs[nFDs].fd = FDread;
                            pFDs[nFDs].events = POLLIN;
                            nFDs++;
                        }
                        fprintf(stdout, "Inserted %d in poll as %d\n", FDread, pFDs[nFDs].fd);
                    }
                } while (readB >= 0);
                
            } else {
                // Reading from a client?
                // Give that client to the workers!
                fprintf(stdout, "Clients %d wants to write > send to threadpool", pFDs[i]);
                putClient(pFDs[i].fd);    
                pFDs[i].fd = -1; // Remove this fd from the poll!
            }
        }
        
        // debug
        fprintf(stdout, "FD inside now:\n");
        for (short int i = 0; i < nFDs; i++) {
            fprintf(stdout, "%d\t", pFDs[i].fd);
        }

        // removing discarded FDs
        for (short int i = 0; i < nFDs; i++) {
            if (pFDs[i].fd == -1) {
                // fd chiuso
                for (short int j = i; j < nFDs; j++) {
                    pFDs[j].fd = pFDs[j+1].fd;
                    pFDs[j].events = pFDs[j+1].events;
                }
                i--;
                nFDs --;
            }
        }

        //debug
        fprintf(stdout, "\nFD now?:\n");
        for (short int i = 0; i < nFDs; i++) {
            fprintf(stdout, "%d\t", pFDs[i].fd);
        }

        fflush(stdout);
    }

    // closing sockets 
    test_error(-1, unlink(socketAddress.sun_path), "Closing server socket");
    test_error(-1, close(server_listener), "Closing server socket");



    // debug messages and such
    fprintf(stdout, "Main finished");
    mockupCheckMemory(); // currently sum of messages received? 

    // Cleanup? (To implement into a function)
    cleanCache();
    free(pFDs);
    free(workers);
    // TODO undetach threads on creation; implement a signal and then join them to clean resources

    exit(EXIT_SUCCESS);
}




/* -------------------------------------- ------------------- -------------------   */
/* --- READ CONFIG ---------------------- ------------------- -------------------   */
/* -------------------------------------- ------------------- -------------------   */
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
