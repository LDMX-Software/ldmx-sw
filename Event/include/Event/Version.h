/**
 * @file Version.h.in
 * @brief Configure file used by CMake to generate a header file that defines
 *        software related constants e.g. git SHA-1. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef _EVENT_VERSION_H_
#define _EVENT_VERSION_H_

namespace ldmx { 

//
// The following variables will automatically be replaced by the corresponding
// CMake variables.
// 

/**
 * The git commit sha for this installation of ldmx-sw
 */
#define GIT_SHA1 "9c8aec605901a20d699f89f45afb0ebbb0eb1a47"

}

#endif // _EVENT_VERSION_H_
