#include <fuse.h>
#include <string.h>
#include <errno.h>

// #ifdef HAVE_OCTAVE_FUSE
// #define HAVE_OCTAVE_FUSE 1


/*
static int getattr_callback(const char *path, struct stat *stbuf);
static int readdir_callback(const char *path, void *buf, fuse_fill_dir_t filler, struct fuse_file_info *fi);
static int open_callback(const char *path, struct fuse_file_info *fi);
static int read_callback(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
*/

int oct_initFuse(int argc, char *argv[]);



// #endif
