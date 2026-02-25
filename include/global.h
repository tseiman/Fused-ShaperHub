/* ***************************************************************************
 *
 * Thomas Schmidt, 2020
 *
 * This file is part of the MountOctave Demonstrator Project
 *
 * Some global macros and definitions go here
 *
 * License: Not Defined Yet
 *
 * Project URL: https://github.com/tseiman/Fused-ShaperHub
 *
 ************************************************************************** */
#include <stdio.h>
#include <pthread.h>

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

#define MAX_PATH_LEN 1024

#ifndef TRUE
#define TRUE  (1==1)
#endif

#ifndef FALSE
#define FALSE (!TRUE)
#endif

#include <limits.h>

#ifdef __APPLE__
  #include <sys/syslimits.h>
#endif

#ifndef PATH_MAX
  #define PATH_MAX 4096
#endif

#ifndef MAX_PATH_LEN
  #define MAX_PATH_LEN PATH_MAX
#endif

/* NOTE: g_model_lock removed.
 * The old 'static pthread_mutex_t g_model_lock' here was broken: 'static'
 * in a header gives every .c file its own private copy, so threads were
 * locking different mutex instances and not protecting each other at all.
 *
 * Locking is now handled per-subsystem:
 *   - JSON path cache: thread-local storage (pthread_key_t) in data-container.c
 *   - Open file list:  internal ofm_lock in open-file-manager.c              */
