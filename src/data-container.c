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
#include <strings.h>

#include <global.h>
#include <messages.h>
#include <alloc.h>
#include <data-container.h>
#include <http-connector.h>

PathInfo_t *updatePathInfo(const char *path) {
    static PathInfo_t pathInfo = {NULL, NULL};
    MemoryStruct_t *httpResponseBuffer = NULL;
    json_error_t error;
    PathInfo_t *result = NULL;

    LOG_DEBUG("updatePathInfo( %s ) ", path);

    if((!path) && pathInfo.jsonObjectRoot && pathInfo.path) return &pathInfo;  // if path is NULL but we've some info stored we return that

    if(pathInfo.jsonObjectRoot && pathInfo.path) {
        if(strncmp(path,pathInfo.path, MAX_PATH_LEN) == 0) { // Nothing to do as we have not changed the path, can work on the same JSON structure
            LOG_DEBUG("updatePathInfo( %s ) return existing information",path);
            return &pathInfo;
        }
    }

    LOG_DEBUG("updatePathInfo( %s ) ", path);
    if(pathInfo.path) {
        FREE(pathInfo.path);
        pathInfo.path = NULL;
    }

    if(!( pathInfo.path = MALLOC(strnlen(path, MAX_PATH_LEN) + 1))) {
            LOG_ERR("Could not allocate Memory for pathInfo.path");
            return NULL;
    } 

    strncpy(pathInfo.path, path, strnlen(path, MAX_PATH_LEN) +1 );

    if(pathInfo.jsonObjectRoot) {
        json_decref(pathInfo.jsonObjectRoot);
        pathInfo.jsonObjectRoot = NULL;
    }

    if(fsh_HTTPListPath(pathInfo.path, &httpResponseBuffer)) {
         LOG_ERR("fsh_HTTPListPath() failed");
         return NULL;
    }
    

    if(httpResponseBuffer->size == 0) { /* if only 1 byte is allocated (which is the initial allocation of the buffer) then the response was not ok */
        LOG_ERR("Invalid httpResponseBuffer size : %zd", httpResponseBuffer->size);
        goto ERROR; 
    }

    LOG_DEBUG(" httpResponseBuffer size : %s/%zd/%s", pathInfo.path,httpResponseBuffer->size,httpResponseBuffer->memory);
    if(!(pathInfo.jsonObjectRoot = json_loadb(httpResponseBuffer->memory,httpResponseBuffer->size, 0, &error))) {
        LOG_ERR("when loading JSON line:%d  error:%s ", error.line, error.text);
        goto ERROR;
    }
    LOG_DEBUG("returned buffer: %s", httpResponseBuffer->memory );

    result = &pathInfo;

ERROR:
        if(!httpResponseBuffer->memory) FREE(httpResponseBuffer->memory);
        httpResponseBuffer->memory = NULL;
        httpResponseBuffer->size = 0;
        if(!httpResponseBuffer) FREE(httpResponseBuffer);
        httpResponseBuffer = NULL;

    return result;
}

