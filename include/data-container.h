/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This file is part of the MountOctave Demonstrator Project
 *
 * The file is the header for the implementation for local data layer and 
 * storage for simplicity (and because it was already in use) the data
 * managment is implemented with JSON data.
 * 
 * License: Not Defined Yet
 *
 * Project URL: https://github.com/tseiman/Fused-ShaperHub
 *
 ************************************************************************** */


#include <jansson.h>


#include <fuse-dataloader.h>
#include <http-connector.h>

#ifndef DATA_CONTAINER_H
#define DATA_CONTAINER_H

#define BLOB_ID_LEN 128

typedef int (*WalkFolders_Callback_t)(struct Fsh_DirLoaderRef_s *ref);

typedef struct {
    json_t *jsonObjectRoot;
    char   *path;
} PathInfo_t; 


typedef struct  {
    char blobID[BLOB_ID_LEN + 1];
    MemoryStruct_t *memory;
} FileMemoryStruct_t;


// int fsh_setupDataStructure(void);
int fsh_datacontainer_loadDir(WalkFolders_Callback_t callback, struct Fsh_DirLoaderRef_s *ref);
int fsh_datacontainer_getInfo(const char *path,struct Fsh_ObjectStat_s *file_info);
int fsh_datacontainer_openFile(const char *newPath);
FileMemoryStruct_t *fsh_datacontainer_readFile(const char *newPath);
int fsh_datacontainer_createFile(const char *newFile);
int fsh_datacontainer_getLinkInfo(const char *path, char * linkDstPath, size_t size);
int fsh_datacontainer_closeFile(const char *path);
void fsh_datacontainer_container_destroy(void);

#endif