/* ==================================================================
 * title:		commands.h
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 * This file contains all the available commands used by the tool in the main routine.
 * Moved here to not blow up the main file.
 * D&C: this file uses no API Calls.
 * Attention: There is not much variable checking going on here.
 * Make sure that values are valid.
 * ==================================================================
 */

#ifndef INCLUDE_COMMANDS_H_
#define INCLUDE_COMMANDS_H_

#include "Device.h"
#include "globals.h"
#include "WAT.h"
#include "LimeSuite.h"

#include <vector>
#include <unistd.h>

using namespace std;

class Device;

bool calibrate(deviceVector& deviceVec, int devID, float_type bandwidth, const char *dir, int channel);
int connect(deviceVector& deviceVec, int nConnected);
bool constellation(deviceVector& deviceVec, int devID, const char *constel);
void disconnect(deviceVector& deviceVec);
bool enable(deviceVector& deviceVec, int devID, bool en, const char *dir, int channel);
void init(deviceVector& deviceVec, int devID);
int playWaveform(deviceVector& deviceVec, int devID, const char *filename);
void printConnectedDevices(int nConnected);
void printHelp();
void printOpenedDevices(deviceVector& deviceVec);
void reset(deviceVector& deviceVec, int devID);

// Load/Save Configuration
bool loadConfiguration(deviceVector& deviceVec, int devID, const char *filename);
bool saveConfiguration(deviceVector& deviceVec, int devID, const char *filename);

// Parameter getter / setter
bool getAntennaPorts(deviceVector& deviceVec, int devID, int channel);
bool getGain(deviceVector& deviceVec, int devID, int channel);
bool getLOFreq(deviceVector& deviceVec, int devID, int channel);
bool getLPBW(deviceVector& deviceVec, int devID, int channel);
bool getSamplingRate(deviceVector& deviceVec, int devID, int channel);
bool setAntennaPorts(deviceVector& deviceVec, int devID, const char *dir, int channel);
bool setGain(deviceVector& deviceVec, int devID, const char *dir, int channel);
bool setLOFreq(deviceVector& deviceVec, int devID, const char *dir, int channel);
bool setLPBW(deviceVector& deviceVec, int devID, const char *dir, int channel);
bool setSamplingRate(deviceVector& deviceVec, int devID, const char *dir);

#endif /* INCLUDE_COMMANDS_H_ */
