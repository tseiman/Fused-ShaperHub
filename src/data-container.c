/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This file is part of the MountOctave Demonstrator Project
 *
 * The file contains the implementation for local data layer and storage
 * for simplicity (and because it was already in use) the data managment
 * is implemented with JSON data.
 *
 * License: Not Defined Yet
 *
 * Project URL: https://github.com/tseiman/Fused-ShaperHub
 *
 ************************************************************************** */

#include <jansson.h>
#include <string.h>
#include <libgen.h>

// the defines are needed to get strptime()
#define __USE_XOPEN
#define _GNU_SOURCE
#include <time.h>

#include <global.h>
#include <alloc.h>
#include <messages.h>
#include <data-container.h>
#include <http-connector.h>
#include <open-file-manager.h>
#include <pthread.h>

/* -----------------------------------------------------------------------
 * RACE CONDITION FIX - COMPLETE SOLUTION
 * -----------------------------------------------------------------------
 *
 * PROBLEM 1: Single global loadedJsonPathInfo shared by all FUSE threads.
 *   Thread A sets cache to "/AnotherFolder/", Thread B overwrites with "/".
 *   Thread A's json_t* now points at wrong JSON tree -> AnotherFolder found
 *   as child of /AnotherFolder/ -> /AnotherFolder/AnotherFolder/... loop.
 *
 * PROBLEM 2: Even with TLS, getActualSubelement() returned a raw json_t*
 *   pointing INTO the TLS cache. Any subsequent updatePathInfo() call in
 *   the same thread replaces the cache, json_decref's the old tree, and
 *   the caller's pointer becomes a use-after-free -> same corruption.
 *
 * FIX 1: pthread_key_t TLS - each thread has its own PathInfo_t cache.
 *
 * FIX 2: getActualSubelement() calls json_incref() before returning.
 *   The CALLER owns the reference and MUST call json_decref() when done.
 *   All callers copy the scalar values they need into local C variables
 *   immediately, then call json_decref(). After that the cache can be
 *   freely replaced without affecting the caller.
 * ----------------------------------------------------------------------- */

static pthread_key_t  tls_pathinfo_key;
static pthread_once_t tls_key_once = PTHREAD_ONCE_INIT;

static void destroy_thread_pathinfo(void *ptr) {
    PathInfo_t *info = (PathInfo_t *)ptr;
    if (!info) return;
    if (info->jsonObjectRoot) {
        json_decref(info->jsonObjectRoot);
        info->jsonObjectRoot = NULL;
    }
    FREE(info->path);
    FREE(info);
}

static void create_tls_key(void) {
    pthread_key_create(&tls_pathinfo_key, destroy_thread_pathinfo);
}

static PathInfo_t *get_thread_pathinfo(void) {
    pthread_once(&tls_key_once, create_tls_key);
    PathInfo_t *info = (PathInfo_t *)pthread_getspecific(tls_pathinfo_key);
    if (!info) {
        info = (PathInfo_t *)MALLOC(sizeof(PathInfo_t));
        if (!info) return NULL;
        info->jsonObjectRoot = NULL;
        info->path = NULL;
        pthread_setspecific(tls_pathinfo_key, info);
    }
    return info;
}

/* *********************************************************************** */
int comparePathLevel(char *a, char *b) {
    int i, j = 0;
    for (i = 0; a[i]; a[i] == '/' ? i++ : *a++);
    for (j = 0; b[j]; b[j] == '/' ? j++ : *b++);
    return i - j;
}

