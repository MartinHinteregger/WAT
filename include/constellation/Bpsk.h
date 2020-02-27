/* ==================================================================
 * title:		Bpsk.h
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 *
 * ==================================================================
 */

#ifndef INCLUDE_CONSTELLATION_BPSK_H_
#define INCLUDE_CONSTELLATION_BPSK_H_

#include "Constellation.h"

using namespace std;

class Bpsk : public Constellation
{
public:
	Bpsk();
	~Bpsk();

	complex16_t modulateSingleSymbol(int8_t toMod);
	int8_t demodulateSingleSymbol(int16_t i_, int16_t q_);
	char demodulateToChar(int16_t toDemod[], int len);

	int getConstellationID();
	const char *getConstellationName();
	int getNumBits();
	int getSymbolsBits();
	int getBitmask();

private:
	int constellationID;
	const char *constellationName;
	int numBits;
	int numSymbols;
	int bitmask;

	bool phaseShift;
};



#endif /* INCLUDE_CONSTELLATION_BPSK_H_ */
