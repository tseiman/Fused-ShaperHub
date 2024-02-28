/** ***************************************************************************
 *  ***************************************************************************
 *
 * alloc.c is part of the project: FILLME 
 * Project Page: https://github.com/tseiman/
 * Author: Thomas Schmidt
 * Copyright (c) 2024 
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

int alloc_counter;


void *gcp_malloc(size_t size, char* caller, unsigned int line ) {
    void *pointer = NULL;
    ++alloc_counter; 
    if(! (pointer = malloc(size))) { 
        LOG_MEM_ERR("was not able to allocate %ld bytes, failed. \t(%s:%d)" , (long int) size, caller, line);
    } else MEM_DBG("Allocated %ld bytes at %p, %d different memory segments for different allocations allocated. \t(%s:%d)", (long int) size, pointer, alloc_counter, caller, line);
    return pointer;
}

void *gcp_realloc(void *p, size_t size, char* caller, unsigned int line ) {
    void  *pointer = NULL; 
    if(!p) ++alloc_counter; 
    if(! (pointer = realloc(p, size))) { 
        LOG_MEM_ERR("was not able to re-allocate %ld bytes, failed. \t(%s:%d)", (long int) size, caller, line); 
    } else MEM_DBG("(Re-)Allocated %ld bytes at %p, %d different memory segments for different allocations allocated. \t(%s:%d)", (long int) size, pointer, alloc_counter, caller, line); 
    return pointer;
}

void gcp_free(void *p, char* caller, unsigned int line ) { 
    if(p) { 
        --alloc_counter; 
        free(p); 
        MEM_DBG("Freeing at %p, still to free %d.  \t(%s:%d)",p, alloc_counter, caller, line); 
        p = NULL; 
    }
}

