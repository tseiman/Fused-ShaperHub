/** ***************************************************************************
 *  ***************************************************************************
 *
 * alloc.c is part of the project: FILLME 
 * Project Page: https://github.com/tseiman/
 * Author: Thomas Schmidt
 * Copyright (c) 2024-2026 
 *
 * Description:
 *
 * Handleing Memory allocation - basically it helps in a very 
 * simple way debugging of memory issues
 *
 * ****************************************************************************
 * **************************************************************************** **/

#include <stdlib.h>

#include "messages.h"

static int alloc_counter;


void *gcp_malloc(const char *funcName, size_t size, char* caller, unsigned int line ) {
    void *pointer = NULL;
    ++alloc_counter; 
    if(! (pointer = malloc(size))) { 
        LOG_MEM_ERR("%s(): was not able to allocate %ld bytes, failed. \t(%s:%d)", funcName, (long int) size, caller, line);
    } else MEM_DBG("%s(): Allocated %ld bytes at %p, %d different memory segments for different allocations allocated. \t(%s:%d)", funcName, (long int) size, pointer, alloc_counter, caller, line);
    return pointer;
}

void *gcp_realloc(const char *funcName, void *p, size_t size, char* caller, unsigned int line ) {
    void  *pointer = NULL; 
    if(!p) ++alloc_counter; 
    if(! (pointer = realloc(p, size))) { 
        LOG_MEM_ERR("%s(): was not able to re-allocate %ld bytes, failed. \t(%s:%d)", funcName, (long int) size, caller, line); 
    } else MEM_DBG("%s(): (Re-)Allocated %ld bytes at %p, %d different memory segments for different allocations allocated. \t(%s:%d)", funcName, (long int) size, pointer, alloc_counter, caller, line); 
    return pointer;
}

void gcp_free(const char *funcName, void *p, char* caller, unsigned int line ) { 
    if(p) { 
        --alloc_counter; 
        free(p); 
        MEM_DBG("%s(): Freeing at %p, still to free %d.  \t(%s:%d)", funcName, p, alloc_counter, caller, line); 
        p = NULL; 
    }
}

int getAllocCounter() {
    return alloc_counter;
}