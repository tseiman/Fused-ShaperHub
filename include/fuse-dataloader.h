/* ***************************************************************************
 *
 * Thomas Schmidt, 2020-2026
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
 * Project URL: https://github.com/tseiman/Fused-ShaperHub
 *
 ************************************************************************** */

#include <fuse.h>

#ifndef HAVE_FUSE_DATALOADER
#define HAVE_FUSE_DATALOADER

struct Fsh_DirLoaderRef_s {
    const char *path;
    void *buf;
    fuse_fill_dir_t filler;
    const char *filename;
};


#define FSH_STAT_TYPE_FOLDER		0
#define FSH_STAT_TYPE_FILE		1
// #define FSH_STAT_TYPE_REFERENCE 	2


struct Fsh_ObjectStat_s {
    int type;
//    char *onlinePath;
    unsigned int atime;
    unsigned int filesize;
};


int fsh_fusedataloader_dirLoader(struct Fsh_DirLoaderRef_s *ref, int offset);
int fsh_fusedataloader_statForPath(const char *path, struct stat *stbuf);
int fsh_fusedataloader_linkInfo(const char *path, char * linkDstPath, size_t size);
int fsh_fusedataloader_fileOpener(const char *path);
int fsh_fusedataloader_fileReader(const char *path, char *buf, size_t size, off_t offset);
int fsh_fusedataloader_mknod(const char *path);
int fsh_fusedataloader_releaseFile(const char *path);
void fsh_fusedataloader_destroy(void);

#endif
