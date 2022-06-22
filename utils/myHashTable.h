#ifndef MY_HASHTABLE_HEADER
#define MY_HASHTABLE_HEADER

    typedef struct hashtable_t {
        unsigned int loadFactor;
        list_t * *table_Ls;
        pthread_mutex_t *locks;
    } hashtable_t;

    // Initializes an empty hash table;
    // Returns null and sets errno in case of errors.
    hashtable_t* empty_HashTable(unsigned int);

    /* Looks for the value associated to the key given
       Returns null in case of value-not-found or errors, and in the second event sets errno.
            If the 3rd argument is not null, after the function returns, it will be set
            to either true or false depending on whether the table contained a value 
            associated to the key given.                                                */
    void* look_and_get (hashtable_t*, char*, int*);

    // As look_and_get, but also removes the value from the table after a successful execution.
    void* hashtable_remove (hashtable_t*, char*, int*);

    // Adds a value with the associated key to the table
    // Return 0 in case the value existed, 1 in case of success, or a negative value in case of errors, setting errno.
    int hashtable_add (hashtable_t*, char*, void*);

    /* Frees a hashtable resources.
      If something is accessing the table or the second function is not appropriate for freeing
        the elements of the table, the behaviour is undefined                               */
    void delete_table(hashtable_t **, void (*freeFunction)(void*));
#endif