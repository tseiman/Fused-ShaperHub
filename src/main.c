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



    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGKILL, sig_handler);
    

    fsh_shaperhub_initFuse(argc, argv, &show_help);


    return 0;
}


void sig_handler(int sig) {
    int toFree = 0;
    LOG_INFO("Received SIGTERM - cleaning up and exiting.");
    fsh_shaperhub_destroy();
    if(toFree = getAlloCounter()) {
        LOG_WARN("Not all memory have been freed - still left: %d", toFree);
    }
    exit(0);
}
