
#include <string.h>
#include <pthread.h>

#include <global.h>
#include <messages.h>
#include <data-container.h>
#include <alloc.h>

typedef struct OpenFileList {
    char path[MAX_PATH_LEN +1];
    size_t pathLen;
    struct OpenFileList *next;
    FileMemoryStruct_t *file;
} OpenFileList_t;

static OpenFileList_t *firstOpenFile = NULL;
size_t openFiles = 0;

/* Own mutex ONLY for the open-file list.
 * Using a separate mutex avoids any risk of deadlock with g_model_lock. 
 * MUST be a recursive mutex: openFile() calls getFileContext_locked() 
 * internally without releasing the lock. */
static pthread_mutex_t ofm_lock = PTHREAD_MUTEX_INITIALIZER;

/* Internal version - assumes ofm_lock is already held by caller */
static FileMemoryStruct_t *getFileContext_locked(const char *path) {
    OpenFileList_t *fileInList = firstOpenFile;
    while (fileInList) {
        size_t pathLen = strnlen(path, MAX_PATH_LEN);
        size_t cmpLen = pathLen > fileInList->pathLen ? fileInList->pathLen : pathLen;
        if (strncmp(path, fileInList->path, cmpLen) == 0) {
            LOG_DEBUG("Found file OPEN >%s<", path);
            return fileInList->file;
        }
        fileInList = fileInList->next;
    }
    return NULL;
}

/* Public API - acquires lock itself */
FileMemoryStruct_t *fsh_openfilemanager_getFileContext(const char *path) {
    pthread_mutex_lock(&ofm_lock);
    FileMemoryStruct_t *result = getFileContext_locked(path);
    pthread_mutex_unlock(&ofm_lock);
    if (!result) LOG_DEBUG("File wasn't opened negative result >%s<", path);
    return result;
}

FileMemoryStruct_t *fsh_openfilemanager_openFile(const char *path, const char *newBlobID) {
    pthread_mutex_lock(&ofm_lock);

    /* Check for already-open file while holding lock */
    FileMemoryStruct_t *fileCtx = getFileContext_locked(path);
    if (fileCtx) {
        LOG_INFO("Denying double opening of file >%s<, sending back existing context.", path);
        ++fileCtx->referenceCount;
        pthread_mutex_unlock(&ofm_lock);
        return fileCtx;
    }

    ++openFiles;
    LOG_DEBUG("Register open file >%s<. Open are >%zu< now.", path, openFiles);

    FileMemoryStruct_t *result = NULL;
    OpenFileList_t *openedFile = MALLOC(sizeof(OpenFileList_t));
    MEMCHK(openedFile) goto ERROR;

    LOG_DEBUG("File in list pointer %p", openedFile);

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
    firstOpenFile = openedFile;

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
        OpenFileList_t *tmpFileInListNext = fileInList->next;
        size_t pathLen = strnlen(path, MAX_PATH_LEN);
        size_t cmpLen = pathLen > fileInList->pathLen ? fileInList->pathLen : pathLen;

        if (strncmp(path, fileInList->path, cmpLen) == 0) {
            if (fileInList->file->referenceCount > 1) {
                LOG_DEBUG("Won't close file >%s< as there are still %d references",
                          path, fileInList->file->referenceCount);
                --fileInList->file->referenceCount;
                result = 0;
                goto EXIT;
            }

            --openFiles;
            LOG_DEBUG("Found file and CLOSING it locally >%s<, %zu still open", path, openFiles);

            if (!previousFileInList) {
                firstOpenFile = tmpFileInListNext;
            } else {
                previousFileInList->next = tmpFileInListNext;
            }

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
        fileInList = tmpFileInListNext;
    }

    LOG_ERR("Tried to close a file which is either closed or wasn't opened >%s<", path);

EXIT:
    pthread_mutex_unlock(&ofm_lock);
    return result;
}


void fsh_openfilemanager_closeAllFiles(void) {
    pthread_mutex_lock(&ofm_lock);
    OpenFileList_t *fileInList = firstOpenFile;

    while (fileInList) {
        OpenFileList_t *tmpFileInListNext = fileInList->next;
        LOG_DEBUG("Attempt to clean up file >%s< still to clean %zu", fileInList->path, openFiles);
        --openFiles;

        if (fileInList->file && fileInList->file->memory) FREE(fileInList->file->memory->memory);
        if (fileInList->file) FREE(fileInList->file->memory);
        FREE(fileInList->file);
        FREE(fileInList);

        fileInList = tmpFileInListNext;
    }

    firstOpenFile = NULL;

    if (openFiles != 0) {
        LOG_WARN("There seems an issue with still open files (still open %zu)", openFiles);
    } else {
        LOG_INFO("Closed all files");
    }

    pthread_mutex_unlock(&ofm_lock);
}
