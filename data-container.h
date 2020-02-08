
#include "fuse-dataloader.h"

typedef int (*WalkFolders_Callback_t)(struct Oct_DirLoaderRef_s *ref);




int oct_setupDataStructure(void);
// int oct_walkFolders(void *callback, void *ref);
int oct_walkFolders(WalkFolders_Callback_t callback, struct Oct_DirLoaderRef_s *ref);
