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


#include <global.h>
#include <alloc.h>
#include <messages.h>
#include <data-container.h>
#include <http-connector.h>
#include <open-file-manager.h>



static PathInfo_t loadedJsonPathInfo = {NULL, NULL};

/* *********************************************************************** */
/* *********************************************************************** */
int comparePathLevel(char *a, char *b) {
    int i,j = 0;
    for (i=0; a[i]; a[i]=='/' ? i++ : *a++);
    for (j=0; b[j]; b[j]=='/' ? j++ : *b++);

    return i - j;

}



/* *********************************************************************** */
/* *********************************************************************** */
PathInfo_t *updatePathInfo(const char *path) {
    
    MemoryStruct_t *httpResponseBuffer = NULL;
    json_error_t error;
    PathInfo_t *result = NULL;

    LOG_DEBUG("Entry with path >%s< ", path);

    if ((!path) && loadedJsonPathInfo.jsonObjectRoot && loadedJsonPathInfo.path)
        return &loadedJsonPathInfo; // if path is NULL but we've some info stored we return that

    if (loadedJsonPathInfo.jsonObjectRoot && loadedJsonPathInfo.path) {
        if (strncmp(path, loadedJsonPathInfo.path, MAX_PATH_LEN) == 0) { // Nothing to do as we have not changed the path, can work on the same JSON structure
            LOG_DEBUG(">%s< already existing - return existing information", path);
            return &loadedJsonPathInfo;
        }
    }

    LOG_DEBUG("working now with path >%s< ", path);

    FREE(loadedJsonPathInfo.path); // prevent allocating again on an already allocated memory

    size_t pathInfo_path_len = ALLOC_PATH_STRLEN(path);
    loadedJsonPathInfo.path =  MALLOC(pathInfo_path_len);

    MEMCHK(loadedJsonPathInfo.path) goto ERROR;
    strncpy(loadedJsonPathInfo.path, path, pathInfo_path_len);

    if (loadedJsonPathInfo.jsonObjectRoot) {
        json_decref(loadedJsonPathInfo.jsonObjectRoot);
        loadedJsonPathInfo.jsonObjectRoot = NULL;
    }

    if (fsh_httpconnector_ListPath(loadedJsonPathInfo.path, &httpResponseBuffer)) {
        LOG_ERR("fsh_httpconnector_ListPath() failed");
        goto ERROR;
    }

    if (httpResponseBuffer->size == 0) { /* if only 1 byte is allocated (which is the initial allocation of the buffer) then the response was not ok */
        LOG_ERR("Invalid httpResponseBuffer size : %zd", httpResponseBuffer->size);
        goto ERROR;
    }
    LOG_DEBUG(" httpResponseBuffer size : %s/%zd/%s", loadedJsonPathInfo.path, httpResponseBuffer->size, httpResponseBuffer->memory);
    if (!(loadedJsonPathInfo.jsonObjectRoot = json_loadb(httpResponseBuffer->memory, httpResponseBuffer->size, 0, &error))) {
        LOG_ERR("when loading JSON line:%d  error:%s ", error.line, error.text);
        goto ERROR;
    }
    LOG_DEBUG("returned buffer: %s", httpResponseBuffer->memory);

    result = &loadedJsonPathInfo;

    goto EXIT;
   
ERROR:
    LOG_ERR("Leaving in an error condition.");
EXIT:
    if(httpResponseBuffer) {
        FREE(httpResponseBuffer->memory);
        httpResponseBuffer->size = 0;
    }
    FREE(httpResponseBuffer);

    return result;
}


/* *********************************************************************** */
/* *********************************************************************** */
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
            const char *objName = json_string_value(name);
            LOG_DEBUG("Comparing searched: >%s< to found: >%s<",path, objName);
            if(strncmp(path, objName, MAX_PATH_LEN) == 0) {
            
                return result_json;
            }
        }
    }

   return NULL;
}

/* *********************************************************************** */
/* *********************************************************************** */
int fsh_datacontainer_loadDir(WalkFolders_Callback_t callback, struct Fsh_DirLoaderRef_s *ref) {

    LOG_DEBUG("Function entry with path >%s<", ref->path);
    PathInfo_t *pathInfo = NULL;

    int pathLen = strnlen(ref->path, MAX_PATH_LEN);
    if(ref->path[pathLen -1] == '/') {
        pathInfo = updatePathInfo(ref->path);
    } else {
        char *tmpPath = MALLOC(pathLen + 1);
        MEMCHK(tmpPath) return -1;

        strncpy(tmpPath,ref->path, pathLen + 1);
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
        LOG_WARN("having single element no Array");
    }

    json_array_foreach(root, i, result_json) {

        if (result_json && json_is_object(result_json)) {
            json_t *type = json_object_get(result_json, "type");

            json_t *path = json_object_get(result_json, "path");
            json_t *name = json_object_get(result_json, "name");

            if (path && type && name && json_is_string(type) && json_is_string(path) && json_is_string(name) ) {
                ref->filename = json_string_value(name);
                ref->path = json_string_value(path);
                if(callback) callback(ref);
            }
        }
    }  /* END looping trough the current listing */

    return 0;
}

