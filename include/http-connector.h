/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This file is part of the MountOctave Demonstrator Project
 *
 * header file for the HTTPS interface to the Octave cloud API service
 *
 * License: Not Defined Yet
 *
 * Project URL: https://github.com/tseiman/Fused-ShaperHub
 *
 ************************************************************************** */


#ifndef HAVE_HTTP_CONNECTOR_H
#define HAVE_HTTP_CONNECTOR_H

typedef struct  {
  char *memory;
  size_t size;
} MemoryStruct_t;


int fsh_HTTPListPath(char *path, MemoryStruct_t **responseBuffer);

#endif