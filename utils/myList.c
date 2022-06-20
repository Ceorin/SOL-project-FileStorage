#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include "myList.h"

/*  THIS DATA STRUCTURE CURRENTLY WORKS ONLY AS A STACK, AS A QUEUE OR AS SOMETHING IN-BETWEEN
    A function to add or get elements in a certain index has yet to be made                    */
struct node {
    node_t* next;
    void* item;
};

// Initializes an empty list
// returns NULL if malloc failes, or a pointer to an empty list_t otherwise
list_t* empty_List () {
    list_t* newList = (list_t *) malloc (sizeof(list_t));
    if (newList == NULL) // error
        return NULL;

    newList->head = NULL;
    newList->last = NULL;
    newList->size = 0;

    return newList;
}

// Returns a node_t ptr.
// This is an internal function and shouldn't be used outside of this file.
node_t * create_Node( void* val) {
    node_t* newNode = (node_t*) malloc (sizeof(node_t));
    if (newNode == NULL) // error
        return NULL;

    newNode->next = NULL;
    newNode->item = val;

    return newNode;
}

// Adds a node to the tail of a list.
// If an error occurs, returns -1 and sets errno, otherwise returns the list new size.
int add_Last (void* val, list_t* list) {
    if (list == NULL) {
        errno = EINVAL;
        return -1;
    }

    node_t* newVal = (node_t*) create_Node(val);
    if (newVal == NULL)
        return -1;

    if (list->size == 0) {
        list->head = newVal;
        list->last = newVal;
    } else {
        list->last->next = newVal;
        list->last = newVal;
    }

    list->size++;

    return list->size;
}


// add a node to the tail of a list.
// if an error occurs, returns -1 and sets errno, otherwise returns the list new size.
int add_Head (void* val, list_t* list) {
    if (list == NULL) {
        errno = EINVAL;
        return -1;
    }

    node_t* newVal = (node_t*) create_Node(val);
    if (newVal == NULL)
        return -1;

    newVal->next = list->head;
    list->head = newVal;

    if (list->size == 0)
        list->last = newVal;

    

    list->size++;

    return list->size;
}

/* Returns the item in the first node of the list (note - it's a void*, a cast will be necessary). 
 If remove is set true, it is also removed from the list, and the size reduced by 1.
 Returns NULL both in case of error and of null value to return!
 Sets errno in case of error - EINVAL for a bad argument. 

 DO NOTE: this operation will free the node, but the argument inside given back! Remember to free 
 the use!                                                                                         */
void* list_first (list_t* list, bool remove) {
    if (list == NULL) {
        errno = EINVAL;
        return NULL;
    }
    if (list->size == 0)
        return NULL;

    if (!remove) {
        if (list->head == NULL) {
            errno = EFAULT; // broken state of the list
            return NULL;
        } else
            return list->head->item;
    }

    void* returnVal = list->head->item;

    node_t *temp = list->head;

    list->head = list->head->next;
    list->size--;

    if (list->size == 0)
        list->last = NULL;
    
    free(temp);

    return returnVal;
}

/* Returns the item in the last node of the list (note - it's a void*, a cast will be necessary). 
 If remove is set true, it is also removed from the list, and the size reduced by 1.
 Returns NULL both in case of error and of null value to return!
 Sets errno in case of error - EINVAL for a bad argument, or EFAULT if the list internal structure 
 is broken.                                                                
                       
 DO NOTE: this operation will free the node, but the argument inside given back! Remember to free 
 the use!                                                                                         */
void* list_last (list_t* list, bool remove) {
    if (list == NULL) {
        errno = EINVAL;
        return NULL;
    }
    if (list->size == 0)
        return NULL;

    if (!remove) {
        if (list->last == NULL) {
            errno = EFAULT; // broken state of the list
            return NULL;
        } else
            return list->last->item;
    }

    if (list->head == NULL) {
            errno = EFAULT; // broken state of the list
            return NULL;
    }
    void* returnVal;

     // 1 element -> must remove that and set nulls
    if (list->head->next == NULL) {
        returnVal = list->head->item;
        free(list->head);

        list->head = list->last = NULL;
        list->size = 0;
        return returnVal;
    }
    
    // 2 or more elements -> navigate until second-to-last one, which will be the new last
    node_t *temp = list->head;
    while(temp->next->next != NULL) {
        temp = temp->next;
    }

    returnVal = list->last->item;
    temp->next = NULL;

    free(list->last);

    list->last = temp;
    list->size--;

    return returnVal;
}

