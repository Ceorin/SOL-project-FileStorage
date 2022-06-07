#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>

#define CONFIG_PATH "../config/config.txt"
#define true 1
#define false 0
#define test_perror_NULL(sc, msg) \
    if ((sc) == NULL ) { perror (msg); exit(EXIT_FAILURE); }
#define test_perror_NEG1(sc, msg) \
    if ((sc) == -1 ) { perror (msg); exit(EXIT_FAILURE); }

struct config {
    unsigned int file_num;
    unsigned int cache_size;
    unsigned short thread_num;
    char *server_socket_name; 
    char *log_file_name;
} config;

int readConfig (char* path) { // returns 0 if okay, other codes for errors
    int i = 5;
    FILE * inp;
    test_perror_NULL(inp = fopen (path, "r"), "Opening config file: ");

}

int main (int argc, char *argv[]) {
    if (argc > 2) { 
        fprintf(stderr, "Starting server - the only possible parameter for the application is the config file");
        exit(EXIT_FAILURE);
    }
    int e;
    if (argc == 2) 
        e = readConfig (argv[1]);
    else
        e = readConfig ("../config/config.txt");
    if (e!=0) return 2;
    return 0;
}