/* *********************************************************************** */
PathInfo_t *updatePathInfo(const char *path) {

    MemoryStruct_t *httpResponseBuffer = NULL;
    json_error_t error;
    PathInfo_t *result = NULL;

    LOG_DEBUG("Entry with path >%s< ", path);

    PathInfo_t *loadedJsonPathInfo = get_thread_pathinfo();
    if (!loadedJsonPathInfo) {
        LOG_ERR("Failed to get thread-local PathInfo");
        return NULL;
    }

    if ((!path) && loadedJsonPathInfo->jsonObjectRoot && loadedJsonPathInfo->path)
        return loadedJsonPathInfo;

    if (loadedJsonPathInfo->jsonObjectRoot && loadedJsonPathInfo->path) {
        if (strncmp(path, loadedJsonPathInfo->path, MAX_PATH_LEN) == 0) {
            LOG_DEBUG(">%s< already existing - return existing information", path);
            return loadedJsonPathInfo;
        }
    }

    LOG_DEBUG("working now with path >%s< ", path);

    FREE(loadedJsonPathInfo->path);

    size_t pathInfo_path_len = ALLOC_PATH_STRLEN(path);
    loadedJsonPathInfo->path = MALLOC(pathInfo_path_len);
    MEMCHK(loadedJsonPathInfo->path) goto ERROR;
    strncpy(loadedJsonPathInfo->path, path, pathInfo_path_len);

    if (loadedJsonPathInfo->jsonObjectRoot) {
        json_decref(loadedJsonPathInfo->jsonObjectRoot);
        loadedJsonPathInfo->jsonObjectRoot = NULL;
    }

    if (fsh_httpconnector_ListPath(loadedJsonPathInfo->path, &httpResponseBuffer)) {
        LOG_ERR("fsh_httpconnector_ListPath() failed");
        goto ERROR;
    }

    if (httpResponseBuffer->size == 0) {
        LOG_ERR("Invalid httpResponseBuffer size : %zd", httpResponseBuffer->size);
        goto ERROR;
    }

    LOG_DEBUG(" httpResponseBuffer size : loadedJsonPathInfo=%s; httpResponseBuffer->size=%zd;  httpResponseBuffer->memory=%s",
              loadedJsonPathInfo->path, httpResponseBuffer->size, httpResponseBuffer->memory);

    if (!(loadedJsonPathInfo->jsonObjectRoot = json_loadb(httpResponseBuffer->memory, httpResponseBuffer->size, 0, &error))) {
        LOG_ERR("when loading JSON line:%d  error:%s ", error.line, error.text);
        goto ERROR;
    }

    LOG_DEBUG("returned buffer: %s", httpResponseBuffer->memory);

    /* Sanity check: verify the server actually returned data for the path we
     * requested. If the URL was malformed the server may return the root
     * listing for any path - we must not cache that under the wrong key.
     * Strategy: strip the trailing slash from our requested path and check
     * that at least one entry in the JSON array has a matching "path" field.
     * Exception: for the root path "/" we skip this check.                   */
    if (strncmp(loadedJsonPathInfo->path, "/", MAX_PATH_LEN) != 0) {
        /* Build the expected "path" value (without trailing slash) */
        char expectedPath[MAX_PATH_LEN];
        strncpy(expectedPath, loadedJsonPathInfo->path, MAX_PATH_LEN - 1);
        expectedPath[MAX_PATH_LEN - 1] = '\0';
        size_t epLen = strnlen(expectedPath, MAX_PATH_LEN);
        if (epLen > 1 && expectedPath[epLen - 1] == '/')
            expectedPath[epLen - 1] = '\0';

        int validated = 0;
        json_t *checkEntry;
        size_t checkIdx;
        json_array_foreach(loadedJsonPathInfo->jsonObjectRoot, checkIdx, checkEntry) {
            json_t *pField = json_object_get(checkEntry, "path");
            if (pField && json_is_string(pField)) {
                if (strncmp(json_string_value(pField), expectedPath, MAX_PATH_LEN) == 0) {
                    validated = 1;
                    break;
                }
            }
        }
        if (!validated) {
            LOG_ERR("Server returned data for wrong path! Requested >%s< but got entries for different path. Discarding.", loadedJsonPathInfo->path);
            json_decref(loadedJsonPathInfo->jsonObjectRoot);
            loadedJsonPathInfo->jsonObjectRoot = NULL;
            goto ERROR;
        }
    }

    result = loadedJsonPathInfo;
    goto EXIT;

ERROR:
    LOG_ERR("Leaving in an error condition.");
EXIT:
    if (httpResponseBuffer) {
        FREE(httpResponseBuffer->memory);
        httpResponseBuffer->size = 0;
    }
    FREE(httpResponseBuffer);
    return result;
}


