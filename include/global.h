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
  #include <sys/syslimits.h>   // PATH_MAX auf macOS zuverlässig
#endif

#ifndef PATH_MAX
  #define PATH_MAX 4096        // Fallback (sicherer Default)
#endif

// Dein Projekt-Makro:
#ifndef MAX_PATH_LEN
  #define MAX_PATH_LEN PATH_MAX
#endif

static pthread_mutex_t g_model_lock = PTHREAD_MUTEX_INITIALIZER;




