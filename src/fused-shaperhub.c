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
 * Project URL: https://github.com/tseiman/Fused-ShaperHub
 *
 ************************************************************************** */

#define FUSE_USE_VERSION 26

#include <assert.h>
#include <errno.h>
#include <fuse.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <fuse-dataloader.h>
#include <fused-shaperhub.h>
#include <global.h>
#include <messages.h>

#ifdef IS_MOC
#include "MOCFILE.h"
MOC_DEMO_FILES;
#endif

static int getattr_callback(const char *path, struct stat *stbuf) {
    LOG_DEBUG("getattr_callback %s", path);
    memset(stbuf, 0, sizeof(struct stat));

    if (strncmp(path, "/", MAX_PATH_LEN) == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 1;
        stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = time(NULL);
        return 0;
    }

    if (fsh_fusedataloader_statForPath(path, stbuf)) {
        LOG_WARN("STAT fort Path failed");
        return -ENOENT;
    }

    return 0;
}

static int readdir_callback(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    LOG_DEBUG("readdir_callback %s", path);
    (void)offset;
    (void)fi;
    int ret;
    struct Fsh_DirLoaderRef_s dirLoaderRef;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    dirLoaderRef.path = path;
    dirLoaderRef.buf = buf;
    dirLoaderRef.filler = filler;

    ret = fsh_fusedataloader_dirLoader(&dirLoaderRef);
    if (ret == 0)
        return 0;

    return -ENOENT;
}

static int open_callback(const char *path, struct fuse_file_info *fi) { 
    (void)fi;
    fsh_fusedataloader_fileOpener(path);
    return 0; 
}

static int read_callback(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    LOG_DEBUG("Reading file callback %s", path);
    return fsh_fusedataloader_fileLoader(path,buf,size,offset);
}

int readlink_callback(const char *path, char *buf, size_t size) {
    LOG_INFO("readlink_callback %s", path);
    if (fsh_fusedataloader_linkInfo((const char *)path, buf, size)) {
        return -ENOENT;
    }
    return 0;
}

static struct fuse_operations mount_fsh_operations = {
    .getattr = getattr_callback,
    .open = open_callback,
    .read = read_callback,
    .readdir = readdir_callback,
    .readlink = readlink_callback,
};

extern int verbosity;
extern int logColor;

int fsh_shaperhub_initFuse(int argc, char *argv[], showHelp_f showHelp) {

    static struct options {
        int verbose;
        int logColor;
        int showHelp;
    } options;

#define OPTION(t, p)  { t, offsetof(struct options, p), 1 }

    static const struct fuse_opt option_spec[] = {
        OPTION("-v=%d", verbose), 
        OPTION("-h", showHelp), 
        OPTION("-c", logColor), 
        OPTION("--help", showHelp), 
        FUSE_OPT_END
    };

    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    /* Set defaults -- we have to use strdup so that
       fuse_opt_parse can free the defaults if other
       values are specified */
    options.verbose = 1;
    options.logColor = 0;
    /* Parse options */
    if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
        return 1;

    /* When --help is specified, first print our own file-system
       specific help text, then signal fuse_main to show
       additional help (by adding `--help` to the options again)
       without usage: line (by setting argv[0] to the empty
       string) */
    if (options.showHelp) {
        showHelp(argv[0]);
        assert(fuse_opt_add_arg(&args, "--help") == 0);
        args.argv[0][0] = '\0';
    }


    verbosity = options.verbose;
    logColor = options.logColor;
    ret = fuse_main(args.argc, args.argv, &mount_fsh_operations, NULL);
    fuse_opt_free_args(&args);
}

void fsh_shaperhub_destroy() {
   	LOG_DEBUG("calling destroy chain");
    fsh_fusedataloader_destroy();
}