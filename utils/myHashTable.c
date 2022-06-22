#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include "myList.h"
#include "myHashTable.h"

/*  Known hash algorithm developed by Peter J. Weinberger
    It is used in UNIX for ELF files so it should be efficient for distributing 
    names of files evenly on a hashmap                                          */
unsigned int pjw_hash (char* key, unsigned int range) {
    unsigned int h = 0, high;
    while (*key) {
        h = (h << 4) + *key++;
        high = h & 0xF0000000;
        if (high)
            h ^= high >> 24;
        h &= ~high;
    }
    return h % range;
}

/*  Initialized an empty hash table of the given load Factor 
        (note - this hashtable currently has a fixed load factor!)

    Returns the initialized hash table.
        In case of error, sets ERRNO and returns NULL.              */
hashtable_t* empty_HashTable(unsigned int lf) {
    hashtable_t* ht;

    ht = (hashtable_t *) malloc (sizeof(hashtable_t));
    if (ht == NULL)
        return NULL; // should throw ENOMEM upwards
    
    if (lf == 0) {
        errno = EINVAL;
        return NULL;
    }
    ht->loadFactor = lf;
    ht->locks = (pthread_mutex_t *) calloc (ht->loadFactor, sizeof(pthread_mutex_t));
    if (ht->locks == NULL)
        return NULL; // upthrow ENOMEM

    ht->table_Ls = (list_t**) calloc (ht->loadFactor, sizeof(list_t*));
    if (ht->table_Ls == NULL)
        return NULL; // upthrow ENOMEM

    for (unsigned int i = 0; i < ht->loadFactor; i++) {
        ht->table_Ls[i] = empty_List(true);
        if (ht->table_Ls[i] == NULL)
            return NULL; // should upthrow ENOMEM for

        if (pthread_mutex_init(&(ht->locks[i]), NULL) != 0)
            return NULL; // should upthrow eventual init errors
    }
    
    return ht;
}

/*  Looks concurrently in the table for a key and its associated value.
        If found is not NULL, sets whether the element is present or not in the table.

    Returns the value associated to the key given, or NULL
    DO NOTE - In case of error, the return value will be NULL as well, it's important
        therefore to check errno and/or the value in found.                             
        
    DO NOTE - Currently it might terminate the program forcefully if an error occurs 
        while accessing concurrently                                                    */
void* look_and_get (hashtable_t* table, char* key, int* found) {
    if (table == NULL || key == NULL) {
        errno = EINVAL;
        return NULL;
    }

    unsigned int ind = pjw_hash(key, table->loadFactor);
    void* returnVal = NULL;

    // acquires lock
    if (errno = pthread_mutex_lock(&(table->locks[ind])) != 0)
        return NULL;

    // gets and removes item
    returnVal = list_get(table->table_Ls[ind], key, false, found);
    if (errno != 0) {
        pthread_mutex_unlock(&(table->locks[ind]));
        return NULL;
    }

    // releases lock
    if (errno = pthread_mutex_unlock(&(table->locks[ind])) != 0)
        return NULL;

    return returnVal;
}

/*  Adds concurrently a key and a correspondent value in the table.

    Returns 1 if the element is added successfully or 0 if the element was already in the table.
        In case of error, returns a negative value and sets errno.
    
    DO NOTE - Currently it might terminate the program forcefully if an error occurs 
        while accessing concurrently                                                    */
int hashtable_add (hashtable_t* table, char* key, void* value) {
    if (value == NULL || table == NULL || key == NULL) { // We want an hashmap, so we don't take any empty values nor keys!
        errno = EINVAL;
        return -1;
    }

    if (table->loadFactor == 0) {
        errno = EINVAL;
        return -2;
    }
    
    unsigned int ind = pjw_hash(key, table->loadFactor);
    int added;
    // acquires lock
    if (errno = pthread_mutex_lock(&table->locks[ind]) != 0)
        return -3;

    // tries to add value
    if (add_Head(key, value, table->table_Ls[ind]) < 0) {
        if (errno == EEXIST) { // key already in the table
            added = false;
            errno = 0;
        } else { // other errors
            pthread_mutex_unlock(&(table->locks[ind]));
            return -4;
        }
    } else // key not in the table
        added = true;

    if (errno = pthread_mutex_unlock(&(table->locks[ind])) != 0)
        return -5;

    return added; // which should be either 0 -> all god but element already existed : 1 -> element added as new 
}

/*  Removes a key and its associated value from the table.

    Currently works exactly as look_and_get(), except that if an item associated to the given 
        key is found, it is also removed from the table after a proper execution.           */
void* hashtable_remove (hashtable_t* table, char* key, int* found) {
    if (table == NULL || key == NULL) {
        errno = EINVAL;
        return NULL;
    }

    unsigned int ind = pjw_hash(key, table->loadFactor);
    void* returnVal = NULL;

    // acquires lock
    if (errno = pthread_mutex_lock(&(table->locks[ind])) != 0)
        return NULL;

    // gets and removes item
    returnVal = list_get(table->table_Ls[ind], key, true, found);
    if (errno != 0) {
        pthread_mutex_unlock(&(table->locks[ind]));
        return NULL;
    }

    // releases lock
    if (errno = pthread_mutex_unlock(&(table->locks[ind])) != 0)
        return NULL;

    return returnVal;

}

/*  Frees an hashtable and its internal locks and linked lists.

    DO NOTE that this function only works properly if an appropriate free function is given 
        as a second argument and if it's used when nothing is accessing the table.
    
    If something is accessing the table or a non-appropriate free function is given,
    The behaviour is undefined, and most likely errors and/or memory leaks will occur.

    Also note: The second argument can be &free if the value of the list is a basic type. */
void delete_table(hashtable_t ** table, void (*freeFunction)(void*)) {
    if (table == NULL)
        return;

    free((*table)->locks);
    for (unsigned int i = (*table)->loadFactor; i > 0; i--)
        delete_List(&((*table)->table_Ls[i-1]), freeFunction);
    
    (*table)->loadFactor = 0;
        
    
    free((*table)->table_Ls);
    free((*table));
}




/* THIS STRUCTURE HAS NOT BEEN TESTED YET 
#include <stdio.h>
int main () {
    return 0;
} */