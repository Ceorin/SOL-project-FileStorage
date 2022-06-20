#ifndef MY_LIST_HEADER
#define MY_LIST_HEADER
    
    typedef struct node node_t;

    typedef struct list_t {
        node_t* head;
        node_t* last;
        unsigned int size;
    } list_t;

    // Initializes an empty list and returns it
    // Returns NULL in case of error, setting errno
    list_t* empty_List (); 

    // Add the first argument to the list in the second argument.
    // Returns -1 in case of error, setting errno
    int add_Last (void*, list_t*);
    int add_Head (void*, list_t*);   

    /* Return the appropriate values from the list, and if the second argument is 
        true, the value will be removed from the list.
      In case of error, return NULL setting errno.
      Do note, however, that NULL might be a valid return as well.              */
    void* list_first (list_t*, bool);
    void* list_last (list_t*, bool);
    void* list_getAt(list_t*, unsigned int, bool);

    /* Return the amount of elements freed from the list pointed by the first argument.
      To avoid leaks, the second argument must be a funciton equivalent to free that clears
        the structure inside the list appropriately. 
        (Free works for simple structures that don't allocate any field dynamically).      */
    unsigned int delete_List (list_t**, void (*freeFunction)(void*));

#endif