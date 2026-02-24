
#include <string.h>
#include <pthread.h>

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
    char path[MAX_PATH_LEN +1];
    size_t pathLen;
    struct OpenFileList *next;
    FileMemoryStruct_t *file;
} OpenFileList_t;

static OpenFileList_t *firstOpenFile = NULL;
size_t  openFiles = 0;

FileMemoryStruct_t *fsh_openfilemanager_getFileContext(const char * path);

// pthread_mutex_t mutexLock = PTHREAD_MUTEX_INITIALIZER;

FileMemoryStruct_t *fsh_openfilemanager_openFile(const char * path, const char * newBlobID) {
    
    FileMemoryStruct_t *fileCtx = NULL;
    if((fileCtx = fsh_openfilemanager_getFileContext(path))) {
        LOG_INFO("Denying double openeing of file >%s<, sending back existing context.",path);
        ++fileCtx->referenceCount;
        return fileCtx;
    }

   // pthread_mutex_lock(&mutexLock);    
// XX       pthread_mutex_lock(&g_model_lock);    

    ++openFiles;
    LOG_DEBUG("Register open file >%s<. Open are >%zu< now.", path, openFiles);
    FileMemoryStruct_t *result = NULL;
    OpenFileList_t *openedFile = MALLOC(sizeof(OpenFileList_t));
    MEMCHK(openedFile) goto ERROR;

    LOG_DEBUG("File in list pointer %p", openedFile);

    memset(openedFile->path,0,MAX_PATH_LEN + 1);
    strncpy(openedFile->path, path, MAX_PATH_LEN);
    openedFile->pathLen = strnlen(path, MAX_PATH_LEN) + 1;

    FileMemoryStruct_t *file=MALLOC(sizeof(FileMemoryStruct_t));
    MEMCHK(file) goto ERROR;

    memset(file->blobID,0,BLOB_ID_LEN + 1);
    strncpy(file->blobID,newBlobID,BLOB_ID_LEN + 1);

    openedFile->file = file;
    openedFile->file->referenceCount = 1;
    openedFile->next = firstOpenFile;
    firstOpenFile = openedFile;

    result = openedFile->file;
    goto EXIT;
ERROR:
    LOG_ERR("Leaving in an error condition");
    if(openedFile) FREE(openedFile->file);
    FREE(openedFile);
EXIT:

 // XX   pthread_mutex_unlock(&g_model_lock);
    return result;
}


FileMemoryStruct_t *fsh_openfilemanager_getFileContext(const char * path) {
    OpenFileList_t *fileInList = firstOpenFile;

 // XX   pthread_mutex_lock(&g_model_lock);

    while(fileInList) {
//       if(fileInList->path[0] != '\0') {
//        if(fileInList->path) {
        if(fileInList) {
            size_t pathLen = strnlen(path, MAX_PATH_LEN);

//            if(fileInList->path && (strncmp(path, fileInList->path, pathLen > fileInList->pathLen ? fileInList->pathLen : pathLen ) == 0)) {

            if(fileInList && (strncmp(path, fileInList->path, pathLen > fileInList->pathLen ? fileInList->pathLen : pathLen ) == 0)) {
                LOG_DEBUG("Found file OPEN >%s<",path);
          // XX      pthread_mutex_unlock(&g_model_lock);
                return fileInList->file;
            }
        }
        fileInList = fileInList->next;
    }

    LOG_DEBUG("File wasn't opened negative result >%s<",path);
    // XX pthread_mutex_unlock(&g_model_lock);
    return NULL;
}


int fsh_openfilemanager_closeFile(const char * path) {
    OpenFileList_t *fileInList = firstOpenFile;
    OpenFileList_t *previousFileInList = NULL;
    int result = -1;

    LOG_DEBUG("Attempt to close file >%s<", path);

  // XX  pthread_mutex_lock(&g_model_lock);

    while(fileInList) {
        OpenFileList_t *tmpFileInListNext = fileInList->next;
//        if(fileInList->path[0] != '\0') {
//        if(fileInList->path) {
            if(fileInList) {
            size_t pathLen = strnlen(path, MAX_PATH_LEN);
            //if(strncmp(path, fileInList->path, pathLen > fileInList->pathLen ? fileInList->pathLen : pathLen) == 0) {
            if(fileInList && strncmp(path, fileInList->path, pathLen > fileInList->pathLen ? fileInList->pathLen : pathLen) == 0) {


                if(fileInList->file->referenceCount > 0) {
                    LOG_DEBUG("Won't close file >%s< as there are still %d references", path, fileInList->file->referenceCount);
                    result = 0;
                    goto EXIT;
                }
                --fileInList->file->referenceCount;
                --openFiles;

                LOG_DEBUG("Found file and CLOSING it locally >%s<, %zu still open",path, openFiles);

                if(!previousFileInList) {
                    firstOpenFile = tmpFileInListNext;
                } else {
                    previousFileInList->next = tmpFileInListNext;
                }

                if(fileInList->file) {
                    if(fileInList->file->memory) FREE(fileInList->file->memory->memory);
                    FREE(fileInList->file->memory);
                }
                FREE(fileInList->file);
                FREE(fileInList);

                result = 0;
                goto EXIT;
            } else previousFileInList = fileInList;
        }

        fileInList = tmpFileInListNext;
    }

    LOG_ERR("Tried to close a file which is either closed or wasn't opened >%s<",path);

EXIT:
 
  // XX  pthread_mutex_unlock(&g_model_lock);
    return result;
}



void fsh_openfilemanager_closeAllFiles(void) {
    OpenFileList_t *fileInList = firstOpenFile;

// XX    pthread_mutex_lock(&g_model_lock);

    while(fileInList) {
        OpenFileList_t *tmpFileInListNext = fileInList->next;
        LOG_DEBUG("Attemp to clean up file >%s< still to clean %zu",fileInList->path, openFiles);
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

   // XX pthread_mutex_unlock(&g_model_lock);
}

