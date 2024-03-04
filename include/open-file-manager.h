
#include <data-container.h>


#ifndef OPEN_FILE_MANAGER_H
#define  OPEN_FILE_MANAGER_H


FileMemoryStruct_t *fsh_openfilemanager_openFile(const char * path, const char * newBlobID);
FileMemoryStruct_t *fsh_openfilemanager_getFileContext(const char * path);
int fsh_openfilemanager_closeFile(const char * path);
void fsh_openfilemanager_closeAllFiles(void);

#endif