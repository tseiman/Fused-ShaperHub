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
#include <linux/limits.h>
#include <string.h>
#include <libgen.h>

// the defines are needed to get strptime()
#define __USE_XOPEN  
#define _GNU_SOURCE
#include <time.h>


#include <alloc.h>
#include <data-container.h>
#include <global.h>
#include <http-connector.h>
#include <messages.h>

static PathInfo_t pathInfo = {NULL, NULL};

int comparePathLevel(char *a, char *b) {
    int i,j = 0;
    for (i=0; a[i]; a[i]=='/' ? i++ : *a++);
    for (j=0; b[j]; b[j]=='/' ? j++ : *b++);

    return i - j;

}

PathInfo_t *updatePathInfo(const char *path) {
    
    MemoryStruct_t *httpResponseBuffer = NULL;
    json_error_t error;
    PathInfo_t *result = NULL;

    LOG_DEBUG("Entry with path >%s< ", path);

    if ((!path) && pathInfo.jsonObjectRoot && pathInfo.path)
        return &pathInfo; // if path is NULL but we've some info stored we return that

    if (pathInfo.jsonObjectRoot && pathInfo.path) {
        if (strncmp(path, pathInfo.path, MAX_PATH_LEN) == 0) { // Nothing to do as we have not changed the path, can work on the same JSON structure
            LOG_DEBUG(">%s< already existing - return existing information", path);
            return &pathInfo;
        }
    }

    LOG_DEBUG("working now with path >%s< ", path);

    if (pathInfo.path) FREE(pathInfo.path); // prevent allocating again on an already allocated memory
    pathInfo.path = NULL;
    if (!(pathInfo.path = MALLOC(strnlen(path, MAX_PATH_LEN) + 1))) {
        LOG_ERR("Could not allocate Memory for pathInfo.path");
        goto ERROR;
    }

    strncpy(pathInfo.path, path, strnlen(path, MAX_PATH_LEN) + 1);

    if (pathInfo.jsonObjectRoot) {
        json_decref(pathInfo.jsonObjectRoot);
        pathInfo.jsonObjectRoot = NULL;
    }

    if (fsh_HTTPListPath(pathInfo.path, &httpResponseBuffer)) {
        LOG_ERR("fsh_HTTPListPath() failed");
        goto ERROR;
    }

    if (httpResponseBuffer->size == 0) { /* if only 1 byte is allocated (which is the initial allocation of the buffer) then the response was not ok */
        LOG_ERR("Invalid httpResponseBuffer size : %zd", httpResponseBuffer->size);
        goto ERROR;
    }

    LOG_DEBUG(" httpResponseBuffer size : %s/%zd/%s", pathInfo.path, httpResponseBuffer->size, httpResponseBuffer->memory);
    if (!(pathInfo.jsonObjectRoot = json_loadb(httpResponseBuffer->memory, httpResponseBuffer->size, 0, &error))) {
        LOG_ERR("when loading JSON line:%d  error:%s ", error.line, error.text);
        goto ERROR;
    }
    LOG_DEBUG("returned buffer: %s", httpResponseBuffer->memory);

    result = &pathInfo;

    goto EXIT;
   
ERROR:
    LOG_ERR("Leaving in an error condition.");
EXIT:
    if(httpResponseBuffer) { 
        if(httpResponseBuffer->memory) FREE(httpResponseBuffer->memory);
        httpResponseBuffer->memory = NULL;
        httpResponseBuffer->size = 0;
    }

    if (httpResponseBuffer) FREE(httpResponseBuffer);
    httpResponseBuffer = NULL;

    return result;
}


json_t *getSubElementByNamedPath(char *path, json_t *root) {
    json_t *result_json;
    int i;

    LOG_DEBUG("searching for subelement >%s<",path);

    if (!root) {
        LOG_ERR("can't walk through folders if data is not loaded");
        return NULL;
    }
    if (!json_is_array(root)) {
        LOG_DEBUG("having single element no Array");
    }

    json_array_foreach(root, i, result_json) {

         json_t *name = json_object_get(result_json, "name");

        if (name && json_is_string(name)) {
            char *objName = (char *) json_string_value(name);
            LOG_DEBUG("Comparing searched: >%s< to found: >%s<",path, objName);
            if(strncmp(path, objName, MAX_PATH_LEN) == 0) {
            
                return result_json;
            }
        }
    }

   return NULL;
}

