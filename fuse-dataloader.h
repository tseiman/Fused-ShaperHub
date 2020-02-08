
#include <fuse.h>

#ifndef HAVE_FUSE_DATALOADER
#define HAVE_FUSE_DATALOADER

struct Oct_DirLoaderRef_s {
    const char *path;
    void *buf;
    fuse_fill_dir_t filler;
    const char *filename;
//    off_t offset;
//    struct fuse_file_info *fi;
};


int oct_dirLoader(struct Oct_DirLoaderRef_s *ref);


#endif