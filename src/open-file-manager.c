
#include <string.h>

#include <global.h>
#include <messages.h>
#include <data-container.h>
#include <alloc.h>

/*
typedef struct  {
  char *memory;
  size_t size;
} MemoryStruct_t;



typedef struct  {
    char blobID[BLOB_ID_LEN + 1];
    MemoryStruct_t *memory;
} FileMemoryStruct_t;


*/

typedef struct OpenFileList {                   // simple linked list to maintain open files
    char path[MAX_PATH_LEN];
    struct OpenFileList *next;
    FileMemoryStruct_t *file;
} OpenFileList_t;

static OpenFileList_t *firstOpenFile = NULL;
size_t  openFiles = 0;

FileMemoryStruct_t *fsh_openfilemanager_getFileContext(const char * path);

FileMemoryStruct_t *fsh_openfilemanager_openFile(const char * path, const char * newBlobID) {
    
    FileMemoryStruct_t *fileCtx = NULL;
    if(fileCtx = fsh_openfilemanager_getFileContext(path)) {
        LOG_INFO("Denying double openeing of file >%s<, sending back existing context.",path);
        return fileCtx;
    }

    ++openFiles;
    LOG_DEBUG("Register open file >%s<. Open are >%zu< now.", path, openFiles);
    FileMemoryStruct_t *result = NULL;
    OpenFileList_t *openedFile = MALLOC(sizeof(OpenFileList_t));
    MEMCHK(openedFile) goto ERROR;

    strncpy(openedFile->path, path, MAX_PATH_LEN);

    FileMemoryStruct_t *file=MALLOC(sizeof(FileMemoryStruct_t));
    MEMCHK(file) goto ERROR;

    memset(file->blobID,0,BLOB_ID_LEN + 1);
    strncpy(file->blobID,newBlobID,BLOB_ID_LEN + 1);

    openedFile->file = file;
    openedFile->next = firstOpenFile;
    firstOpenFile = openedFile;

    result = openedFile->file;
    goto EXIT;
ERROR:
    LOG_ERR("Leaving in an error condition");
    if(openedFile) FREE(openedFile->file);
    FREE(openedFile);
EXIT:
    return result;
}

FileMemoryStruct_t *fsh_openfilemanager_getFileContext(const char * path) {
    OpenFileList_t *fileInList = firstOpenFile;

    while(fileInList) {
        if(strncmp(path, fileInList->path, MAX_PATH_LEN) == 0) {
            LOG_DEBUG("Found file OPEN >%s<",path);
            return fileInList->file;
        }
        fileInList = fileInList->next;
    }

    LOG_WARN("Tried to check for an open file which is either closed or wasn't opened >%s<",path);
    return NULL;
}


int fsh_openfilemanager_closeFile(const char * path) {
    OpenFileList_t *fileInList = firstOpenFile;
    OpenFileList_t *previousFileInList = NULL;

    while(fileInList) {
        OpenFileList_t *tmpFileInListNext = firstOpenFile->next;

        if(strncmp(path, fileInList->path, MAX_PATH_LEN) == 0) {
            --openFiles;
            LOG_DEBUG("Found file and CLOSING it locally >%s<, %zu still open",path, openFiles);

            if(!previousFileInList) {
                firstOpenFile = tmpFileInListNext;
            } else {
                previousFileInList->next = tmpFileInListNext;
            }

            if(fileInList->file && fileInList->file->memory) FREE(fileInList->file->memory->memory);
            if(fileInList->file) FREE(fileInList->file->memory);
            FREE(fileInList->file);
            FREE(fileInList);

            return 0;

        } else previousFileInList = fileInList;
        fileInList = tmpFileInListNext;
    }

    LOG_WARN("Tried to close a file which is either closed or wasn't opened >%s<",path);
    return -1;
}



void fsh_openfilemanager_closeAllFiles(void) {
    OpenFileList_t *fileInList = firstOpenFile;

    while(fileInList) {
        OpenFileList_t *tmpFileInListNext = firstOpenFile->next;
        --openFiles;
        
        if(fileInList->file && fileInList->file->memory) FREE(fileInList->file->memory->memory);
        if(fileInList->file) FREE(fileInList->file->memory);
        FREE(fileInList->file);
        FREE(fileInList);

        fileInList = tmpFileInListNext;
    }

    if(openFiles != 0) {
        LOG_WARN("There seems an issue with still open files (still open %zu)",openFiles);
    } else { 
        LOG_INFO("Closed all files"); 
    }

}

