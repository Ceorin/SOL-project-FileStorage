#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>

#define CONFIG_PATH "../config/config.txt"
#define true 1
#define false 0
#define test_null (sc, msg) \
    if ((sc) == NULL ) { perror (msg); exit(EXIT_FAILURE); }
#define test_error (comp, sc, msg) \
    if ((sc) == (comp) ) { perror (msg); exit(EXIT_FAILURE); }
#define test_error_isNot (comp, sc, msg) \
    if ((sc) != (comp) ) { perror (msg); exit(EXIT_FAILURE); }

typedef struct config {
    unsigned int file_num;
    unsigned int cache_size;
    unsigned short thread_num;
    char *server_socket_name; 
    char *log_file_name;
} _config;

void readConfig (char* path) { 
    int i = 0;
    char buffer [64];
    char *key, *strVal;
    unsigned int intVal;
    unsigned short shortVal;
    FILE * inp;
    test_null ((inp = fopen (path, "r")), "Opening config file");
    fprintf(stdout, "Opening config file: Success!\n");
    while (i<5 || !feof (inp)) {
    	test_null (fgets(buffer, 64, inp), "Reading config file");
    	switch (i) {
    	  case 0:
    	  	test_error_isNot (2, sscanf(buffer, "%s %d", key, intVal);
    	  case 1:
    	  	test_error_isNot (2, sscanf(buffer, "%s %d", key, intVal);
    	  case 2:
    	  	test_error_isNot (2, sscanf(buffer, "%s %d", key, shortVal);
    	  case 3:
    	  	test_error_isNot (2, sscanf(buffer, "%s %d", key, strVal);
    	  case 4:
    	  	test_error_isNot (2, sscanf(buffer, "%s %d", key, strVal);
    	}
    	fprtinf(stdout, "TEST of Read: %s\n", &key);
    	i++;
    }

    test_error_isNot(0, fclose(inp), "Closing config file");
    fprintf(stdout, "Read config file: Success!\nLeaving readConfig()\n");
}

int main (int argc, char *argv[]) {
    if (argc > 2) { fprintf(stderr, "Starting server: the only possible parameter for the application is the config file\n"); exit(EXIT_FAILURE);
    }
    
    // readd Config file
    if (argc == 2) readConfig (argv[1]);
    else readConfig ("../config/config.txt");
    
    exit(EXIT_SUCCESS);
}
