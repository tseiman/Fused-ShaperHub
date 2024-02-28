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

#include <fused-shaperhub.h>
#include <messages.h>
#include <http-connector.h>
#include <data-container.h>


int verbosity = 0;


static void show_help(const char *progname) {
	printf("usage: %s [options] <mountpoint>\n\n", progname);
	printf("File-system specific options:\n"
	       "    -v=<n>              0-4 Debug Level\n"
	       "\n");
}



int main(int argc, char **argv) {

    fsh_initFuse(argc, argv, &show_help);


    return 0;
}

