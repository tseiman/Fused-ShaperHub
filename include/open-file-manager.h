/* ***************************************************************************
 *
 * Thomas Schmidt, 2020-2026
 *
 * This file is part of the Fused ShaperHub Project
 *
 * This file implements the open file manager which keeps track of all
 * currently opened files. It maintains a linked list of open file contexts
 * including their blob IDs and memory buffers. Thread safety is ensured
 * by an internal mutex (ofm_lock) which is kept strictly local to this
 * module to avoid cross-module deadlocks.
 *
 * License: Not Defined Yet
 *
 * Project URL: https://github.com/tseiman/Fused-ShaperHub
 *
 ************************************************************************** */

#include <data-container.h>


#ifndef OPEN_FILE_MANAGER_H
#define  OPEN_FILE_MANAGER_H


FileMemoryStruct_t *fsh_openfilemanager_openFile(const char * path, const char * newBlobID);
FileMemoryStruct_t *fsh_openfilemanager_getFileContext(const char * path);
int fsh_openfilemanager_closeFile(const char * path);
void fsh_openfilemanager_closeAllFiles(void);

#endif