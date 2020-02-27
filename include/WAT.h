/* ==================================================================
 * title:		WAT.h
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 * ==================================================================
 */

#ifndef INCLUDE_WAT_H_
#define INCLUDE_WAT_H_

#include "randomStream.h"
#include "stream.h"
#include "commands.h"
#include "debug_logger.h"
#include "globals.h"
#include "LimeSuite.h"

#include <iostream>
#include <vector>
#include <wait.h>
#include <thread>

using namespace std;

// Commands ENUM. Make sure to have the same length+1 as commands strings, as well as the same order.
enum eCmd {
	ANTENNA = 0,
	CALIBRATE = 1,
	CONNECT = 2,
	CONSTELLATION = 3,
	DEVICES = 4,
	DISCONNECT = 5,
	ENABLE = 6,
	EXIT = 7,
	GAIN = 8,
	HELP = 9,
	INIT = 10,
	LO = 11,
	LOAD = 12,
	LPBW = 13,
	QUIT = 14,
	RESET = 15,
	SAMPLE = 16,
	SAVE = 17,
	STREAM = 18,
	WFMPLAYER = 19,
	NUMBEROFCOMMANDS = 20 // Has to be the last entry
};

// Commands strings. Make sure to have the same length-1 as commands ENUM, as well as the same order.
static const char *commands[NUMBEROFCOMMANDS] {
	"antenna",
	"calibrate",
	"connect",
	"constellation",
	"devices",
	"disconnect",
	"enable",
	"exit",
	"gain",
	"help",
	"init",
	"lo",
	"load",
	"lpbw",
	"quit",
	"reset",
	"sample",
	"save",
	"stream",
	"wfm"
};

bool checkIfOpenedStillConnected(const deviceVector& deviceVec, int nConnected);

#endif /* INCLUDE_WAT_H_ */
