/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This file is part of the MountOctave Demonstrator Project
 *
 * This file contains the header forh the fuse Filesystem implementation
 *
 * License: Not Defined Yet
 *
 * Project URL: https://github.com/tseiman/Fused-ShaperHub 
 *
 ************************************************************************** */

#include <fuse.h>
#include <string.h>
#include <errno.h>


typedef void (*showHelp_f)(const char *progname);


int fsh_initFuse(int argc, char *argv[],showHelp_f showHelp);


