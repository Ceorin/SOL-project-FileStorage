#ifndef FILE_CACHE_HEADER
#define FILE_CACHE_HEADER
    
    typedef struct {
        char* pathname; // unique ID
        char* contents;
        int ownerProcess; //I think?
        // flags?
        
        pthread_mutex_t fileLock; // remember to initialize
    } myFile; // maybe hide this inside src file -> access through functions & fileCache

    typedef struct {
        unsigned int memory_in_use;
        unsigned int file_num;
        // TODO add BETTER data structure
        myFile* fileArray;
        // TODO add mutex for access? or at least for the clean algorithm?
        pthread_mutex_t mutex;
    } fileCache;

    static fileCache _fileCache;

    int initCache ();
#endif