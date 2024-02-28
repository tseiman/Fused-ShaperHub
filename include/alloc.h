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

#ifndef ALLOC_H
#define ALLOC_H

#define MALLOC(size) gcp_malloc(size, __FILE_NAME__, __LINE__)
#define REALLOC(p,size) gcp_realloc(p, size, __FILE_NAME__, __LINE__)
#define FREE(p) gcp_free(p, __FILE_NAME__, __LINE__)

void *gcp_malloc(size_t size, char* caller, unsigned int line );
void *gcp_realloc(void *p, size_t size, char* caller, unsigned int line );
void gcp_free(void *p, char* caller, unsigned int line );


#endif