/*
int fsh_datacontainer_loadDir(WalkFolders_Callback_t callback, struct Fsh_DirLoaderRef_s *ref) {
    walkFolders(ref);
    LOG_DEBUG("Calling callback for folder update");
    callback(ref);
}
*/

/* *********************************************************************** */
/* *********************************************************************** */
json_t *getActualSubelement(const char *path) {

    size_t bufferLen = ALLOC_PATH_STRLEN(path);
    json_t *result_json = NULL;
    char *tmpPath = MALLOC(bufferLen);
    MEMCHK(tmpPath) return NULL;
    
    strncpy(tmpPath,path, bufferLen);
    for(int i = bufferLen - 1; i >= 0; --i) {
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
        result_json = NULL;
        goto ERROR;
    }

    json_t *root = pathInfo->jsonObjectRoot;

    if (!root) {
        LOG_ERR("Data is not loaded");
        result_json = NULL;
        goto ERROR;
    }
    size_t pathLen = ALLOC_PATH_STRLEN(path);
    tmpPath = MALLOC(pathLen);
    MEMCHK(tmpPath) {
        result_json = NULL;
        goto ERROR;
    }
    strncpy(tmpPath, path,pathLen);

    result_json = getSubElementByNamedPath(basename(tmpPath) , root);
    FREE(tmpPath);

    if (!result_json) {
        LOG_WARN("getSubElementByNamedPath() retuned NULL, using path: %s", path);
        result_json = NULL;
        goto ERROR;
    }

    if (!json_is_object(result_json)) {
        LOG_ERR("commit data is not an object");
        result_json = NULL;
        goto ERROR;
    }

ERROR:
    

    return result_json;
}