/*
json_t *getSubElementByNamedPath(char *path, json_t *rootElement) {
    unsigned int delimiterPos = 0;
    char *tmpPath = strndup(path, PATH_MAX);
    char *tmpPath_P = tmpPath;
    char *tmpPath_StrChr_P;
    json_t *result_json;
    json_t *tmp_json;

    if (!rootElement) {
        LOG_ERR("can't walk through folders if data is not loaded");
        return (json_t *)-1;
    }

    //    delimiterPos = (unsigned int) (strchr(tmpPath_P, '/') - tmpPath_P);
    tmpPath_StrChr_P = strchr(tmpPath_P, '/');
    delimiterPos = (unsigned int)(tmpPath_StrChr_P - tmpPath_P);

    if ((tmpPath_StrChr_P == tmpPath_P) && (strnlen(tmpPath_P, PATH_MAX) > 1)) {
        ++tmpPath_P;
        tmpPath_StrChr_P = strchr(tmpPath_P, '/');
        delimiterPos = (unsigned int)(tmpPath_StrChr_P - tmpPath_P);
    }
    if (tmpPath_StrChr_P != NULL) {
        tmpPath_P[delimiterPos] = '\0';
    }

    // printf("-------- tmpPath: %s, %lx, %lx\n", tmpPath_P, (unsigned long)(tmpPath_P + delimiterPos + 1), (unsigned long)(tmpPath_P + strnlen(path, PATH_MAX)));

    tmp_json = json_object_get(rootElement, tmpPath_P);
    result_json = tmp_json;

    if (((tmpPath_P + delimiterPos + 1) < (tmpPath_P + strnlen(path, PATH_MAX))) && tmp_json) {
        // printf("-------- going to recursion\n");
        tmp_json = getSubElementByNamedPath(tmpPath_P + delimiterPos + 1, tmp_json);
    }
    if (tmp_json) {
        result_json = tmp_json;
    }

    //    result_json = json_object_get();
    FREE(tmpPath);
    return result_json;
}
*/
int fsh_walkFolders(WalkFolders_Callback_t callback, struct Fsh_DirLoaderRef_s *ref) {

    LOG_DEBUG("fsh_walkFolders( %s )", ref->path);

    PathInfo_t *pathInfo = updatePathInfo(ref->path);

    if(!pathInfo) {
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

    if(!json_is_array(root)) {
        LOG_DEBUG("fsh_walkFolders is single element no Array");

    }

  

    json_array_foreach(root, i, result_json){ 

        LOG_DEBUG("fsh_walkFolders----------------------------------- %d",i);
 
        if (result_json && json_is_object(result_json)) {
           json_t *type = json_object_get(result_json, "type");
  
            json_t *path = json_object_get(result_json, "path");
            json_t *name = json_object_get(result_json, "name");

            if(path && type && name &&  json_is_string(type) &&  json_is_string(path) &&  json_is_string(name) && (strcmp(json_string_value(type), "folder") == 0)) {

                ref->filename = json_string_value(name);
                ref->path = json_string_value(path);
                callback(ref);
            }

        }


    }

/*
  LOG_DEBUG("fsh_walkFolders-----------------------------------");
    void *iter = json_object_iter(root);
    while (iter) {
        LOG_DEBUG("fsh_walkFolders2-----------------------------------");
        const char *key = json_object_iter_key(iter);
        json_t *value = json_object_iter_value(iter);
        LOG_DEBUG("Walking through JSON: %s",key);
        
        iter = json_object_iter_next(result_json, iter);
    } 
    
    */
    
    /* END looping trough the current listing */


#ifdef NOTUSED

    //    result_json = getSubElementByNamedPath("/Cloud_Action/TS_actionOn_ObsLightValueSuperCloudAction/v2", root);
    //    result_json = getSubElementByNamedPath("/Local_Action/actionOn_ObsLightValue/v1", root);
    //    result_json = getSubElementByNamedPath("/", root);

    result_json = (strcmp(ref->path, "/") == 0) ? root : getSubElementByNamedPath((char *)ref->path, root);

    if (!result_json) {
        LOG_ERR("getSubElementByNamedPath() retuned NULL");
        return -2;
    }

    if (!json_is_object(result_json)) {
        LOG_ERR("commit data is not an objec");
        return -3;
    }

    void *iter = json_object_iter(result_json);
    while (iter) {
        const char *key = json_object_iter_key(iter);
        json_t *value = json_object_iter_value(iter);

        if (value && json_is_object(value)) {
            //	    json_t *type = json_object_get(value, "type");

            //	    if(type && json_is_string(type) && (strcmp(json_string_value(type), "folder") == 0)) {

            // printf("-------- going to load33 %s \n", key);

            ref->filename = key;
            callback(ref);
            //	    }
        }

        iter = json_object_iter_next(result_json, iter);
    } /* END looping trough all elements of this level */

    result_json = json_object_get(result_json, "atime");

    //    LOG_DEBUG("json integer value: %d", (int)json_integer_value(result_json));
    /*
        if(!json_is_array(root)) {
            fprintf(stderr, "error: data root is not an array\n");
    //	json_decref(root);
            return -2;
        }
    */

    /*
        for(i = 0; i < json_array_size(root); i++) {
            json_t *data, *type, *dir_name;

            data = json_array_get(root, i);
            if(!json_is_object(data)) {
                fprintf(stderr, "error: commit data %d is not an object\n", i + 1);
    //	    json_decref(root);
                return -3;
            }

            type = json_object_get(data, "type");
            dir_name = json_object_get(data, "name");
            if(json_is_string(type) && json_is_string(dir_name)) {
                if (strcmp(json_string_value(type), "folder") == 0) {

                    ref->filename = json_string_value(dir_name);
                    callback(ref);
    //		printf("found folder: %s\n", json_string_value(dir_name));
                }

            }
        } */

#endif
    return 0;
}

int fsh_getInfo(const char *path, struct Fsh_ObjectStat_s *file_info) {

    LOG_DEBUG("fsh_getInfo %s", path);

    PathInfo_t *pathInfo = updatePathInfo(path);

    if(!pathInfo) {
        LOG_ERR("updatePathInfo() Failed");
        return -2;
    }

    json_t *root = pathInfo->jsonObjectRoot;
//    json_t *result_json;


    if (!root) {
        LOG_ERR("Data is not loaded");
        return -1;
    }
/*
    result_json = getSubElementByNamedPath((char *)path, root);

    if (!result_json) {
        LOG_WARN("getSubElementByNamedPath() retuned NULL, using path: %s", path);
        return -2;
    }

    if (!json_is_object(result_json)) {
        LOG_ERR("commit data is not an object");
        return -3;
    }

    json_t *type = json_object_get(result_json, "type");
    //    json_t *onlinePath = json_object_get(result_json, "onlinePath");
    json_t *atime = json_object_get(result_json, "atime");

    //  && onlinePath && json_is_string(onlinePath)

    if (type && json_is_string(type) && atime && json_is_integer(atime)) {

        //	file_info->onlinePath = (char *) json_string_value(onlinePath);
        file_info->atime = json_integer_value(atime);
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
        } else if (strcmp(json_string_value(type), "reference") == 0) {
            file_info->type = FSH_STAT_TYPE_REFERENCE;
        } else {
            return -100;
        }
    }
*/
    return 0;
}

int fsh_getLinkInfo(const char *path, char *linkDstPath, size_t size) {
    LOG_DEBUG("fsh_getLinkInfo %s", path);
/*
    json_t *root = setGetRoot(NULL);
    json_t *result_json;

    if (!root) {
        LOG_ERR("can't get Info if data is not loaded");
        return -1;
    }

    result_json = getSubElementByNamedPath((char *)path, root);

    if (!result_json) {
        LOG_ERR("getSubElementByNamedPath() retuned NULL");
        return -2;
    }

    if (!json_is_object(result_json)) {
        LOG_ERR("commit data is not an object");
        return -3;
    }

    json_t *reference = json_object_get(result_json, "reference");

    if (reference && json_is_string(reference)) {
        strncpy(linkDstPath, json_string_value(reference), size);
    }
*/
    return 0;
}
