#include <string.h>
#include <stdio.h>

#include "data-container.h"
#include "fuse-dataloader.h"

int oct_dirLoaderCallback(struct Oct_DirLoaderRef_s *ref) {
//    printf("CALLED \n");


    ref->filler(ref->buf, ref->filename, NULL, 0);

    return 0;
}

int oct_dirLoader(struct Oct_DirLoaderRef_s *ref) {



    if (strcmp(ref->path, "/") == 0) {
	oct_walkFolders(oct_dirLoaderCallback, ref);
	return 0;
    }
/*
    dirLoaderRef.path = path;
    dirLoaderRef.buf = buf;
    dirLoaderRef.filler = filler;
    

  if (strcmp(path, "/") == 0) {
    filler(buf, filename, NULL, 0);
    filler(buf, "sub", NULL, 0);
    return 0;
  }
*/
    return 0;
}