/* *********************************************************************** */
/* *********************************************************************** */
int fsh_datacontainer_getInfo(const char *path, struct Fsh_ObjectStat_s *file_info) {

    LOG_DEBUG("working on >%s<", path);

    json_t * result_json = getActualSubelement(path);
    if (!result_json) return -1;

    json_t *type = json_object_get(result_json, "type");
    json_t *atime = json_object_get(result_json, "created");


    if (type && json_is_string(type) && atime && json_is_string(atime)) {

            const char *timeStr = json_string_value(atime);

            struct tm parsedATime;
            memset(&parsedATime, 0, sizeof(parsedATime));
            strptime(timeStr, "%FT%T%z", &parsedATime);
            file_info->atime = mktime(&parsedATime);

            file_info->filesize = 0;
            LOG_DEBUG("path: %s, type: %s", path, json_string_value(type));

            if (strncmp(json_string_value(type), "folder",6) == 0) {
                file_info->type = FSH_STAT_TYPE_FOLDER;
            } else if (strncmp(json_string_value(type), "file",4) == 0) {

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




/* *********************************************************************** */
/* *********************************************************************** */
int fsh_datacontainer_openFile(const char *newPath) {
    int result = -1;

   
    LOG_INFO("Opening file >%s< ", newPath);

    json_t * result_json = getActualSubelement(newPath);
    if (!result_json) return -1;

    json_t *blob = json_object_get(result_json, "blob");
 
    if (!(blob && json_is_string(blob))) return -2;
    

    const char *newBlobID = json_string_value(blob);
    LOG_DEBUG("Extracted a blob from curent Folder Structure data:  >%s<", newBlobID);



    if(fsh_openfilemanager_getFileContext(newPath)) {
        LOG_DEBUG("File already opened: >%s<", newPath);
        return 0;
    }
    /*
    if(newBlobID && strncmp(newBlobID,file->blobID,BLOB_ID_LEN) == 0) { 
        LOG_DEBUG("File still in cache: >%s<", newPath);
        return 0;
    }
*/


    FileMemoryStruct_t *file = fsh_openfilemanager_openFile(newPath,newBlobID);
/*
    if(newBlobID && strncmp(newBlobID,file->blobID,BLOB_ID_LEN) == 0) { 
        LOG_DEBUG("File still in cache: >%s<", newPath);
        return 0;
    }
*/
//    if(file->memory) FREE(file->memory->memory);
 //   FREE(file->memory);
 //   memset(file->blobID,0,BLOB_ID_LEN + 1);
 //   strncpy(file->blobID,newBlobID,BLOB_ID_LEN + 1);

    if (fsh_httpconnector_OpenFile(file->blobID, &file->memory)) {
        LOG_ERR("fsh_httpconnector_OpenFile() failed");
        goto ERROR;
    }

ERROR:
    return result;

}



/* *********************************************************************** */
/* *********************************************************************** */
FileMemoryStruct_t *fsh_datacontainer_readFile(const char *newPath) {

   FileMemoryStruct_t *result = NULL;

   
    LOG_DEBUG("read file >%s< ", newPath);

    json_t * result_json = getActualSubelement(newPath);
    if (!result_json) return result;

    json_t *blob = json_object_get(result_json, "blob");
 
    if (!(blob && json_is_string(blob))) return result;
    

    const char *newBlobID = json_string_value(blob);
    LOG_DEBUG("Extracted a blob from curent Folder Structure data:  >%s<", newBlobID);

    FileMemoryStruct_t *file = fsh_openfilemanager_getFileContext(newPath);

    if(!file) {
        LOG_ERR("fsh_openfilemanager_getFileContext returned in an error condition ! : >%s<", newPath);
        return result;  
    }

    if( newBlobID && (strncmp(newBlobID,file->blobID,BLOB_ID_LEN) != 0)) { 
        LOG_ERR("File was not opnend !!!!!: >%s<", newPath);
        return result;
    }

    return file;
}
/* *********************************************************************** */
/* *********************************************************************** */
int fsh_datacontainer_createFile(const char *newFile) {
    LOG_DEBUG("create new File file >%s< ", newFile);
    int result = -1;
    char timestamp[] = "YYYY-MM-ddTHH:mm:ss.SSS+0000";

    char *tmpPath = MALLOC(ALLOC_PATH_STRLEN(newFile));
    MEMCHK(tmpPath) {
        result = 7;
        goto ERROR;
    }
    strncpy(tmpPath, newFile, ALLOC_PATH_STRLEN(newFile));
    char *tmpFile = MALLOC(ALLOC_PATH_STRLEN(newFile));
    MEMCHK(tmpFile) {
        result = 7;
        goto ERROR;
    }
    strncpy(tmpFile, newFile, ALLOC_PATH_STRLEN(newFile));


    char *pathName = dirname(tmpPath);
    char *fileName = basename(tmpFile);

    LOG_DEBUG("directory of new File file >%s< Filename of new file >%s<", pathName, fileName);

    PathInfo_t *pathInfo = updatePathInfo(pathName);
    
    if (!pathInfo->jsonObjectRoot) goto ERROR;
    


    if (!pathInfo->jsonObjectRoot) {
        LOG_ERR("Havent found path to create file");
        goto ERROR;
    }

    if (!json_is_array(pathInfo->jsonObjectRoot)) {
        LOG_WARN("having single element no Array");
    }

    time_t now;
    time(&now);
    strftime(timestamp, sizeof timestamp, "%FT%TZ", gmtime(&now));

    json_t* newFileEntry = json_object();
    json_object_set_new(newFileEntry, "path", json_string(pathName));
    json_object_set_new(newFileEntry, "name", json_string(fileName));
    json_object_set_new(newFileEntry, "type", json_string("file"));
    json_object_set_new(newFileEntry, "size", json_integer(0));
    json_object_set_new(newFileEntry, "created", json_string(timestamp));
    json_object_set_new(newFileEntry, "modified", json_string(timestamp));
    json_object_set_new(newFileEntry, "state", json_string("new"));

    json_array_append(pathInfo->jsonObjectRoot ,newFileEntry);

    result = 0;
    goto EXIT;

ERROR:
    LOG_WARN("Leave create file in an error condition");
EXIT:
    json_decref(newFileEntry);
    FREE(tmpPath);
    FREE(tmpFile);
    LOG_DEBUG("new file entry\n %s\n",json_dumps(pathInfo->jsonObjectRoot ,JSON_PRESERVE_ORDER));

    return result;

}


/* *********************************************************************** */
/* *********************************************************************** */
int fsh_datacontainer_closeFile(const char *path) {
    int result = -1;

    UNIMPLEMENTED("need to implement HTTP transfer on close file"); 

    
    return fsh_openfilemanager_closeFile(path);
}


/* *********************************************************************** */
/* *********************************************************************** */
void fsh_datacontainer_container_destroy() {
  	LOG_DEBUG("calling destroy chain");
    if (loadedJsonPathInfo.jsonObjectRoot) {
        json_decref(loadedJsonPathInfo.jsonObjectRoot);
        loadedJsonPathInfo.jsonObjectRoot = NULL;
    }
    FREE(loadedJsonPathInfo.path);

    fsh_openfilemanager_closeAllFiles();

//    if(file.memory) FREE(file.memory->memory);
//    FREE(file.memory);

}
