/* ==================================================================
 * title:		Bpsk.cpp
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 *
 * ==================================================================
 */

#include <Bpsk.h>

Bpsk::Bpsk()
{
	printDebugLine("Bpsk()");
	this->constellationID = bpskID;
	this->constellationName = "bpsk";
	this->numBits = 1;
	this->numSymbols = pow(2, this->numBits);
	bitmask = 0b00000001;

	phaseShift = false;
}

Bpsk::~Bpsk()
{
	printDebugLine("~Bpsk()");
}

complex16_t Bpsk::modulateSingleSymbol(int8_t toMod)
{
	//printDebugLine("Bpsk::modulate");

	if (toMod & 0b00000001) // 1
	{
		return {maxTxResolutionI16, 0};
	}
	else // 0
	{
		return {-maxTxResolutionI16, 0};
	}
}

int8_t Bpsk::demodulateSingleSymbol(int16_t i_, int16_t q_)
{
	//printDebugLine("Bpsk::demodulate");
	if (!phaseShift)
	{
		if (i_ > 0)
			return 1;
		else
			return 0;
	}
	else
	{
		if (i_ > 0)
			return 0;
		else
			return 1;
	}
	return 0;
}

char Bpsk::demodulateToChar(int16_t toDemod[], int len)
{
	//printDebugLine("Bpsk::demodulateToChar");
	char ret = 0;

	for (int i = 0; i < len/2; i++)
	{
		ret = (ret << 1) | demodulateSingleSymbol(toDemod[2*i], toDemod[2*i+1]);
	}
	return ret;
}

int Bpsk::getConstellationID()
{
	printDebugLine("Bpsk::getConstellationID");
	return constellationID;
}

const char *Bpsk::getConstellationName()
{
	printDebugLine("Bpsk::getConstellationName");
	return constellationName;
}

int Bpsk::getNumBits()
{
	printDebugLine("Bpsk::getNumBits");
	return numBits;
}

int Bpsk::getSymbolsBits()
{
	printDebugLine("Bpsk::getSymbolsBits");
	return numSymbols;
}

int Bpsk::getBitmask()
{
	printDebugLine("Bpsk::getBitmask");
	return bitmask;
}




