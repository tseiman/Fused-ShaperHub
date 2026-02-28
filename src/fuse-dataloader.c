/* ***************************************************************************
 *
 * Thomas Schmidt, 2020-2026
 *
 * This file is part of the MountOctave Demonstrator Project
 *
 * The file contains a wrapper and helper (intermidieate layer) between local 
 * data layer/storage and the fuse Filesystem. This implementation 
 * accesses the data layer and transalte it to fuse filesystem structures
 * if a mapping needs to be done
 *
 * License: Not Defined Yet
 *
 * Project URL: https://github.com/tseiman/Fused-ShaperHub 
 *
 ************************************************************************** */


#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>


#include <data-container.h>
#include <fuse-dataloader.h>
#include <messages.h>


int fsh_dirLoaderCallback(struct Fsh_DirLoaderRef_s *ref) {
    ref->filler(ref->buf, ref->filename, NULL, 0);
    return 0;
}


int fsh_fusedataloader_dirLoader(struct Fsh_DirLoaderRef_s *ref, int offset) {
    fsh_datacontainer_loadDir(fsh_dirLoaderCallback, ref, offset);
	return 0;
}


int fsh_fusedataloader_fileOpener(const char *path) {
	return fsh_datacontainer_openFile(path);
}

int fsh_fusedataloader_fileReader(const char *path, char *buf, size_t size, off_t offset) {
    LOG_DEBUG("READ data chunk with size %d at offet %lld file: >%s<", (int)size, offset, path);

	FileMemoryStruct_t *file = fsh_datacontainer_readFile(path);

	if(!file) return -1;
	if(!file->memory) return -1;

	if (offset >= file->memory->size) {
    	    return 0;
	}
	if (offset + size > file->memory->size) {
    	    memcpy(buf, file->memory->memory + offset, file->memory->size - offset);
    	    return file->memory->size - offset;
	}
	memcpy(buf, file->memory->memory + offset, size);
	return size;

}



int fsh_fusedataloader_statForPath(const char *path, struct stat *stbuf) {
    struct Fsh_ObjectStat_s file_info;

	LOG_DEBUG("working on >%s<", path);



	stbuf->st_uid = getuid(); // The owner of the file/directory is the user who mounted the filesystem
	stbuf->st_gid = getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
	stbuf->st_atime = time( NULL ); // The last "a"ccess of the file/directory is right now
	stbuf->st_mtime = time( NULL ); // The last "m"odification of the file/directory is right now
	


    if(fsh_datacontainer_getInfo(path, &file_info)) {
        LOG_WARN("fsh_datacontainer_getInfo() returned an error condition: %s",path);
		return -1;
    }


	LOG_DEBUG("fsh_fusedataloader_statForPath( %s)", path);
    switch(file_info.type) {
	case FSH_STAT_TYPE_FOLDER:
	    LOG_DEBUG("FOLDER with name added: %s",path);
	    stbuf->st_mode = S_IFDIR | 0775;
	    stbuf->st_nlink = 2;
	    stbuf->st_size = 4096;
	    stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = file_info.atime;
	    break;
	case FSH_STAT_TYPE_FILE:
	    LOG_DEBUG("FILE with name added: %s",path);
	    stbuf->st_mode = S_IFREG | 0664;
	    stbuf->st_nlink = 1;
	    stbuf->st_size = file_info.filesize;
	    stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = file_info.atime;
	    break;
/*	case FSH_STAT_TYPE_REFERENCE:
	    LOG_DEBUG("LINK with name addded: %s",path);
	    stbuf->st_mode = S_IFLNK | 0777;
	    stbuf->st_nlink = 1;
	    stbuf->st_size = 4096;
	    stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = file_info.atime;
	    break; */
	default: return -1;
    }

    return 0;
}

int fsh_fusedataloader_releaseFile(const char *path) {
	return fsh_datacontainer_closeFile(path);
}

int fsh_fusedataloader_mknod(const char *path) {
	return fsh_datacontainer_createFile(path);
}

void fsh_fusedataloader_destroy() {
	LOG_DEBUG("calling destroy chain");
	fsh_datacontainer_container_destroy();
}