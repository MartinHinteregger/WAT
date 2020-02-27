/* ==================================================================
 * title:		Qpsk.cpp
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 *
 * ==================================================================
 */

#include <Qpsk.h>

Qpsk::Qpsk()
{
	printDebugLine("Qpsk()");
	this->constellationID = qpskID;
	this->constellationName = "qpsk";
	this->numBits = 2;
	this->numSymbols = pow(2, this->numBits);
	bitmask = 0b00000011;
}

Qpsk::~Qpsk()
{
	printDebugLine("~Qpsk()");
}

complex16_t Qpsk::modulateSingleSymbol(int8_t toMod)
{
	printDebugLine("Qpsk::modulate");
	if (toMod & 0b00000010)
	{
		if (toMod & 0b00000001)
			return {maxTxResolutionI16, maxTxResolutionI16}; // 11
		else
			return {maxTxResolutionI16, -maxTxResolutionI16}; // 10
	}
	else
	{
		if (toMod & 0b00000001)
			return {-maxTxResolutionI16, maxTxResolutionI16}; // 01
		else
			return {-maxTxResolutionI16, -maxTxResolutionI16}; // 00
	}
}

int8_t Qpsk::demodulateSingleSymbol(int16_t i_, int16_t q_)
{
	printDebugLine("Qpsk::demodulate");
	return 0;
}

char Qpsk::demodulateToChar(int16_t toDemod[], int len)
{
	return 0;
}

int Qpsk::getConstellationID()
{
	printDebugLine("Qpsk::getConstellationID");
	return constellationID;
}

const char *Qpsk::getConstellationName()
{
	printDebugLine("Qpsk::getConstellationName");
	return constellationName;
}

int Qpsk::getNumBits()
{
	printDebugLine("Qpsk::getNumBits");
	return numBits;
}

int Qpsk::getSymbolsBits()
{
	printDebugLine("Qpsk::getSymbolsBits");
	return numSymbols;
}

int Qpsk::getBitmask()
{
	printDebugLine("Qpsk::getBitmask");
	return bitmask;
}


