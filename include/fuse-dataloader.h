/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This file is part of the MountOctave Demonstrator Project
 *
 * The hider file for the wrapper and helper (intermidieate layer) between
 * local data layer/storage and the fuse Filesystem. This implementation 
 * accesses the data layer and transalte it to fuse filesystem structures
 * if a mapping needs to be done
 *
 * License: Not Defined Yet
 *
 * Project URL: https://github.com/tseiman/MountOctave 
 *
 ************************************************************************** */

#include <fuse.h>

#ifndef HAVE_FUSE_DATALOADER
#define HAVE_FUSE_DATALOADER

struct Oct_DirLoaderRef_s {
    const char *path;
    void *buf;
    fuse_fill_dir_t filler;
    const char *filename;
};


#define OCT_STAT_TYPE_FOLDER		0
#define OCT_STAT_TYPE_FILE		1
#define OCT_STAT_TYPE_REFERENCE 	2


struct Oct_ObjectStat_s {
    int type;
    char *onlinePath;
    unsigned int atime;
    unsigned int filesize;
};


int oct_dirLoader(struct Oct_DirLoaderRef_s *ref);
int oct_statForPath(const char *path, struct stat *stbuf);
int oct_LinkInfo(const char *path, char * linkDstPath, size_t size);
int oct_FileLoader(const char *path, char *buf, size_t size, off_t offset);


#endif
