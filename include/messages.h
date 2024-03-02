
/** ***************************************************************************
 *  ***************************************************************************
 *
 * messages.h is part of the project: FILLME 
 * Project Page: https://github.com/tseiman/
 * Author: Thomas Schmidt
 * Copyright (c) 2024 
 *
 * Description:
 *
 * Macros for console messages mainly, including filtering on debug level
 * CLI parameter
 *
 * ****************************************************************************
 * **************************************************************************** **/


#ifndef printf
#include <stdio.h>
#endif


#ifndef MESSAGES_H
#define MESSAGES_H

extern int verbosity;
extern int logColor;


#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define COND_COLOR(color) (logColor ? color : "")

#define MSG_HELP(command) \
	        "File-system specific options:\n"                                  \
	        "    -v=<n>              0-4 verbosity levelv 0= only errors\n"    \
            "                                             1= Warning    \n"    \
            "                                             2= Info       \n"    \
            "                                             3= Debug      \n"    \
            "                                             4= Memory Debug\n"   \
            "\n"                                                               \
	        "    -c                  Log Color\n"                              \
            "\n"                                                               \
            "Basic Usage:\n"                                                   \
            "    %s -f testdirectory\n"                                        \
            "\n", command


#define PRINT_MSG_HELP(command)       printf(MSG_HELP(command));


#ifndef __FUNCTION_NAME__
    #ifdef WIN32   //WINDOWS
        #define __FUNCTION_NAME__   __FUNCTION__  
    #else          //*NIX
        #define __FUNCTION_NAME__   __func__ 
    #endif
#endif


#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif

#define MEM_DBG(_fmt,...)         if(verbosity > 3) fprintf(stdout, "%s[MEM_DBG] "   _fmt "%s\n", COND_COLOR(ANSI_COLOR_MAGENTA),  ##__VA_ARGS__  , COND_COLOR(ANSI_COLOR_RESET))
#define LOG_DEBUG(_fmt,...)       if(verbosity > 2) fprintf(stdout, "%s[DEBUG]   %s():\t"   _fmt "\t(%s:%d)%s\n", COND_COLOR(ANSI_COLOR_GREEN), __FUNCTION_NAME__,    ##__VA_ARGS__  , __FILE_NAME__, __LINE__, COND_COLOR(ANSI_COLOR_RESET))
#define LOG_INFO(_fmt,...)        if(verbosity > 1) fprintf(stdout, "%s[INFO]    %s():\t"   _fmt "\t(%s:%d)%s\n", COND_COLOR(ANSI_COLOR_BLUE), __FUNCTION_NAME__,     ##__VA_ARGS__  , __FILE_NAME__, __LINE__, COND_COLOR(ANSI_COLOR_RESET))
#define LOG_WARN(_fmt,...)        if(verbosity > 0) fprintf(stdout, "%s[WARN]    %s():\t"   _fmt "\t(%s:%d)%s\n", COND_COLOR(ANSI_COLOR_YELLOW), __FUNCTION_NAME__,   ##__VA_ARGS__  , __FILE_NAME__, __LINE__, COND_COLOR(ANSI_COLOR_RESET))
#define LOG_ERR(_fmt,...)                           fprintf(stderr, "%s[ERROR]   %s():\t"   _fmt "\t(%s:%d)%s\n", COND_COLOR(ANSI_COLOR_RED), __FUNCTION_NAME__,      ##__VA_ARGS__  , __FILE_NAME__, __LINE__, COND_COLOR(ANSI_COLOR_RESET))

#define LOG_MEM_ERR(_fmt,...)         fprintf(stderr, "[ERROR]   " _fmt "\n", ##__VA_ARGS__ )


#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
/*
#define LOG_INFO_MSG_WITH_OK(_fmt,...)        if(verbosity > 0) fprintf(stdout, "[INFO]    " _fmt "... %s", ##__VA_ARGS__, (verbosity > 1 ? "\t("__FILE_NAME__":" LINE_STRING ")\n" : ""));
#define LOG_INFO_OK()                         if(verbosity > 0) fprintf(stdout, "OK%s\n",  (verbosity > 1 ? "\t("__FILE_NAME__":" LINE_STRING ")\n" : ""))
#define LOG_INFO_FAIL()                       if(verbosity > 0) fprintf(stdout, "FAIL%s\n",(verbosity > 1 ? "\t("__FILE_NAME__":" LINE_STRING ")\n" : ""))
*/
#define DBG_PRINT() fprintf(stdout, "[DEBUG PRINT]  %s:%d)\n", __FILE_NAME__, __LINE__)

#define UNIMPLEMENTED() fprintf(stdout, "%s ------------ UNIMPLEMENTED ------------ %s() in %s:%d)%s\n", COND_COLOR(ANSI_COLOR_CYAN), __FUNCTION_NAME__,__FILE_NAME__, __LINE__, COND_COLOR(ANSI_COLOR_RESET))

#endif