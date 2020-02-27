/* ==================================================================
 * title:		stream.h
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 * ==================================================================
 */

#ifndef INCLUDE_STREAM_H_
#define INCLUDE_STREAM_H_

#include "globals.h"
#include "Device.h"

// LimeSuite and externals.
#include "dataTypes.h"
#include "LMS7002M_parameters.h"
#ifdef USE_GNU_PLOT
#include "gnuPlotPipe.h"
#endif

#include <fstream>
#include <thread>
#include <math.h>
#include <limits.h>
#include "sys/stat.h"
#include "sys/types.h"

using namespace std;

// The integer commands available during pause of stream.
enum iCommands
{
	iCONTINUE = 1,
	iQUIT = 2,
	iNEWDESTFILE = 4,
	iCHANGECONSTELLATION = 5,
	iPRINTTXDATA = 10,
	iPRINTRXDATA = 11,
	iPRINTSTREAMDATA = 15,
	iPRINTDEVICEINFO = 16,
	iPRINTTXDATATOFILE = 18,
	iPRINTRXDATATOFILE = 19,
	iCHANGELPBW = 20,
	iGETLPBW = 21,
	iCHANGESAMPLING = 22,
	iGETSAMPLING = 23,
	iCHANGEGAIN = 24,
	iGETGAIN = 25,
	iCHANGECLOCK = 26,
	iGETCLOCK = 27,
	iTUNECLOCK = 28,
	iCHANGELO = 30,
	iGETLO = 31,
	iCHANGEANTENNAPORT = 32,
	iGETANTENNAPORT = 33,
	iSETGFIRLPF = 34,
	iSETINTANDDEC = 36,
//	iCHANGETHVSLAT = 50,
	iONETONE = 100,
	iSPIMODE = 200
};

// Argument for pause thread.
typedef struct pauseThreadArgs_
{
	Device *dev_;
} pauseThreadArgs;


class Stream {
public:
	Stream(deviceVector& deviceVec, int destID, int sourceID);
	~Stream();

	// Initialization
	int setupStream();
	int createResultsFile();
	int fileManagement(const char* sourceFilename);

	// The primary stream function
	int startStream(bool continuousMode);
	// The loop during pause of stream
	int pauseLoop(int16_t* tx_buffer, int &tx_size, int16_t* rx_buffer, int &rx_size,
			bool continuousMode);
	// SPI loop during pause of stream
	void SPIMode();

	// Data calculation and manipulation
	int modulateData(int16_t* tx_buffer, int tx_size, bool continuousMode);

	// Data In-/Output
	void plotTxData(int16_t* tx_buffer, int tx_size);
	bool printTxDataToFile(int16_t* tx_buffer, int tx_size);
	bool printRxDataToFile(int16_t* rx_buffer, int rx_size);
	string returnStreamStatus(int channel);

	// Various
	int setIntpAndDeci(int interpolation, int decimation);
	//preamble();
private:
	Device *rxDev;
	Device *txDev;
	lms_stream_t rx_stream[globalNumChannels];
	lms_stream_t tx_stream[globalNumChannels];
	lms_stream_status_t rx_status[globalNumChannels];
	lms_stream_status_t tx_status[globalNumChannels];
	streamsize lengthSourceBytes;
	int maximumBufferSize;

	string sourceFileName;
	fstream fsSourceFile;
	FILE *fdDestinationFile_ch0;
	FILE *fdDestinationFile_ch1;
	FILE *fdResultsFile;

#ifdef USE_GNU_PLOT
	GNUPlotPipe gppRx, gppTx;
#endif
};

// Thread function for pause thread.
void *streamPause(void *args_);

#endif /* INCLUDE_STREAM_H_ */
