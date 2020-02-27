/* ==================================================================
 * title:		Constellation.cpp
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 *
 * ==================================================================
 */

#include <Constellation.h>

Constellation::Constellation()
{
	printDebugLine("constellation()");
	constellationID = 0;
	constellationName = "Empty";
	numBits = 1;
	numSymbols = 1;
	bitmask = 0;
}

Constellation::~Constellation()
{
	printDebugLine("~constellation()");

}

complex16_t Constellation::modulateSingleSymbol(int8_t toMod)
{
	printDebugLine("Error: constellation::modulate");
	// Should not be reached due to abstract.
	return {-1, -1};
}

int8_t Constellation::demodulateSingleSymbol(int16_t i_, int16_t q_)
{
	printDebugLine("Error: constellation::demodulateSingleSymbol");
	// Should not be reached due to abstract.
	return 0;
}

char Constellation::demodulateToChar(int16_t toDemod[], int len)
{
	printDebugLine("Error: constellation::demodulateToChar");
	// Should not be reached due to abstract.
	return -1;
}

int Constellation::getConstellationID()
{
	printDebugLine("Error: constellation::getConstellationID");
	// Should not be reached due to abstract.
	return constellationID;
}

const char *Constellation::getConstellationName()
{
	printDebugLine("Error: constellation::getConstellationName");
	// Should not be reached due to abstract.
	return constellationName;
}

int Constellation::getNumBits()
{
	printDebugLine("Error: constellation::getNumBits");
	// Should not be reached due to abstract.
	return numBits;
}

int Constellation::getSymbolsBits()
{
	printDebugLine("Error: constellation::getSymbolsBits");
	// Should not be reached due to abstract.
	return numSymbols;
}

int Constellation::getBitmask()
{
	printDebugLine("Error: constellation::getBitmask");
	// Should not be reached due to abstract.
	return bitmask;
}
