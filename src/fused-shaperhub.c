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

#include <alloc.h>
#include <fuse-dataloader.h>
#include <fused-shaperhub.h>
#include <global.h>
#include <messages.h>





static void *init_callback(struct fuse_conn_info *conn) {
    LOG_DEBUG("init_callback");
    UNIMPLEMENTED("init callback not implemented yet");
    return NULL;
}


static int mkdir_callback(const char *path, mode_t mode) {
    LOG_DEBUG("mkdir_callback");
    UNIMPLEMENTED("mkdir callback not implemented yet");
    return 0;
}

static int rmdir_callback(const char *path) { 
    LOG_DEBUG("rmdir_callback");
    UNIMPLEMENTED("mkdir callback not implemented yet");
    return 0;  
}

static int rename_callback(const char *from, const char *to) {
    LOG_DEBUG("rename_callback");
    UNIMPLEMENTED("rename callback not implemented yet");
    return 0;  
}


static int getattr_callback(const char *path, struct stat *stbuf) {
    LOG_DEBUG("getattr_callback %s", path);

    pthread_mutex_lock(&g_model_lock);

    memset(stbuf, 0, sizeof(struct stat));

    if (strncmp(path, "/", MAX_PATH_LEN) == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 1;
        stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = time(NULL);
        pthread_mutex_unlock(&g_model_lock);
        return 0;
    }

    if (fsh_fusedataloader_statForPath(path, stbuf)) {
        LOG_WARN("STAT fort Path failed");
        pthread_mutex_unlock(&g_model_lock);
        return -ENOENT;
    }

    pthread_mutex_unlock(&g_model_lock);
    return 0;
}


static int setattr_callback(const char * path, const char *name, const char *value, size_t size,  int flags) { 
    
    LOG_DEBUG("setattr_callback path:%s, name:%s, value:%s, size:%zu, flags: %d",path,name,value , size, flags);
    UNIMPLEMENTED("setattr callback not implemented yet");
    return 0;
}



static int readdir_callback(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    LOG_DEBUG("readdir_callback %s", path);
 //   (void)offset;
    pthread_mutex_lock(&g_model_lock);
    (void)fi;
    int ret;
    struct Fsh_DirLoaderRef_s dirLoaderRef;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    dirLoaderRef.path = path;
    dirLoaderRef.buf = buf;
    dirLoaderRef.filler = filler;

    ret = fsh_fusedataloader_dirLoader(&dirLoaderRef, offset);
    if (ret == 0) {
    pthread_mutex_unlock(&g_model_lock);
        return 0;
    }

    pthread_mutex_unlock(&g_model_lock);
    return -ENOENT;
}

static int create_callback(const char *path, mode_t mode, struct fuse_file_info *fi) {
    LOG_DEBUG("create_callback");
    UNIMPLEMENTED("create callback not implemented yet");
    return 0;
}

static int mknod_callback(const char *path, mode_t mode, dev_t rdev) {
    LOG_DEBUG("mknod_callback");

    pthread_mutex_lock(&g_model_lock);
    int res = fsh_fusedataloader_mknod(path);
    pthread_mutex_unlock(&g_model_lock);

    return res;
}




static int open_callback(const char *path, struct fuse_file_info *fi) { 
    (void)fi;
    pthread_mutex_lock(&g_model_lock);
    fsh_fusedataloader_fileOpener(path);
    pthread_mutex_unlock(&g_model_lock);    
    return 0; 
}

static int read_callback(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    LOG_DEBUG("Reading file callback %s", path);
//    fsh_fusedataloader_fileOpener(path);
    pthread_mutex_lock(&g_model_lock);
    int res = fsh_fusedataloader_fileReader(path,buf,size,offset);
    pthread_mutex_unlock(&g_model_lock);
    return res;
}

static int write_callback(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) { 
    LOG_DEBUG("write_callback");
    UNIMPLEMENTED("write callback not implemented yet");
    return size;  
}
static int write_buf_callback(const char *path, struct fuse_bufvec *buf, off_t offset, struct fuse_file_info *fi) {
    LOG_DEBUG("write_buf_callback");
    UNIMPLEMENTED("write_buff callback not implemented yet");
    return 0; 
}

static int truncate_callback(const char *path, off_t offset) { 
    LOG_DEBUG("truncate_callback");
    UNIMPLEMENTED("truncate callback not implemented yet");;
    return 0; 
}



static int release_callback(const char *path, struct fuse_file_info *fi) { 
    LOG_INFO("release_callback");

    pthread_mutex_lock(&g_model_lock);
    int res = fsh_fusedataloader_releaseFile(path);  
    pthread_mutex_unlock(&g_model_lock);

    return res;
}

static int flush_callback(const char *path, struct fuse_file_info *fi) { 
    LOG_DEBUG("flush_callback");
    UNIMPLEMENTED("flush callback not implemented yet");
    return 0;  
}
static int fsync_callback(const char *path, int datasync, struct fuse_file_info *fi) {
    LOG_DEBUG("fsync_callback");
    UNIMPLEMENTED("fsync callback not implemented yet");
    return 0;  
}


static int chmod_callback(const char *path, mode_t mode) { return 0; } /* We do not do anything here - seems some editors like to fo chmod without "operation not permitted" */
static int chown_callback(const char *path, uid_t uid, gid_t gid) {
    LOG_DEBUG("chown_callback");
    UNIMPLEMENTED("chown callback not implemented yet");
    return 0;  
}

static int utime_callback(const char *path, struct utimbuf * time) {
    LOG_DEBUG("utime_callback");
    UNIMPLEMENTED("utime callback not implemented yet");
    return 0;  
}

static void destroy_callback(void *priv_data) { 
    LOG_INFO("Received SIGTERM - cleaning up and exiting.");
    LOG_DEBUG("calling destroy chain");
    pthread_mutex_lock(&g_model_lock);
    fsh_fusedataloader_destroy();
    int toFree;
    if((toFree = getAllocCounter())) {
        LOG_ERR("!!!!!!!!!! Not all memory have been freed - still left: %d !!!!!!!!!!!", toFree);
    }
    pthread_mutex_unlock(&g_model_lock);
}



static struct fuse_operations mount_fsh_operations = {
  //  .init       = init_callback,
    .mkdir      = mkdir_callback,
 //   .rmdir      = rmdir_callback,
  //  .rename     = rename_callback,
    .getattr    = getattr_callback,
    .readdir    = readdir_callback,
        .mknod    = mknod_callback,
 //   .create     = create_callback,
    .open       = open_callback,
    .read       = read_callback,
    .write      = write_callback,
//    .write_buf  = write_buf_callback,
    .release    = release_callback,
 //   .flush      = flush_callback,
 //   .fsync      = fsync_callback,
    .destroy    = destroy_callback,
  .chmod = chmod_callback,
//  .chown = chown_callback,
//  .utime = utime_callback, 
//  .truncate = truncate_callback, // << this would be needed for online editing
//  .setxattr = setattr_callback,
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

    int ret = 0;
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
    return ret;
}
