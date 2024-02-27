/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This file is part of the MountOctave Demonstrator Project
 *
 * This file contains the fuse Filesystem implementation
 * it gets it's information from the data layer
 * All fuse filesyste interaction is imoelemented here.
 *
 * License: Not Defined Yet
 *
 * Project URL: https://github.com/tseiman/MountOctave 
 *
 ************************************************************************** */


#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "fuse-octave.h"
#include "fuse-dataloader.h"


#ifdef IS_MOC
#include "MOCFILE.h"
MOC_DEMO_FILES;
#endif

static int getattr_callback(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));

    if(strcmp(path, "/") == 0) {
	stbuf->st_mode = S_IFDIR | 0755;
	stbuf->st_nlink = 1;
	stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = time(NULL);
	return 0;
    }


    if(!oct_statForPath(path,stbuf)) {
#ifdef MOCFILES_ATTR
	MOCFILES_ATTR;  // this brings in for some hardcoded "files" for mockup 
#endif
	return 0;
    } 



  return -ENOENT;
}

static int readdir_callback(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;
    int ret;
    struct Oct_DirLoaderRef_s dirLoaderRef;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);


    dirLoaderRef.path = path;
    dirLoaderRef.buf = buf;
    dirLoaderRef.filler = filler;

    ret = oct_dirLoader(&dirLoaderRef);
    if(ret == 0) return 0;


  return -ENOENT;

}

static int open_callback(const char *path, struct fuse_file_info *fi) {
  return 0;
}



static int read_callback(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {


//    return oct_FileLoader(path,buf,size,offset);



  if (strcmp(path, filepath) == 0) {
    size_t len = strlen(filecontent);
    if (offset >= len) {
      return 0;
    }

    if (offset + size > len) {
      memcpy(buf, filecontent + offset, len - offset);
      return len - offset;
    }

    memcpy(buf, filecontent + offset, size);
    return size;
  }



  if (strcmp(path, filepath1) == 0) {
    size_t len = strlen(filecontent1);
    if (offset >= len) {
      return 0;
    }

    if (offset + size > len) {
      memcpy(buf, filecontent1 + offset, len - offset);
      return len - offset;
    }

    memcpy(buf, filecontent1 + offset, size);
    return size;
  }



  return -ENOENT;


}


int readlink_callback(const char *path, char *buf, size_t size) {
    if(oct_LinkInfo(path, buf, size)) {
	return -ENOENT;
    }
    return 0;
}


static struct fuse_operations mount_octave_operations = {
  .getattr = getattr_callback,
  .open = open_callback,
  .read = read_callback,
  .readdir = readdir_callback,
  .readlink = readlink_callback,
};


int oct_initFuse(int argc, char *argv[]) {
  return fuse_main(argc, argv, &mount_octave_operations, NULL);
}


