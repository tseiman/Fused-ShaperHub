#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <string.h>
#include <errno.h>

#include "fuse-octave.h"
#include "fuse-dataloader.h"

static const char *filepath = "/file";
// static const char *filename = "file";
static const char *filecontent = "I'm the content of the only file available there\n";


static const char *filepath1 = "/sub/file1";
// static const char *filename1 = "file1";
static const char *filecontent1 = "I'm the content of the only file1 available there\n";

static const char *filepath2 = "/sub/file2";
// static const char *filename2 = "file2";
static const char *filecontent2 = "I'm the content of the only file2 available there\n";


static int getattr_callback(const char *path, struct stat *stbuf) {
  memset(stbuf, 0, sizeof(struct stat));

  if (strcmp(path, "/") == 0) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;
    stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = 1580428873;
    return 0;
  }

  if (strcmp(path, "/sub") == 0) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 3;
    stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = 1580428873;
    return 0;
  }


  if (strcmp(path, filepath) == 0) {
    stbuf->st_mode = S_IFREG | 0666;
    stbuf->st_nlink = 1;
    stbuf->st_size = strlen(filecontent);
    stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = 1580428873;
    return 0;
  }


  if (strcmp(path, filepath1) == 0) {
    stbuf->st_mode = S_IFREG | 0666;
    stbuf->st_nlink = 2;
    stbuf->st_size = strlen(filecontent1);
    stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = 1580428873;
    return 0;
  }

  if (strcmp(path, filepath2) == 0) {
    stbuf->st_mode = S_IFREG | 0666;
    stbuf->st_nlink = 3;
    stbuf->st_size = strlen(filecontent2);
    stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = 1580428873;
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
    if(ret == 0) {
	return ret;
    }
/*
  if (strcmp(path, "/") == 0) {
    filler(buf, filename, NULL, 0);
    filler(buf, "sub", NULL, 0);
    return 0;
  }

  if (strcmp(path, "/sub") == 0) {
    filler(buf, filename1, NULL, 0);
    filler(buf, filename2, NULL, 0);
    return 0;
  }

*/



  return -ENOENT;

}

static int open_callback(const char *path, struct fuse_file_info *fi) {
  return 0;
}



static int read_callback(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {

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

  if (strcmp(path, filepath2) == 0) {
    size_t len = strlen(filecontent2);
    if (offset >= len) {
      return 0;
    }

    if (offset + size > len) {
      memcpy(buf, filecontent2 + offset, len - offset);
      return len - offset;
    }

    memcpy(buf, filecontent2 + offset, size);
    return size;
  }



  return -ENOENT;
}


static struct fuse_operations mount_octave_operations = {
  .getattr = getattr_callback,
  .open = open_callback,
  .read = read_callback,
  .readdir = readdir_callback,
};


int oct_initFuse(int argc, char *argv[]) {
  return fuse_main(argc, argv, &mount_octave_operations, NULL);
}


