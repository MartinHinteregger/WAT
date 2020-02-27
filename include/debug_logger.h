/* ==================================================================
 * title:		debug_logger.h
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 * This is a simple, custom debug logger.
 * It includes functions for printing into a log file, printing to console or both.
 * ==================================================================
 */

#ifndef INCLUDE_DEBUG_LOGGER_H_
#define INCLUDE_DEBUG_LOGGER_H_

#include "globals.h"

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <mutex>

using namespace std;

bool debug_logger();
void writeHeader();
bool openLogFile(bool newFile = false);

// Functions to print a text to the console and the log file
void printConsoleAndDebugLine(const char* text, int addInt);
void printConsoleAndDebugLine(const char* text, float addFloat);
void printConsoleAndDebugLine(const char* text);

// Console log functions.
void printConsoleLine(const char* text, int addInt);
void printConsoleLine(const char* text, float addFloat);
void printConsoleLine(const char* text);

// Log File functions.
void printDebugLine(const char* text, int addInt);
void printDebugLine(const char* text, float addFloat);
bool printDebugLine(const char* text);

#endif /* INCLUDE_DEBUG_LOGGER_H_ */
