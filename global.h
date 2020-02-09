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


// #define ERR_LOG(format,...) fprintf (stderr, "ERROR:%s:%d> " format "\n", __FILE__,__LINE__ ,## __VA_ARGS__)
#define ERR_LOG(format, ...) fprintf (stderr, format, ## __VA_ARGS__)


