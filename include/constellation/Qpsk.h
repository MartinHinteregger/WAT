/* ==================================================================
 * title:		Qpsk.h
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 *
 * ==================================================================
 */

#ifndef INCLUDE_CONSTELLATION_QPSK_H_
#define INCLUDE_CONSTELLATION_QPSK_H_

#include "Constellation.h"

using namespace std;

class Qpsk : public Constellation
{
public:
	Qpsk();
	~Qpsk();

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
};



#endif /* INCLUDE_CONSTELLATION_QPSK_H_ */
