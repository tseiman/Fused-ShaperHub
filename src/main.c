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
 * Project URL: https://github.com/tseiman/MountOctave 
 *
 ************************************************************************** */


#include <fuse.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


#include "fuse-octave.h"
#include "http-connector.h"
#include "data-container.h"


int main(int argc, char *argv[]) {
//    char *httpdata = NULL;
//    getLocalAction("l5e336e4950f068603b5e6e2f.json", &httpdata);
//    printf("My HTTP data:\n%s\n",httpdata);
//    free(httpdata);

    oct_setupDataStructure();
    oct_initFuse(argc, argv);


    return 0;
}

