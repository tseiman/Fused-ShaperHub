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
 * Project URL: https://github.com/tseiman/MountOctave 
 *
 ************************************************************************** */

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

#define __FILENAME__ (strrchr(__FILE__, PATH_SEPARATOR) ? strrchr(__FILE__, PATH_SEPARATOR) + 1 : __FILE__)

#define   ERR_LOG(format,...) fprintf (stderr, ANSI_COLOR_RED    "ERROR :%s:%d> " format ANSI_COLOR_RESET "\n", __FILENAME__,__LINE__ ,## __VA_ARGS__)
#define  WARN_LOG(format,...) fprintf (stdout, ANSI_COLOR_YELLOW "WARN  :%s:%d> "  format ANSI_COLOR_RESET "\n", __FILENAME__,__LINE__ ,## __VA_ARGS__)
#define  INFO_LOG(format,...) fprintf (stdout, ANSI_COLOR_BLUE   "INFO  :%s:%d> "  format ANSI_COLOR_RESET "\n", __FILENAME__,__LINE__ ,## __VA_ARGS__)
#define DEBUG_LOG(format,...) fprintf (stdout, ANSI_COLOR_GREEN  "DEBUG :%s:%d> " format ANSI_COLOR_RESET "\n", __FILENAME__,__LINE__ ,## __VA_ARGS__)