/* *********************************************************************** */
json_t *getSubElementByNamedPath(char *path, json_t *root) {
    json_t *result_json;
    int i;

    LOG_DEBUG("searching for subelement >%s<", path);

    if (!path) { LOG_ERR("path not set"); return NULL; }
    if (!root)  { LOG_ERR("can't walk through folders if data is not loaded"); return NULL; }

    if (!json_is_array(root)) LOG_DEBUG("having single element no Array");

    json_array_foreach(root, i, result_json) {
        if (result_json == NULL) { LOG_ERR("json dataset error"); return NULL; }
        if (!json_is_object(result_json)) {
            LOG_ERR("result_json is not an object, type=%d", json_typeof(result_json));
            return NULL;
        }
        json_t *name = json_object_get(result_json, "name");
        if (name == NULL) { LOG_ERR("json dataset error"); return NULL; }
        if (json_is_string(name)) {
            const char *objName = json_string_value(name);
            LOG_DEBUG("Comparing searched: >%s< to found: >%s<", path, objName);
            if (objName == NULL) { LOG_ERR("json dataset error"); return NULL; }
            if (strncmp(path, objName, MAX_PATH_LEN) == 0) return result_json;
        }
    }
    return NULL;
}

/* *********************************************************************** */
int fsh_datacontainer_loadDir(WalkFolders_Callback_t callback, struct Fsh_DirLoaderRef_s *ref, int offset) {

    int ret = 0;
    LOG_DEBUG("Function entry with path >%s<", ref->path);
    PathInfo_t *pathInfo = NULL;

    int pathLen = strnlen(ref->path, MAX_PATH_LEN);
    if (ref->path[pathLen - 1] == '/') {
        pathInfo = updatePathInfo(ref->path);
    } else {
        char *tmpPath = MALLOC(pathLen + 2);
        MEMCHK(tmpPath) { ret = -1; goto EXIT; }
        strncpy(tmpPath, ref->path, pathLen + 1);
        tmpPath[pathLen]     = '/';
        tmpPath[pathLen + 1] = '\0';
        pathInfo = updatePathInfo(tmpPath);
        FREE(tmpPath);
    }

    if (!pathInfo) {
        LOG_ERR("updatePathInfo( %s ) failed.", ref->path);
        ret = -1;
        goto EXIT;
    }

    json_t *root = pathInfo->jsonObjectRoot;
    json_t *result_json;
    int i;

    if (!root) {
        LOG_ERR("can't walk through folders if data is not loaded");
        ret = -1;
        goto EXIT;
    }

    if (!json_is_array(root)) LOG_WARN("having single element no Array");

    int idx = 0;
    json_array_foreach(root, i, result_json) {
        if (idx < offset) { idx++; continue; }
        if (result_json && json_is_object(result_json)) {
            json_t *type = json_object_get(result_json, "type");
            json_t *path = json_object_get(result_json, "path");
            json_t *name = json_object_get(result_json, "name");
            if (path && type && name &&
                json_is_string(type) && json_is_string(path) && json_is_string(name)) {
                ref->filename = json_string_value(name);
                ref->path     = json_string_value(path);
                if (callback) callback(ref);
            }
        }
        idx++;
    }

EXIT:
    return ret;
}


/* *********************************************************************** */
/*
 * Returns a json_incref'd object — the CALLER must call json_decref() when
 * done. This decouples the caller's lifetime from the TLS cache: even if
 * the cache is replaced by a subsequent updatePathInfo() call in the same
 * thread, the returned pointer stays valid until explicitly released.
 */
json_t *getActualSubelement(const char *path) {

    size_t bufferLen = ALLOC_PATH_STRLEN(path);
    json_t *result_json = NULL;
    char *tmpPath = MALLOC(bufferLen + 1);
    MEMCHK(tmpPath) return NULL;

    memset(tmpPath, 0, bufferLen + 1);
    strncpy(tmpPath, path, bufferLen);

    /* Truncate to parent directory (keep trailing slash) */
    for (int i = bufferLen - 1; i >= 0; --i) {
        if (tmpPath[i] == '/') {
            tmpPath[i + 1] = '\0';
            break;
        }
    }

    LOG_DEBUG("calling updatePathInfo with >%s<", tmpPath);
    PathInfo_t *pathInfo = updatePathInfo(tmpPath);
    FREE(tmpPath);
    tmpPath = NULL;

    if (!pathInfo) { LOG_ERR("updatePathInfo() Failed"); goto ERROR; }

    json_t *root = pathInfo->jsonObjectRoot;
    if (!root) { LOG_ERR("Data is not loaded"); goto ERROR; }

    size_t pathLen = ALLOC_PATH_STRLEN(path);
    tmpPath = MALLOC(pathLen);
    MEMCHK(tmpPath) goto ERROR;
    strncpy(tmpPath, path, pathLen);

    json_t *found = getSubElementByNamedPath(basename(tmpPath), root);
    FREE(tmpPath);
    tmpPath = NULL;

    if (!found) { LOG_WARN("getSubElementByNamedPath() returned NULL, path: %s", path); goto ERROR; }
    if (!json_is_object(found)) { LOG_ERR("element is not a JSON object"); goto ERROR; }

    /* Caller owns this reference and must json_decref() it when done */
    result_json = json_incref(found);

ERROR:
    return result_json;
}


