/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This file is part of the MountOctave Demonstrator Project
 *
 * Entry point/main routine for the Fuse Filesystem application.
 * it has the main control over the different functions
 * (Fuse Filesystem, Data Layer, HTTPS Octave API Conector)
 * and the lifecycle of the applicaiton.
 *
 * License: Not Defined Yet
 *
 * Project URL: https://github.com/tseiman/Fused-ShaperHub
 *
 ************************************************************************** */


#include <fuse.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include <fused-shaperhub.h>
#include <alloc.h>
#include <messages.h>


void sig_handler(int);

int verbosity = 0;
int logColor = 0;

static void show_help(const char *progname) {
    PRINT_MSG_HELP(progname);
}



int main(int argc, char **argv) {
    fsh_shaperhub_initFuse(argc, argv, &show_help);
    return 0;
}
