/* ==================================================================
 * title:		globals.h
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 * Global variables for the program. Not pretty, but makes things easier.
 * ==================================================================
 */

#ifndef INCLUDE_GLOBALS_H_
#define INCLUDE_GLOBALS_H_

#include <Device.h>
#include "LimeSuite.h"

#include <vector>

#define swVersion 0.3
// For now we will set a hard coded number of channels.
#define globalNumChannels 2
#define maxTxResolutionI16 (32768-1)

class Device;

// This vector will hold all opened devices. Will be passed to functions.
typedef std::vector<Device*> deviceVector;

// Declare this new for our code.
struct complex16_t
{
    int16_t i;
    int16_t q;
};

#endif /* INCLUDE_GLOBALS_H_ */