/* *********************************************************************** */
int fsh_datacontainer_getInfo(const char *path, struct Fsh_ObjectStat_s *file_info) {

    int ret = 0;
    LOG_DEBUG("working on >%s<", path);

    /* We own this reference — must json_decref() before returning */
    json_t *result_json = getActualSubelement(path);
    if (!result_json) {
        LOG_DEBUG("getActualSubelement returned NULL for >%s<", path);
        return -1;
    }

    json_t *type  = json_object_get(result_json, "type");
    json_t *atime = json_object_get(result_json, "created");

    if (type && json_is_string(type) && atime && json_is_string(atime)) {
        const char *timeStr = json_string_value(atime);
        struct tm parsedATime;
        memset(&parsedATime, 0, sizeof(parsedATime));
        strptime(timeStr, "%FT%T%z", &parsedATime);
        file_info->atime    = mktime(&parsedATime);
        file_info->filesize = 0;

        const char *t = json_string_value(type);
        LOG_DEBUG("path: %s, type: %s", path, t ? t : "(null)");

        if (t == NULL) { ret = -102; goto EXIT; }

        if (strncmp(t, "folder", 6) == 0) {
            file_info->type = FSH_STAT_TYPE_FOLDER;
        } else if (strncmp(t, "file", 4) == 0) {
            json_t *filesize = json_object_get(result_json, "size");
            if (filesize && json_is_integer(filesize)) {
                file_info->type     = FSH_STAT_TYPE_FILE;
                file_info->filesize = json_integer_value(filesize);
            } else {
                LOG_ERR("File object did not contain size information");
                ret = -102;
                goto EXIT;
            }
        } else {
            LOG_WARN("Unknown file type");
            ret = -100;
            goto EXIT;
        }
    }

EXIT:
    json_decref(result_json);   /* release our reference */
    return ret;
}


/* *********************************************************************** */
int fsh_datacontainer_openFile(const char *newPath) {

    int  result = -1;
    /* blobID copied out of JSON immediately so json_decref can happen before
     * any further calls that might trigger updatePathInfo() again            */
    char blobID[BLOB_ID_LEN + 1];
    memset(blobID, 0, sizeof(blobID));

    LOG_INFO("Opening file >%s< ", newPath);

    /* We own this reference — must json_decref() before returning */
    json_t *result_json = getActualSubelement(newPath);
    if (!result_json) { result = -1; goto ERROR; }

    json_t *blob = json_object_get(result_json, "blob");
    if (!(blob && json_is_string(blob))) {
        result = -2;
        json_decref(result_json);
        goto ERROR;
    }

    /* Copy blobID into local buffer BEFORE releasing the JSON reference */
    strncpy(blobID, json_string_value(blob), BLOB_ID_LEN);
    json_decref(result_json);
    result_json = NULL;

    LOG_DEBUG("Extracted blob: >%s<", blobID);

    if (fsh_openfilemanager_getFileContext(newPath)) {
        LOG_DEBUG("File already opened: >%s<", newPath);
        result = 0;
        goto EXIT;
    }

    FileMemoryStruct_t *file = fsh_openfilemanager_openFile(newPath, blobID);
    if (!file) { result = -3; goto ERROR; }

    if (fsh_httpconnector_OpenFile(file->blobID, &file->memory)) {
        LOG_ERR("fsh_httpconnector_OpenFile() failed");
        result = -4;
        goto ERROR;
    }

    result = 0;
EXIT:
ERROR:
    return result;
}