int fsh_walkFolders(WalkFolders_Callback_t callback, struct Fsh_DirLoaderRef_s *ref) {

    LOG_DEBUG("Function entry with path >%s<", ref->path);
    PathInfo_t *pathInfo = NULL;

    int pathLen = strlen(ref->path);
    if(ref->path[pathLen -1] == '/') {
        pathInfo = updatePathInfo(ref->path);
    } else {
        char *tmpPath = MALLOC(pathLen + 1);
        if(!tmpPath) {
            LOG_ERR("cant allocate memory");
            return -1;
        }
        strcpy(tmpPath,ref->path);
        tmpPath[pathLen] = '/';
        tmpPath[pathLen +1] = '\0';

        pathInfo = updatePathInfo(tmpPath);
        FREE(tmpPath);
    }


    if (!pathInfo) {
        LOG_ERR("updatePathInfo( %s ) failed.", ref->path);
        return -1;
    }
    json_t *root = pathInfo->jsonObjectRoot;
    json_t *result_json;
    int i;

    if (!root) {
        LOG_ERR("can't walk through folders if data is not loaded");
        return -1;
    }

    if (!json_is_array(root)) {
        LOG_DEBUG("having single element no Array");
    }

    json_array_foreach(root, i, result_json) {

        if (result_json && json_is_object(result_json)) {
            json_t *type = json_object_get(result_json, "type");

            json_t *path = json_object_get(result_json, "path");
            json_t *name = json_object_get(result_json, "name");

            if (path && type && name && json_is_string(type) && json_is_string(path) && json_is_string(name) ) {
                ref->filename = json_string_value(name);
                ref->path = json_string_value(path);
                LOG_DEBUG("Calling callback for folder update");
                callback(ref);
            }
        }
    }  /* END looping trough the current listing */

    return 0;
}

int fsh_getInfo(const char *path, struct Fsh_ObjectStat_s *file_info) {

    LOG_DEBUG("working on >%s<", path);


    char *tmpPath = MALLOC(strlen(path) + 1);
    if(!tmpPath) {
        LOG_ERR("cant allocate memory");
        return -1;
    }
    strcpy(tmpPath,path);
    for(int i = strlen(tmpPath); i >= 0; --i) {
        if(tmpPath[i] == '/') {
            tmpPath[i+1] = '\0';
            break;
        }
    }

    LOG_DEBUG("calling updatePathInfo with >%s<", tmpPath);

    PathInfo_t *pathInfo = updatePathInfo(tmpPath);

    FREE(tmpPath);
 
 
    if (!pathInfo) {
        LOG_ERR("updatePathInfo() Failed");
        return -2;
    }

    json_t *root = pathInfo->jsonObjectRoot;
    json_t *result_json;

    if (!root) {
        LOG_ERR("Data is not loaded");
        return -1;
    }


    result_json = getSubElementByNamedPath(basename((char *)path) , root);

    if (!result_json) {
        LOG_WARN("getSubElementByNamedPath() retuned NULL, using path: %s", path);
        return -2;
    }

    if (!json_is_object(result_json)) {
        LOG_ERR("commit data is not an object");
        return -3;
    }

    json_t *type = json_object_get(result_json, "type");
    json_t *atime = json_object_get(result_json, "created");


    if (type && json_is_string(type) && atime && json_is_string(atime)) {

            char *timeStr = (char *) json_string_value(atime);

            struct tm parsedATime;
            memset(&parsedATime, 0, sizeof(parsedATime));
            strptime(timeStr, "%FT%T%z", &parsedATime);
            file_info->atime = mktime(&parsedATime);

            file_info->filesize = 0;
            LOG_DEBUG("path: %s, type: %s", path, json_string_value(type));

            if (strcmp(json_string_value(type), "folder") == 0) {
                file_info->type = FSH_STAT_TYPE_FOLDER;
            } else if (strcmp(json_string_value(type), "file") == 0) {

                json_t *filesize = json_object_get(result_json, "size");
                if (filesize && json_is_integer(filesize)) {
                    file_info->type = FSH_STAT_TYPE_FILE;
                    file_info->filesize = json_integer_value(filesize);
                } else {
                    LOG_ERR("File object did not contain size information");
                    return -101;
                }

            } else {
                LOG_WARN("Unknown file type");
                return -100;
            }
        }
  
    return 0;
}

int fsh_getLinkInfo(const char *path, char *linkDstPath, size_t size) {
    LOG_DEBUG("fsh_getLinkInfo %s", path);
    return 0;
}

void fsh_container_destroy() {
  	LOG_DEBUG("calling destroy chain");
    if (pathInfo.jsonObjectRoot) {
        json_decref(pathInfo.jsonObjectRoot);
        pathInfo.jsonObjectRoot = NULL;
    }
    if (pathInfo.path) {
        FREE(pathInfo.path);
        pathInfo.path = NULL;
    }
 

}
