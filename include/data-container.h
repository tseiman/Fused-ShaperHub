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

typedef int (*WalkFolders_Callback_t)(struct Fsh_DirLoaderRef_s *ref);


typedef struct {
    json_t *jsonObjectRoot;
    char   *path;
} PathInfo_t; 

// int fsh_setupDataStructure(void);
int fsh_walkFolders(WalkFolders_Callback_t callback, struct Fsh_DirLoaderRef_s *ref);
int fsh_getInfo(const char *path,struct Fsh_ObjectStat_s *file_info);
int fsh_getLinkInfo(const char *path, char * linkDstPath, size_t size);

