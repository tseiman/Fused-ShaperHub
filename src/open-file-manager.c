
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


#include <string.h>
#include <pthread.h>

#include <global.h>
#include <messages.h>
#include <data-container.h>
#include <alloc.h>

typedef struct OpenFileList {
    char path[MAX_PATH_LEN + 1];
    size_t pathLen;
    struct OpenFileList *next;
    FileMemoryStruct_t *file;
} OpenFileList_t;

static OpenFileList_t *firstOpenFile = NULL;
size_t openFiles = 0;

/* Own mutex for the open-file list only. Completely separate from any
 * other locking so there is no risk of cross-module deadlock.
 * The internal helper getFileContext_locked() assumes the lock is already
 * held — this avoids recursive locking when openFile() calls it.           */
static pthread_mutex_t ofm_lock = PTHREAD_MUTEX_INITIALIZER;

/* Internal: caller must hold ofm_lock */
static FileMemoryStruct_t *getFileContext_locked(const char *path) {
    OpenFileList_t *fileInList = firstOpenFile;
    while (fileInList) {
        size_t pathLen = strnlen(path, MAX_PATH_LEN);
        size_t cmpLen  = pathLen > fileInList->pathLen ? fileInList->pathLen : pathLen;
        if (strncmp(path, fileInList->path, cmpLen) == 0) {
            LOG_DEBUG("Found file OPEN >%s<", path);
            return fileInList->file;
        }
        fileInList = fileInList->next;
    }
    return NULL;
}

/* Public API */
FileMemoryStruct_t *fsh_openfilemanager_getFileContext(const char *path) {
    pthread_mutex_lock(&ofm_lock);
    FileMemoryStruct_t *result = getFileContext_locked(path);
    pthread_mutex_unlock(&ofm_lock);
    if (!result) LOG_DEBUG("File not open: >%s<", path);
    return result;
}

FileMemoryStruct_t *fsh_openfilemanager_openFile(const char *path, const char *newBlobID) {
    pthread_mutex_lock(&ofm_lock);

    /* Check while holding lock to avoid TOCTOU race */
    FileMemoryStruct_t *fileCtx = getFileContext_locked(path);
    if (fileCtx) {
        LOG_INFO("Denying double opening of >%s<, returning existing context.", path);
        ++fileCtx->referenceCount;
        pthread_mutex_unlock(&ofm_lock);
        return fileCtx;
    }

    ++openFiles;
    LOG_DEBUG("Register open file >%s<. Open: >%zu<", path, openFiles);

    FileMemoryStruct_t *result  = NULL;
    OpenFileList_t     *openedFile = MALLOC(sizeof(OpenFileList_t));
    MEMCHK(openedFile) goto ERROR;

    memset(openedFile->path, 0, MAX_PATH_LEN + 1);
    strncpy(openedFile->path, path, MAX_PATH_LEN);
    openedFile->pathLen = strnlen(path, MAX_PATH_LEN) + 1;

    FileMemoryStruct_t *file = MALLOC(sizeof(FileMemoryStruct_t));
    MEMCHK(file) goto ERROR;

    memset(file->blobID, 0, BLOB_ID_LEN + 1);
    strncpy(file->blobID, newBlobID, BLOB_ID_LEN + 1);
    file->memory = NULL;

    openedFile->file = file;
    openedFile->file->referenceCount = 1;
    openedFile->next = firstOpenFile;
    firstOpenFile    = openedFile;

    result = openedFile->file;
    goto EXIT;

ERROR:
    LOG_ERR("Leaving in an error condition");
    if (openedFile) FREE(openedFile->file);
    FREE(openedFile);
    --openFiles;

EXIT:
    pthread_mutex_unlock(&ofm_lock);
    return result;
}


int fsh_openfilemanager_closeFile(const char *path) {
    OpenFileList_t *fileInList;
    OpenFileList_t *previousFileInList = NULL;
    int result = -1;

    LOG_DEBUG("Attempt to close file >%s<", path);

    pthread_mutex_lock(&ofm_lock);
    fileInList = firstOpenFile;

    while (fileInList) {
        OpenFileList_t *next    = fileInList->next;
        size_t          pathLen = strnlen(path, MAX_PATH_LEN);
        size_t          cmpLen  = pathLen > fileInList->pathLen ? fileInList->pathLen : pathLen;

        if (strncmp(path, fileInList->path, cmpLen) == 0) {
            if (fileInList->file->referenceCount > 1) {
                LOG_DEBUG("Won't close >%s<, still %d references", path, fileInList->file->referenceCount);
                --fileInList->file->referenceCount;
                result = 0;
                goto EXIT;
            }

            --openFiles;
            LOG_DEBUG("Closing >%s<, %zu still open", path, openFiles);

            if (!previousFileInList)
                firstOpenFile = next;
            else
                previousFileInList->next = next;

            if (fileInList->file) {
                if (fileInList->file->memory) FREE(fileInList->file->memory->memory);
                FREE(fileInList->file->memory);
            }
            FREE(fileInList->file);
            FREE(fileInList);

            result = 0;
            goto EXIT;
        }

        previousFileInList = fileInList;
        fileInList         = next;
    }

    LOG_ERR("Tried to close a file which wasn't opened: >%s<", path);

EXIT:
    pthread_mutex_unlock(&ofm_lock);
    return result;
}


void fsh_openfilemanager_closeAllFiles(void) {
    pthread_mutex_lock(&ofm_lock);
    OpenFileList_t *fileInList = firstOpenFile;

    while (fileInList) {
        OpenFileList_t *next = fileInList->next;
        LOG_DEBUG("Cleaning up >%s<, still %zu to clean", fileInList->path, openFiles);
        --openFiles;

        if (fileInList->file && fileInList->file->memory) FREE(fileInList->file->memory->memory);
        if (fileInList->file) FREE(fileInList->file->memory);
        FREE(fileInList->file);
        FREE(fileInList);

        fileInList = next;
    }

    firstOpenFile = NULL;

    if (openFiles != 0) {
        LOG_WARN("Still open files after closeAll: %zu", openFiles);
    } else {
        LOG_INFO("Closed all files");
    }

    pthread_mutex_unlock(&ofm_lock);
}
