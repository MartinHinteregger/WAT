/* ==================================================================
 * title:		randomStream.h
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 * These functions should transmit a random symbol stream of 9 symbols.
 * However, it never worked well. Only one device was available, so since
 * the device class was not thread safe by then, lot's of problems occurred.
 * Also, the function startRandomStream creates two threads, which was not
 * necessary at all.
 * One thing was positive: writing this code teached me a lot of things.
 * ==================================================================
 */

#ifndef INCLUDE_RANDOMSTREAM_H_
#define INCLUDE_RANDOMSTREAM_H_

#include "globals.h"
#include "Device.h"
#ifdef USE_GNU_PLOT
#include "gnuPlotPipe.h"
#endif

#include <pthread.h>
#include <limits.h>
#include <time.h>
#include "sys/stat.h"
#include "sys/types.h"
#include <fstream>

using namespace std;

typedef struct arguments_
{
	Device *dev_;
} arguments;

bool startRandomStream(deviceVector& deviceVec, int destID, int sourceID);

void *streamRandomRX(void *args_);
void *streamRandomTX(void *args_);

string createTestFilename(const char* dir, const char *constellation);

#endif /* INCLUDE_RANDOMSTREAM_H_ */