/* Returns the item in N-th position. If remove is set true, it is also removed from the list, 
  and the size reduced by 1.
 Returns NULL both in case of error and of null value to return!
 Sets errno in case of error - EINVAL if the list is empty, ERANGE if the index is out of bound,
  and EFAULT if the list internal structure is broken.                                                                
                       
 DO NOTE: this operation will free the node, but the argument inside given back! Remember to free 
 the use!                                                                                         */
void* list_getAt(list_t* list, unsigned int index, bool remove) {
    if (list == NULL) {
        errno = EINVAL;
        return NULL;
    }

    if (list->size <= index) {
        errno = ERANGE;
        return NULL;
    }
    
    // this will ease the case of removing and replacing head
    if (index == 0)
        return list_first(list, remove);
    if (index == list->size -1)
        return list_last(list, remove);

    if (list->head == NULL) { 
        // should have blocked at EINVAL - list->size > index >= 0 given it's unsigned int
        errno = EFAULT;
        return NULL;
    }

    
    node_t *temp = list->head;

    if (temp == NULL) {
        // should have blocked before: To get here, size must be > index 
        errno = EFAULT;
        return NULL;
    }

    unsigned int i = 1;
    while (temp->next != NULL && i<index) {
        temp = temp->next;
        i++;
    }

    if (temp == NULL) {
        // should have blocked before: To get here, size must be > index 
        errno = EFAULT;
        return NULL;
    }
    if (temp->next == list->last) {
        // should have blocked before: if index == size-1 => the function called list_last instead
        errno = EFAULT;
        return NULL;
    }

    void* returnVal = temp->next->item;
    if (!remove)
        return returnVal;

    node_t *toRemove = temp->next;
    // Removing the node to remove from the linked list
    temp->next = toRemove->next;
    toRemove->next = NULL;

    list->size--;

    free(toRemove);

    return returnVal;
}

/* Frees all the items in the list given, and then the list itself, turning it into a NULL ptr.
 Returns the amount of elements removed
 If the returned value is lower than what the previously allocated list size was, the list was 
 badly structured (and some memory leaks might have occurred).        

 DO NOTE: The second argument can be &free if the value of the list is a normal value, but in the
 case that it's a structure, to avoid memory leaks a proper function should be created which frees
 all the dynamically allocated fields of the structure and then itself.                         */
unsigned int delete_List (list_t** list, void (*freeFunction)(void*) ) {
    if (list == NULL)
        return 0;

    unsigned int count = 0;

    node_t* deleting;

    while ((*list)->head !=NULL) {
        deleting = (*list)->head;
        (*list)->head = (*list)->head->next;

        freeFunction(deleting->item);
        free(deleting);
        count++;
    }


    free(*list);
    *list = NULL; // in case the free implementation just unchecks the value

    return count;
}

/* EXAMPLE MAIN FOR TESTING (uncomment and recompile this file alone for that)
    This test currently should print on stdout numbers from 32 to 99
#include <stdio.h>
int main () {
    list_t* testInt = empty_List();

    if (errno!=0)
        perror("Created list");
    
    int err;
    int* wrapper;
    char errMsg[5];
    for (int i = 0; i<100; i++) {
        wrapper = malloc (sizeof(int));
        *wrapper = i;
        err= add_Last(wrapper, testInt);
        if (err == -1) {
            snprintf(errMsg, 5, "%d", i);
            perror(errMsg);
        }
    }
    
    fprintf(stdout, "List is:\n");
    do {
        wrapper = (int*) list_getAt(testInt, 32, true);
        if (wrapper == NULL)
            continue;
        fprintf(stdout, "%d\t", *wrapper);
        if ((*wrapper)%10==9)
            fprintf(stdout, "\n");
        fflush(stdout);
        free(wrapper);
    } while (wrapper!=NULL);

    delete_List(&testInt, &free);
    return 0;
} */