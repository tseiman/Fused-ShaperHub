
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



#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define MSG_HELP(command) \
            "\n"                                                              \
            "-h                         Help (this print basically)\n"        \
            "-k <pathAndKeyfile>        give path to keyfile\n"               \
            "-v <level 0-3>             verbosity levelv 0= only errors\n"    \
            "                                            1= Info       \n"    \
            "                                            2= Debug      \n"    \
            "                                            3= Memeory Debug\n"  \
            "\n"                                                              \
            "Basic Usage:\n"                                                  \
            "    %s -k key/googleKey-1234.json\n"                             \
            "\n", command


#define PRINT_MSG_HELP_AND_EXIT(command)       printf(MSG_HELP(command)); exit(1);


#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif

#define MEM_DBG(_fmt,...)         if(verbosity > 3) fprintf(stdout, ANSI_COLOR_MAGENTA  "[MEM_DBG] " _fmt ANSI_COLOR_RESET "\n", ##__VA_ARGS__  )
#define LOG_DEBUG(_fmt,...)       if(verbosity > 2) fprintf(stdout, ANSI_COLOR_GREEN    "[DEBUG]   " _fmt "\t(%s:%d)" ANSI_COLOR_RESET "\n", ##__VA_ARGS__  , __FILE_NAME__, __LINE__)
#define LOG_INFO(_fmt,...)        if(verbosity > 1) fprintf(stdout, ANSI_COLOR_BLUE     "[INFO]    " _fmt "\t(%s:%d)" ANSI_COLOR_RESET "\n", ##__VA_ARGS__  , __FILE_NAME__, __LINE__)
#define LOG_WARN(_fmt,...)        if(verbosity > 0) fprintf(stdout, ANSI_COLOR_YELLOW   "[WARN]    " _fmt "\t(%s:%d)" ANSI_COLOR_RESET "\n", ##__VA_ARGS__  , __FILE_NAME__, __LINE__)
#define LOG_ERR(_fmt,...)                           fprintf(stderr, ANSI_COLOR_RED      "[ERROR]   " _fmt "\t(%s:%d)" ANSI_COLOR_RESET "\n", ##__VA_ARGS__  , __FILE_NAME__, __LINE__)

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



#endif