/* *********************************************************************** */
FileMemoryStruct_t *fsh_datacontainer_readFile(const char *newPath) {

    FileMemoryStruct_t *result = NULL;
    char blobID[BLOB_ID_LEN + 1];
    memset(blobID, 0, sizeof(blobID));

    LOG_DEBUG("read file >%s< ", newPath);

    /* We own this reference — must json_decref() before returning */
    json_t *result_json = getActualSubelement(newPath);
    if (!result_json) goto EXIT;

    json_t *blob = json_object_get(result_json, "blob");
    if (!(blob && json_is_string(blob))) {
        json_decref(result_json);
        goto EXIT;
    }

    /* Copy blobID into local buffer BEFORE releasing the JSON reference */
    strncpy(blobID, json_string_value(blob), BLOB_ID_LEN);
    json_decref(result_json);
    result_json = NULL;

    LOG_DEBUG("Extracted blob: >%s<", blobID);

    FileMemoryStruct_t *file = fsh_openfilemanager_getFileContext(newPath);
    if (!file) {
        LOG_ERR("getFileContext returned NULL, trying to open: >%s<", newPath);
        file = fsh_openfilemanager_openFile(newPath, blobID);
    }
    if (!file) { LOG_ERR("Could not open file: >%s<", newPath); goto EXIT; }

    if (strncmp(blobID, file->blobID, BLOB_ID_LEN) != 0) {
        LOG_ERR("BlobID mismatch for >%s<: expected >%s< got >%s<", newPath, blobID, file->blobID);
        goto EXIT;
    }

    result = file;

EXIT:
    return result;
}

/* *********************************************************************** */
int fsh_datacontainer_createFile(const char *newFile) {
    LOG_DEBUG("create new File file >%s< ", newFile);
    int result = -1;
    char timestamp[] = "YYYY-MM-ddTHH:mm:ss.SSS+0000";
    json_t *newFileEntry = NULL;

    char *tmpPath = MALLOC(ALLOC_PATH_STRLEN(newFile));
    MEMCHK(tmpPath) { result = 7; goto ERROR; }
    strncpy(tmpPath, newFile, ALLOC_PATH_STRLEN(newFile));

    char *tmpFile = MALLOC(ALLOC_PATH_STRLEN(newFile));
    MEMCHK(tmpFile) { result = 7; goto ERROR; }
    strncpy(tmpFile, newFile, ALLOC_PATH_STRLEN(newFile));

    char *pathName = dirname(tmpPath);
    char *fileName = basename(tmpFile);

    LOG_DEBUG("directory of new File >%s< Filename >%s<", pathName, fileName);

    PathInfo_t *pathInfo = updatePathInfo(pathName);
    if (!pathInfo || !pathInfo->jsonObjectRoot) {
        LOG_ERR("Haven't found path to create file");
        goto ERROR;
    }

    if (!json_is_array(pathInfo->jsonObjectRoot)) LOG_WARN("having single element no Array");

    time_t now;
    time(&now);
    strftime(timestamp, sizeof timestamp, "%FT%TZ", gmtime(&now));

    newFileEntry = json_object();
    json_object_set_new(newFileEntry, "path",     json_string(pathName));
    json_object_set_new(newFileEntry, "name",     json_string(fileName));
    json_object_set_new(newFileEntry, "type",     json_string("file"));
    json_object_set_new(newFileEntry, "size",     json_integer(0));
    json_object_set_new(newFileEntry, "created",  json_string(timestamp));
    json_object_set_new(newFileEntry, "modified", json_string(timestamp));
    json_object_set_new(newFileEntry, "state",    json_string("new"));

    json_array_append(pathInfo->jsonObjectRoot, newFileEntry);
    result = 0;
    goto EXIT;

ERROR:
    LOG_WARN("Leave create file in an error condition");
EXIT:
    if (newFileEntry) json_decref(newFileEntry);
    FREE(tmpPath);
    FREE(tmpFile);
    return result;
}


/* *********************************************************************** */
int fsh_datacontainer_closeFile(const char *path) {
    UNIMPLEMENTED("need to implement HTTP transfer on close file");
    return fsh_openfilemanager_closeFile(path);
}


/* *********************************************************************** */
void fsh_datacontainer_container_destroy() {
    LOG_DEBUG("calling destroy chain");

    PathInfo_t *info = get_thread_pathinfo();
    if (info) {
        if (info->jsonObjectRoot) {
            json_decref(info->jsonObjectRoot);
            info->jsonObjectRoot = NULL;
        }
        FREE(info->path);
        FREE(info);
        pthread_setspecific(tls_pathinfo_key, NULL);
    }

    fsh_openfilemanager_closeAllFiles();
}

