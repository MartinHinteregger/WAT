/* ==================================================================
 * title:		Constellation.h
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 *
 * ==================================================================
 */

#ifndef INCLUDE_CONSTELLATION_CONSTELLATION_H_
#define INCLUDE_CONSTELLATION_CONSTELLATION_H_

#include "debug_logger.h"
#include "globals.h"

#include <iostream>

using namespace std;

// Constellation identifiers
#define bpskID 1
#define qpskID 2

class Bpsk;
struct complex16_t;

class Constellation
{
public:
	Constellation();
	virtual ~Constellation();

	virtual complex16_t modulateSingleSymbol(int8_t toMod) = 0;
	virtual int8_t demodulateSingleSymbol(int16_t i_, int16_t q_) = 0;
	virtual char demodulateToChar(int16_t toDemod[], int len) = 0;

	virtual int getConstellationID() = 0;
	virtual const char *getConstellationName() = 0;
	virtual int getNumBits() = 0;
	virtual int getSymbolsBits() = 0;
	virtual int getBitmask() = 0;

private:
	int constellationID;
	const char *constellationName;
	int numBits;
	int numSymbols;
	// TODO is this member still necessary?
	int bitmask;
};



#endif /* INCLUDE_CONSTELLATION_CONSTELLATION_H_ */
