/** ***************************************************************************
 *  ***************************************************************************
 *
 * alloc.h is part of the project: FILLME 
 * Project Page: https://github.com/tseiman/
 * Author: Thomas Schmidt
 * Copyright (c) 2024 
 *
 * Description:
 *
 * Handling Memory allocation - basically it helps in a very 
 * simple way debugging of memory issues
 *
 * ****************************************************************************
 * **************************************************************************** **/

#include <messages.h>

#ifndef ALLOC_H
#define ALLOC_H

#define MALLOC(size) gcp_malloc(__FUNCTION__, size, __FILE_NAME__, __LINE__)
#define REALLOC(p,size) gcp_realloc(__FUNCTION__, p, size, __FILE_NAME__, __LINE__)
#define FREE(p) gcp_free(__FUNCTION__, p, __FILE_NAME__, __LINE__)

#define MEMCHK(p) if(!p)

void *gcp_malloc(const char *funcName, size_t size, char* caller, unsigned int line );
void *gcp_realloc(const char *funcName, void *p, size_t size, char* caller, unsigned int line );
void gcp_free(const char *funcName, void *p, char* caller, unsigned int line );
int getAlloCounter(void);

#endif