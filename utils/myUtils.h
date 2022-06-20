#ifndef MY_UTILITIES_HEADER
#define MY_UTILITIES_HEADER
    
    #define test_error(comp, sc, msg) \
        if ((sc) == (comp) ) { perror (msg); exit(EXIT_FAILURE); }
    #define test_error_isNot(comp, sc, msg) \
        if ((sc) != (comp) ) { perror (msg); exit(EXIT_FAILURE); }
    
    #ifndef UNIX_PATH_MAX
        #define UNIX_PATH_MAX 108
    #endif
    
#endif