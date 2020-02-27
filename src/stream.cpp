/* ==================================================================
 * title:		stream.cpp
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 * ==================================================================
 */

#include "stream.h"

bool pauseStream;
extern std::vector<const LMS7Parameter*> LMS7parameterList;
bool send;

/*
 * Stream(deviceVector& deviceVec, int destID, int sourceID)
 * Create a stream object. Pass the constructor a vector of available devices
 * and IDs of tx and rx.
 */
Stream::Stream(deviceVector& deviceVec, int destID, int sourceID)
{
	bool foundTX = false, foundRX = false;

	if (deviceVec.empty())
		throw std::invalid_argument("Stream: Device list is empty.");

	// Check if devices are available
	for (const auto& device : deviceVec)
	{
		if (destID == device->getId())
		{
			foundRX = true;
			rxDev = device;
		}
		if (sourceID == device->getId())
		{
			foundTX = true;
			txDev = device;
		}
	}

	if (!(foundRX & foundTX))
		throw std::invalid_argument("Stream: At least one of the specified devices is not in the device list.");

	lengthSourceBytes = 0;
	maximumBufferSize = 0;
	fdDestinationFile_ch0 = NULL;
	fdDestinationFile_ch1 = NULL;
	fdResultsFile = NULL;
	for (int chan = 0; chan < globalNumChannels; chan++)
	{
		// TODO FIFO size and throughput should be setable.
		rx_stream[chan].channel = chan;
		rx_stream[chan].fifoSize = 10 * 1024;
		rx_stream[chan].throughputVsLatency = 0.5;
		rx_stream[chan].dataFmt = lms_stream_t::LMS_FMT_I16;
		rx_stream[chan].isTx = false;

		tx_stream[chan].channel = chan;
		tx_stream[chan].fifoSize = 2 * 1024;
		tx_stream[chan].throughputVsLatency = 0.5;
		tx_stream[chan].dataFmt = lms_stream_t::LMS_FMT_I16;
		tx_stream[chan].isTx = true;
	}
}

/*
 * ~Stream()
 * Try to destroy stream objects and close filedescriptors.
 */
Stream::~Stream()
{
	for (int chan = 0; chan < globalNumChannels; chan++)
	{
		rxDev->devStopStream(&rx_stream[chan]);
		rxDev->devDestroyStream(&rx_stream[chan]);

		txDev->devStopStream(&tx_stream[chan]);
		txDev->devDestroyStream(&tx_stream[chan]);
	}

	fsSourceFile.close();
	if (fdDestinationFile_ch0 != NULL)
		fclose(fdDestinationFile_ch0);
	if (fdDestinationFile_ch1 != NULL)
		fclose(fdDestinationFile_ch1);
	if (fdResultsFile != NULL)
		fclose(fdResultsFile);
}

/*
 * setupStream()
 * Configure both tx and rx devices with default values, setup stream threads.
 */
int Stream::setupStream()
{
	// The following code was commented out, because it was useful for development:
	// Instead of using WAT commands in console to configure devices every time the
	// program was started, it was easier to just hard code them here and change them
	// and recompile if necessary.

	// Standard configuration examples
	float_type bandwidth = 5e6, loFreq = 2450e6;
	float_type samplingRate = 1e6;
	int oversampling = 4;
	int gainTX = 40, gainRX = 40;

	// Init and reset both devices
	rxDev->devReset();
	rxDev->devInit();
	if (rxDev->getId() != txDev->getId())
	{
		txDev->devReset();
		txDev->devInit();
	}

	// Set sampling rates for both devices
	// Either with this (set sampling rates of RX and TX of both devices)
	if (rxDev->devSetSamplingRate(samplingRate, oversampling) ||
		txDev->devSetSamplingRate(samplingRate, oversampling))
	{
		printConsoleAndDebugLine("Could not set sampling rates.");
		return -1;
	}
	// or this (set sampling rate of RX on one device and sampling rate of TX of the other
//	if (rxDev->devSetSamplingRate(samplingRate, oversampling, LMS_CH_RX) ||
//		txDev->devSetSamplingRate(samplingRate, oversampling, LMS_CH_TX))
//	{
//		printConsoleAndDebugLine("Could not set sampling rates.");
//		return -1;
//	}

	// Set up devices for both channels
	for (int chan = 0; chan < globalNumChannels; chan++)
	{
		// RX path of RX dev
		if (rxDev->devEnable(LMS_CH_RX, true, chan) ||
				rxDev->devSetAntennaPorts(chan, LMS_CH_RX, 1) ||
				rxDev->devSetLOFreq(chan, LMS_CH_RX, loFreq) ||
				rxDev->devSetLPBW(chan, LMS_CH_RX, bandwidth) ||
				txDev->devSetGain(chan, LMS_CH_RX, gainRX))// ||
				//rxDev->devCalibrate(LMS_CH_RX, bandwidth, chan))
		{
			printConsoleAndDebugLine("RX: Could not set up RX device.", chan);
			return -1;
		}
		// TX path of RX dev (only if different)
		if (rxDev->getId() != txDev->getId())
		{
			if (rxDev->devEnable(LMS_CH_TX, true, chan) ||
					rxDev->devSetAntennaPorts(chan, LMS_CH_TX, 1) ||
					rxDev->devSetLOFreq(chan, LMS_CH_TX, loFreq) ||
					rxDev->devSetLPBW(chan, LMS_CH_TX, bandwidth) ||
					txDev->devSetGain(chan, LMS_CH_RX, gainTX))// ||
					//rxDev->devCalibrate(LMS_CH_TX, bandwidth, chan))
			{
				printConsoleAndDebugLine("TX: Could not set up RX device.", chan);
				return -1;
			}
		}

		// TX path of TX dev
		if (txDev->devEnable(LMS_CH_TX, true, chan) ||
				txDev->devSetAntennaPorts(chan, LMS_CH_TX, 1) ||
				txDev->devSetLOFreq(chan, LMS_CH_TX, loFreq) ||
				txDev->devSetLPBW(chan, LMS_CH_TX, bandwidth) ||
				txDev->devSetGain(chan, LMS_CH_TX, gainTX))//||
				//txDev->devCalibrate(LMS_CH_TX, bandwidth, chan))
		{
			printConsoleAndDebugLine("TX: Could not set up TX device.", chan);
			return -1;
		}
		// RX path of TX dev (only if different)
		if (rxDev->getId() != txDev->getId())
		{
			if (txDev->devEnable(LMS_CH_RX, true, chan) ||
					txDev->devSetAntennaPorts(chan, LMS_CH_RX, 1) ||
					txDev->devSetLOFreq(chan, LMS_CH_RX, loFreq) ||
					txDev->devSetLPBW(chan, LMS_CH_RX, bandwidth) ||
					txDev->devSetGain(chan, LMS_CH_RX, gainRX))// ||
					//txDev->devCalibrate(LMS_CH_RX, bandwidth, chan))
			{
				printConsoleAndDebugLine("RX: Could not set up TX device.", chan);
				return -1;
			}
		}
	}

	// Set up devices for both channels
	for (int chan = 0; chan < globalNumChannels; chan++)
	{
		if (rxDev->devSetupStream(&rx_stream[chan]))
		{
			printConsoleAndDebugLine("RX: Could not setup stream.", chan);
			return -1;
		}
		if (txDev->devSetupStream(&tx_stream[chan]))
		{
			printConsoleAndDebugLine("TX: Could not setup stream.", chan);
			return -1;
		}
	}

	return 0;
}

/*
 * fileManagement(const char* sourceFilename)
 * Try to open the source file and set up result files.
 */
int Stream::fileManagement(const char* sourceFilename)
{
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) == NULL)
	{
		printConsoleAndDebugLine("Could not get current directory.");
		return true;
	}
	strcat(cwd, "/");
	strcat(cwd, sourceFilename);
	//strcat(cwd, ".txt");
	sourceFileName = string(sourceFilename);

	try
	{
		fsSourceFile.open(cwd, fstream::in | fstream::binary);
		if (fsSourceFile.fail())
		{
			printConsoleAndDebugLine("Could not open source file.");
			printConsoleAndDebugLine(cwd);
			return -1;
		}

		if (createResultsFile())
		{
			printConsoleAndDebugLine("Could not create results file.");
			return -1;
		}
	}
	catch (exception& E)
	{
		printConsoleAndDebugLine(E.what());
		return errno;
	}

	return 0;
}

/*
 * createResultsFile()
 * From the current path, add some information (modulation, ...) and the source file name
 * membervariable, set up the results file and the destination file in folder "tests".
 */
int Stream::createResultsFile()
{
	// This whole function is just terribly written
	if (fdDestinationFile_ch0 != NULL)
		fclose(fdDestinationFile_ch0);
	if (fdDestinationFile_ch1 != NULL)
		fclose(fdDestinationFile_ch1);
	if (fdResultsFile != NULL)
		fclose(fdResultsFile);

	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) == NULL)
	{
		printConsoleAndDebugLine("Could not get current directory.");
		return -1;
	}

	int ret = mkdir(strcat(cwd, "/tests"), S_IRWXU | S_IRWXG | S_IRWXO);
	if (ret != 0 || ret != EEXIST)
	{
		// Somehow mkdir returns -1 even if EEXIST should be returned.
		// So this line will be called every time.
		printDebugLine("Could not create directory \"/tests\".");
	}

	strcat(cwd, "/");
	// Both devices should have the same constellation, just take TX
	strcat(cwd, txDev->constel->getConstellationName());
	strcat(cwd, "_");

	fdResultsFile = fopen(cwd, "w");
	strcat(cwd, sourceFileName.c_str());
	fdDestinationFile_ch0 = fopen(cwd, "w");
	strcat(cwd, "_ch2");
	fdDestinationFile_ch1 = fopen(cwd, "w");

	if (fdResultsFile == NULL || fdDestinationFile_ch0 == NULL || fdDestinationFile_ch1 == NULL)
		return -1;

	return 0;
}

/*
 * modulateData(int16_t tx_buffer[], size_t tx_size, bool continuousMode)
 * Modulate the data from the source file into tx_buffer with size tx_size.
 * If continuousMode is true, tx_size can be higher and data will be modulated from
 * start after reaching eof until tx_buffer has size tx_size.
 * The function is hard to read since it uses bitshifts.
 */
int Stream::modulateData(int16_t tx_buffer[], int tx_size, bool continuousMode)
{
	// This function will not work when
	// + Modulation is larger than 64QAM
	// + The file is too large for 32 bit (/2) integer size.

	// Just in case we are not at the beginning of the file.
	fsSourceFile.clear();
	fsSourceFile.seekg(0);

	int modNumBits = txDev->constel->getNumBits();
	int bitmask = txDev->constel->getBitmask();

	if (modNumBits > 8)
	{
		printConsoleAndDebugLine("Error: Modulation lager than 64QAM not implemented.");
		return -1;
	}

    char bitStream;
    complex16_t modulatedPoint;

    int i = 0;
	// Modulate this batch
    // Endless loop is not optimal...
	for (;;)
	{
		// Read next 8 bits, return if eof or at buffer size.
		fsSourceFile.get(bitStream);

		if (i >= 2*tx_size)
		{
			if (!continuousMode && !fsSourceFile.eof())
				printConsoleAndDebugLine("Modulation Warning: Buffersize is to small, "
						"data may be incomplete.");
			return i;
		}

    	if (fsSourceFile.eof())
    	{
    		if (continuousMode)
    		{
        		// Start from beginning of file
        		fsSourceFile.clear();
        		fsSourceFile.seekg(0);
        		fsSourceFile.get(bitStream);
    		}
    		else
    			return i;
    	}

		for (int j = 0; j*modNumBits < 8; j++)
		{
			// This bitshift should work for all modulations that use less than 8 bits per symbol.
			modulatedPoint = txDev->constel->modulateSingleSymbol((bitStream >> (8-modNumBits*(1+j))) & bitmask);
			tx_buffer[i] = modulatedPoint.i;
			i++;
			tx_buffer[i] = modulatedPoint.q;
			i++;

			if (i >= 2*tx_size)
			{
				break;
			}
		}
	}
	return i;
}

/*
 * startStream(bool continuousMode)
 * This function will do the actual data transmission:
 * Prepare the data, start the stream threads, send the data, receive the data,
 * process the data.
 * If continuousMode is true, the data will be sent constantly.
 */
int Stream::startStream(bool continuousMode)
{
	printDebugLine("Stream: startStream.");

	int received;
	send = true;

	// Get length of file in bytes
	fsSourceFile.ignore(numeric_limits<std::streamsize>::max());
	lengthSourceBytes = fsSourceFile.gcount();
	fsSourceFile.clear();
	fsSourceFile.seekg(0);
	int modNumBits = txDev->constel->getNumBits();

	// Calculate the size we need
	int tx_size = (lengthSourceBytes * 8 / modNumBits);
	// Calculate maximum buffer size we need (modNumBits == 1)
	maximumBufferSize = lengthSourceBytes * 8;

	if (continuousMode && tx_size <= 510)
	{
		tx_size = 510;
		maximumBufferSize = 510;
	}

	int16_t tx_buffer[2*maximumBufferSize];

	received = this->modulateData(tx_buffer, tx_size, continuousMode);

	// Just in case the calculated size was wrong
	if (received / 2 != tx_size)
	{
		printConsoleAndDebugLine("startStream: Resize buffer size.", received/2);
		tx_size = received/2;
	}

	// Set up receive buffer
    int rx_size = tx_size;
    int16_t rx_buffer[2*maximumBufferSize];

    // Demodulated data buffers for both channels
    char rx_data[2][tx_size/8];

#ifdef USE_GNU_PLOT
    gppRx.write("set size square\n set xrange[-35000:35000]\n set yrange[-35000:35000]\n set grid\n"
			" set title 'I/Q constellation rx'\n set xlabel 'I'\n set ylabel 'Q'\n");

	this->plotTxData(tx_buffer, tx_size);
#endif

    pauseStream = false;
	// Start stream threads.
	for (int chan = 0; chan < globalNumChannels; chan++)
	{
		rxDev->devStartStream(&rx_stream[chan]);
		txDev->devStartStream(&tx_stream[chan]);
	}

	// Start streampause thread.
	pauseThreadArgs thArgs;
	thArgs.dev_ = rxDev;
	pthread_t pauseThread;
	if (pthread_create(&pauseThread, NULL, streamPause, (void*)&thArgs))
	{
		printConsoleAndDebugLine("Failed to create pause thread.");
		return -1;
	}

#ifdef USE_GNU_PLOT
    auto t_plot = chrono::high_resolution_clock::now();
#endif
    bool run = true;
    while (run)
    {
    	// Send the data
    	if (send || continuousMode)
    	{
        	for (int chan = 0; chan < globalNumChannels; chan++)
        	{
        		txDev->devSendStream(&tx_stream[chan], tx_buffer, tx_size, NULL, 100);
        	}
    		send = false;
    	}

    	// Receive the data
    	for (int chan = 0; chan < globalNumChannels; chan++)
    	{
    		received = rxDev->devReceiveStream(&rx_stream[chan], rx_buffer, rx_size, NULL, 100);
    		if (received)
    		{

    			// The following code was programmed at the end of the project
    			// It was a poor try to implement a basic phase shift of the data by
    			// calculating the average phase and subtracting this value from the samples.
    			// It's a very inefficient code. So it just broke the program.
    			// After this, the project was discontinued.

//    			modNumBits = rxDev->constel->getNumBits();
//    			int step = 16 / modNumBits;
//    			int16_t buf[step];
//    			int length = 0;
//    			float magnitude[rx_size];
//    			float angles[rx_size];
//    			float angle_avg = 0;
//    			char dataPrev = 0;
//    			uint16_t twoWord;
//    			uint16_t tmp;
//    			uint16_t tmp2;
//    			bool preambleFound = false;
//
//    			// For now only valid for BPSK
//    			for (int j = 0; j < rx_size; j++)
//    			{
//    				magnitude[j] = sqrt(pow(rx_buffer[2*j],2)+pow(rx_buffer[2*j+1],2));
//    				angles[j] = acos(abs(rx_buffer[2*j])/magnitude[j]);
//    				if(rx_buffer[2*j+1] < 0)
//    				{
//        				angle_avg += angles[j];
//    					angles[j] += M_PI;
//    				}
//    				else
//        				angle_avg += angles[j];
//    			}
//    			angle_avg = angle_avg / rx_size;
//    			for (int j = 0; j < rx_size; j++)
//    			{
//    				angles[j] -= angle_avg + M_PI;
//    				rx_buffer[2*j] = cos(angles[j])*magnitude[j];
//    				rx_buffer[2*j+1] = sin(angles[j])*magnitude[j];
//    			}
//
//    			for (int j = 0; j < 2*rx_size;)
//    			{
//    				if (j + step >= 2*rx_size)
//    				{
//    					cout << "Cut: RX_Buffer size not modulo 16.\n";
//    					break;
//    				}
//    				memcpy(buf, rx_buffer+j, step*sizeof(int16_t));
//    				rx_data[chan][length] = rxDev->constel->demodulateToChar(buf, step);
//
//    				tmp = (uint16_t)dataPrev;
//    				tmp2 = (uint16_t)rx_data[chan][length];
//    				twoWord = (tmp << 8) | tmp2;
//    				if (!preambleFound)
//    				{
//						for (int i = 0; i < 8; i++)
//						{
//							if ((twoWord >> i) == 'U')
//							{
//								preambleFound = true;
//								cout << "YEEEESSSS\n";
//							}
//						}
//    				}
//
//    				if (preambleFound)
//    				{
//    					if (chan == 0)
//    						fprintf(fdDestinationFile_ch0, "%c", rx_data[chan][length]);
//    					if (chan == 1)
//    						fprintf(fdDestinationFile_ch1, "%c", rx_data[chan][length]);
//    				}
//
//    				dataPrev = rx_data[chan][length];
//    				j += step;
//    				length++;
//    			}

    			for (int i = 0; i < rx_size; i++)
    			{
        			if (chan == 0)
        				fprintf(fdDestinationFile_ch0, "%c", rx_data[0][i]);
        			if (chan == 1)
        				fprintf(fdDestinationFile_ch1, "%c", rx_data[1][i]);
    			}
    		}
    	}

    	// Plot every 1 second.
#ifdef USE_GNU_PLOT
		if ((chrono::high_resolution_clock::now() - t_plot > chrono::seconds(1))
				&& received) {
			gppRx.write("plot '-' with points title 'rx'\n");
			for (int j = 0; j < received; ++j)
				gppRx.writef("%i %i\n", rx_buffer[2 * j], rx_buffer[2 * j + 1]);
			gppRx.write("e\n");
			gppRx.flush();
			t_plot = chrono::high_resolution_clock::now();
		}
#endif

		// User hit pause key in the pause thread
    	if (pauseStream)
    	{
    		// Start the pause loop for options
    		if (pauseLoop(tx_buffer, tx_size, rx_buffer, rx_size, continuousMode))
    		{
    		    // TODO problem: When restarting the stream, the pause thread does not respond to inputs.
    		    // User hit quit, so exit
    			pthread_cancel(pauseThread);
    		    this_thread::yield();
    			run = false;
    			continue;
    		}
    		// User hit continue, restart the stream.
    		for (int chan = 0; chan < globalNumChannels; chan++)
    		{
    			rxDev->devStartStream(&rx_stream[chan]);
    			txDev->devStartStream(&tx_stream[chan]);
    		}
    		pauseStream = false;
    	}
    }

	printConsoleAndDebugLine("Stream finished.");
	return 0;
}

/*
 * pauseLoop(int16_t* tx_buffer, int tx_size, int16_t* rx_buffer, size_t rx_size,
 *		bool continuousMode)
 * When the stream is paused by the pauseThread (Setting a global variable), this function
 * is called. It polls in a loop the commands (defined as integers in the header file) from
 * the user. If it returns 0, stream should continue. Else, stream should stop.
 */
int Stream::pauseLoop(int16_t* tx_buffer, int &tx_size, int16_t* rx_buffer, int &rx_size,
		bool continuousMode)
{
	int cmd, iCmd, ret;
	string sCmd;
	float_type fCmd;
	bool bCmd;
	bool runLoop = true;
	// Stop all streams
	for (int chan = 0; chan < globalNumChannels; chan++)
	{
		rxDev->devStopStream(&rx_stream[chan]);
		txDev->devStopStream(&tx_stream[chan]);
	}

	// Print out stream status
	for (int chan = 0; chan < globalNumChannels; chan++)
	{
		txDev->devGetStreamStatus(&tx_stream[chan], &tx_status[chan]);
		rxDev->devGetStreamStatus(&rx_stream[chan], &rx_status[chan]);
		sCmd = returnStreamStatus(chan);
		printConsoleAndDebugLine(sCmd.c_str());
	}

	printConsoleAndDebugLine("Halt stream confirmed. Ready for integer commands.");

	while (runLoop)
	{
		cout << "paused=>";
		cin >> cmd;
		cin.ignore();

		switch (cmd)
		{
		case iCONTINUE:
			runLoop = false;
			return 0;
		case iQUIT:
			return -1;
		case iNEWDESTFILE:
			this->createResultsFile();
			break;
		case iCHANGECONSTELLATION:
			cout << "paused=>i" << iCHANGECONSTELLATION << "=>";
			cin >> iCmd;
			cin.ignore();
			if (!txDev->changeConstellation(iCmd))
			{
				ret = modulateData(tx_buffer, maximumBufferSize, continuousMode);
				if (ret / 2 != tx_size)
				{
					printConsoleAndDebugLine("Resize buffer size. ", ret/2);
					tx_size = ret/2;
				}
				rx_size = tx_size;
				createResultsFile();
			}
			else
				printConsoleAndDebugLine("Change TX constellation failed.");

			if (txDev->getId() != rxDev->getId())
			{
				if(rxDev->changeConstellation(iCmd))
					printConsoleAndDebugLine("Change RX constellation failed.");
			}

			this->plotTxData(tx_buffer, tx_size);
			break;
		case iPRINTTXDATA:
			for (int i = 0; i < tx_size/2; i++)
				cout << i << ": " << tx_buffer[2*i] << " - " << tx_buffer[2*i+1] << "  |  "
					 << i + tx_size/2 << ": " << tx_buffer[2*(i + tx_size/2)] << " - "
					 << tx_buffer[2*(i+tx_size/2)+1] <<"\n";
			break;
		case iPRINTRXDATA:
			for (int i = 0; i < rx_size/2; i++)
				cout << i << ": " << rx_buffer[2*i] << " - " << rx_buffer[2*i+1] << "  |  "
					 << i + rx_size/2 << ": " << rx_buffer[2*(i + rx_size/2)] << " - "
					 << rx_buffer[2*(i+rx_size/2)+1] <<"\n";
			break;
		case iPRINTSTREAMDATA:
	    	for (int chan = 0; chan < globalNumChannels; chan++)
	    	{
				txDev->devGetStreamStatus(&tx_stream[chan], &tx_status[chan]);
				rxDev->devGetStreamStatus(&rx_stream[chan], &rx_status[chan]);
				sCmd = returnStreamStatus(chan);
				printConsoleAndDebugLine(sCmd.c_str());
	    	}
	    	break;
		case iPRINTDEVICEINFO:
			cout << "TX:\n";
			txDev->devPrintInfo();
			cout << "RX:\n";
			rxDev->devPrintInfo();
			break;
		case iPRINTTXDATATOFILE:
			this->printTxDataToFile(tx_buffer, tx_size);
			cout << tx_size << "\n";
			break;
		case iPRINTRXDATATOFILE:
			this->printRxDataToFile(rx_buffer, rx_size);
			cout << rx_size << "\n";
			break;
		case iCHANGELPBW:
			cout << "paused=>f" << iCHANGELPBW << "=>";
			cin >> fCmd;
			cin.ignore();

			if (fCmd == 0.0)
				break;

	    	for (int chan = 0; chan < globalNumChannels; chan++)
	    	{
				txDev->devSetLPBW(chan, LMS_CH_TX, fCmd*1e6);
				txDev->devSetLPBW(chan, LMS_CH_RX, fCmd*1e6);
	    		txDev->devCalibrate(LMS_CH_TX, fCmd*1e6, chan);
	    		txDev->devCalibrate(LMS_CH_RX, fCmd*1e6, chan);
	    	}
			if (txDev->getId() != rxDev->getId())
			{
		    	for (int chan = 0; chan < globalNumChannels; chan++)
		    	{
					rxDev->devSetLPBW(chan, LMS_CH_TX, fCmd*1e6);
					rxDev->devSetLPBW(chan, LMS_CH_RX, fCmd*1e6);
		    		rxDev->devCalibrate(LMS_CH_TX, fCmd*1e6, chan);
		    		rxDev->devCalibrate(LMS_CH_RX, fCmd*1e6, chan);
		    	}
			}
			break;
		case iGETLPBW:
			cout << "TX:\n";
			sCmd = txDev->devGetLPBW(0);
			printConsoleAndDebugLine(sCmd.c_str());
			sCmd = txDev->devGetLPBW(1);
			printConsoleAndDebugLine(sCmd.c_str());
			if (txDev->getId() != rxDev->getId())
			{
				cout << "RX:\n";
				sCmd = rxDev->devGetLPBW(0);
				printConsoleAndDebugLine(sCmd.c_str());
				sCmd = rxDev->devGetLPBW(1);
				printConsoleAndDebugLine(sCmd.c_str());
			}
			break;
		case iCHANGESAMPLING:
			cout << "paused=>f" << iCHANGESAMPLING << "=>";
			cin >> fCmd;
			cin.ignore();
			cout << "paused=>i" << iCHANGESAMPLING << "=>";
			cin >> iCmd;
			cin.ignore();

			if (fCmd == 0.0 || iCmd == 0)
				break;

			txDev->devSetSamplingRate(fCmd*1e6, iCmd);

			if (txDev->getId() != rxDev->getId())
				rxDev->devSetSamplingRate(fCmd*1e6, iCmd);
			break;
		case iGETSAMPLING:
			cout << "TX:\n";
			sCmd = txDev->devGetSamplingRate(0);
			printConsoleAndDebugLine(sCmd.c_str());
			sCmd = txDev->devGetSamplingRate(1);
			printConsoleAndDebugLine(sCmd.c_str());
			if (txDev->getId() != rxDev->getId())
			{
				cout << "RX:\n";
				sCmd = rxDev->devGetSamplingRate(0);
				printConsoleAndDebugLine(sCmd.c_str());
				sCmd = rxDev->devGetSamplingRate(1);
				printConsoleAndDebugLine(sCmd.c_str());
			}
			break;
		case iCHANGEGAIN:
			cout << "paused=>i" << iCHANGEGAIN << "=>";
			cin >> iCmd;
			cin.ignore();

			if (iCmd == 0)
				break;

			cout << "paused=>b" << iCHANGEGAIN << "=>";
			cin >> bCmd;
			cin.ignore();
	    	for (int chan = 0; chan < globalNumChannels; chan++)
	    	{
	    		txDev->devSetGain(chan, bCmd, iCmd);
	    		if (txDev->getId() != rxDev->getId())
		    		rxDev->devSetGain(chan, bCmd, iCmd);
	    	}
			break;
		case iGETGAIN:
			cout << "TX:\n";
			sCmd = txDev->devGetGain(0);
			printConsoleAndDebugLine(sCmd.c_str());
			sCmd = txDev->devGetGain(1);
			printConsoleAndDebugLine(sCmd.c_str());
			if (txDev->getId() != rxDev->getId())
			{
				cout << "RX:\n";
				sCmd = rxDev->devGetGain(0);
				printConsoleAndDebugLine(sCmd.c_str());
				sCmd = rxDev->devGetGain(1);
				printConsoleAndDebugLine(sCmd.c_str());
			}
			break;
		case iCHANGECLOCK:
			cout << "paused=>i" << iCHANGECLOCK << "=>";
			cin >> iCmd;
			cin.ignore();
			cout << "paused=>f" << iCHANGECLOCK << "=>";
			cin >> fCmd;
			cin.ignore();

			if (fCmd == 0.0)
				break;

			txDev->devSetSynthesiserFrequency(iCmd, fCmd*1e6);
			if (txDev->getId() != rxDev->getId())
				rxDev->devSetSynthesiserFrequency(iCmd, fCmd*1e6);
			break;
		case iGETCLOCK:
			cout << "TX:\n";
			sCmd = txDev->devGetSynthesiserFrequency(LMS_CLOCK_SXR);
			printConsoleAndDebugLine(sCmd.c_str());
			sCmd = txDev->devGetSynthesiserFrequency(LMS_CLOCK_SXT);
			printConsoleAndDebugLine(sCmd.c_str());
			sCmd = txDev->devGetSynthesiserFrequency(LMS_CLOCK_CGEN);
			printConsoleAndDebugLine(sCmd.c_str());
			if (txDev->getId() != rxDev->getId())
			{
				cout << "RX:\n";
				sCmd = rxDev->devGetSynthesiserFrequency(LMS_CLOCK_SXR);
				printConsoleAndDebugLine(sCmd.c_str());
				sCmd = rxDev->devGetSynthesiserFrequency(LMS_CLOCK_SXT);
				printConsoleAndDebugLine(sCmd.c_str());
				sCmd = rxDev->devGetSynthesiserFrequency(LMS_CLOCK_CGEN);
				printConsoleAndDebugLine(sCmd.c_str());
			}
			break;
		case iTUNECLOCK:
			cout << "paused=>i" << iCHANGECLOCK << "=>";
			cin >> iCmd;
			cin.ignore();

			txDev->devSetSynthesiserFrequency(iCmd, -1);
			if (txDev->getId() != rxDev->getId())
				rxDev->devSetSynthesiserFrequency(iCmd, -1);
			break;
		case iCHANGELO:
			cout << "paused=>f" << iCHANGELO << "=>";
			cin >> fCmd;
			cin.ignore();

			if (fCmd == 0.0)
				break;

			cout << "paused=>b" << iCHANGELO << "=>";
			cin >> bCmd;
			cin.ignore();
			for (int chan = 0; chan < globalNumChannels; chan++)
			{
				txDev->devSetLOFreq(chan, bCmd, fCmd);
				if (txDev->getId() != rxDev->getId())
					rxDev->devSetLOFreq(chan, bCmd, fCmd);
			}
			break;
		case iGETLO:
			cout << "TX:\n";
			for (int chan = 0; chan < globalNumChannels; chan++)
			{
				sCmd = txDev->devGetLOFreq(chan);
				printConsoleAndDebugLine(sCmd.c_str());
			}
			if (txDev->getId() != rxDev->getId())
			{
				cout << "RX:\n";
				for (int chan = 0; chan < globalNumChannels; chan++)
				{
					sCmd = rxDev->devGetLOFreq(chan);
					printConsoleAndDebugLine(sCmd.c_str());
				}
			}
			break;
		case iCHANGEANTENNAPORT:
			cout << "paused=>i" << iCHANGEANTENNAPORT << "=>";
			cin >> iCmd;
			cin.ignore();

			if (iCmd == 0)
				break;

			cout << "paused=>b" << iCHANGEANTENNAPORT << "=>";
			cin >> bCmd;
			cin.ignore();

			for (int chan = 0; chan < globalNumChannels; chan++)
			{
				txDev->devSetAntennaPorts(chan, bCmd, iCmd);
				if (txDev->getId() != rxDev->getId())
					rxDev->devSetAntennaPorts(chan, bCmd, iCmd);
			}
			break;
		case iGETANTENNAPORT:
			cout << "TX:\n";
			for (int chan = 0; chan < globalNumChannels; chan++)
			{
				sCmd = txDev->devGetAntennaPorts(chan);
				printConsoleAndDebugLine(sCmd.c_str());
			}
			if (txDev->getId() != rxDev->getId())
			{
				cout << "RX:\n";
				for (int chan = 0; chan < globalNumChannels; chan++)
				{
					sCmd = rxDev->devGetAntennaPorts(chan);
					printConsoleAndDebugLine(sCmd.c_str());
				}
			}
			break;
		case iSETGFIRLPF:
			cout << "paused=>i" << iSETGFIRLPF << "=>";
			cin >> iCmd;
			cin.ignore();

			cout << "paused=>b" << iSETGFIRLPF << "=>";
			cin >> bCmd;
			cin.ignore();

			cout << "paused=>f" << iSETGFIRLPF << "=>";
			cin >> fCmd;
			cin.ignore();

			if (fCmd == 0.0)
				break;

			for (int chan = 0; chan < globalNumChannels; chan++)
			{
				txDev->devSetGFIRLPF((bool)iCmd, chan, bCmd, fCmd*1e6);
				if (txDev->getId() != rxDev->getId())
					rxDev->devSetGFIRLPF((bool)iCmd, chan, bCmd, fCmd*1e6);
			}

			break;
		case iSETINTANDDEC:
			cout << "paused=>i" << iSETINTANDDEC << "=>Interpolation=>";
			cin >> iCmd;
			cin.ignore();

			// Sneaky cast
			cout << "paused=>i" << iSETINTANDDEC << "=>Decimation=>";
			cin >> fCmd;
			cin.ignore();

			if (this->setIntpAndDeci(iCmd, (int) fCmd))
				cout << "Failed.\n";
			break;
			// This is no longer necessary
//		case iCHANGETHVSLAT:
//			cout << "Warning: Changing this value will set up a new stream. Sure? (1/0)\n";
//			cout << "paused=>b" << iCHANGETHVSLAT << "=>";
//			cin >> bCmd;
//			cin.ignore();
//
//			if (!bCmd)
//				continue;
//
//			cout << "Currently set: " << tx_stream[0].throughputVsLatency << "\n";
//
//			cout << "paused=>f" << iCHANGETHVSLAT << "=>";
//			cin >> fCmd;
//			cin.ignore();
//
//			for (int chan = 0; chan < globalNumChannels; chan++)
//			{
//				tx_stream[chan].throughputVsLatency = fCmd;
//				rx_stream[chan].throughputVsLatency = fCmd;
//			}
//			for (int chan = 0; chan < globalNumChannels; chan++)
//			{
//				rxDev->devDestroyStream(&rx_stream[chan]);
//				txDev->devDestroyStream(&tx_stream[chan]);
//			}
//
//			for (int chan = 0; chan < globalNumChannels; chan++)
//			{
//				if (rxDev->devSetupStream(&rx_stream[chan]))
//					printConsoleAndDebugLine("RX: Could not setup stream.", chan);
//				if (txDev->devSetupStream(&tx_stream[chan]))
//					printConsoleAndDebugLine("TX: Could not setup stream.", chan);
//			}
//			break;
		case iONETONE:
			cout << "paused=>i" << iONETONE << "=>";
			cin >> iCmd;
			cin.ignore();
			for (int i = 0; i < tx_size; i++)
			{
				tx_buffer[2*i] = cos(2*M_PI*i/(iCmd))*maxTxResolutionI16;
				tx_buffer[2*i+1] = sin(2*M_PI*i/(iCmd))*maxTxResolutionI16;
			}
			break;
		case iSPIMODE:
			SPIMode();
			break;
		default:
			break;
		}
	}
	return -1;
}

void Stream::SPIMode()
{
	char cmd[32];
	char line[2048];
	char find[32];
	uint16_t val;
	bool is_tx = true;
	cout << "Entering SPI Mode, careful from now on. Write \"c\" to quit at all time.\n";

	while (true)
	{
		cout << "paused=>SPI=>rd(0)/wr(1)/help(2)/find(3)=>";
		cin.clear();
		cin >> cmd;
		cin.ignore();

		if (cmd[0] == 99) // c
			return;

		if (cmd[0] == 50) // 2
		{
			for (const auto& parameters : LMS7parameterList)
			{
				sprintf(line, "0x%04x / %2d:%2d/%3d; %s, %s\n", parameters->address, parameters->msb, parameters->lsb,
						parameters->defaultValue, parameters->name, parameters->tooltip);
				cout << line;
			}
			continue;
		}

		if (cmd[0] == 51) // 3
		{
			if (cmd[1] == '\0')
			{
				cout << "Find: write 3 and afterwords the string you want to find.\n";
				continue;
			}

			for (unsigned int i = 1; i < sizeof(cmd); i++)
			{
				find[i-1] = cmd[i];

				if (cmd[i+1] == '\0')
				{
					find[i] = '\0';
					break;
				}
			}

			for (const auto& parameters : LMS7parameterList)
			{
				sprintf(line, "0x%04x / %2d:%2d/%3d; %s, %s\n", parameters->address, parameters->msb, parameters->lsb,
						parameters->defaultValue, parameters->name, parameters->tooltip);
				if (strstr(line, find))
					cout << line;
			}
			continue;
		}

		if (cmd[0] == 48) // 0
		{
			cout << "paused=>SPI=>rd=>Name=>";
			cin >> cmd;
			cin.ignore();

			if (cmd[0] == 99) // c
				continue;

			if (txDev->getId() != rxDev->getId())
			{
				cout << "paused=>SPI=>rd=>rx(0)/tx(1)=>";
				cin >> is_tx;
				cin.ignore();
			}

			if (is_tx)
			{
				txDev->devReadParam(LMS7param(MAC), &val);
				if (val == 3)
				{
					cout << "Failed: MAC register is set to 3, read would corrupt data.\n";
					continue;
				}
			}
			else
			{
				rxDev->devReadParam(LMS7param(MAC), &val);
				if (val == 3)
				{
					cout << "Failed: MAC register is set to 3, read would corrupt data.\n";
					continue;
				}
			}

			if (cmd[0] == 99) // c
				continue;
			else
			{
				for (auto parameters : LMS7parameterList)
				{
					if (strstr(cmd, parameters->name))
					{
						if (is_tx)
							txDev->devReadParam(parameters->name, &val);
						else
							rxDev->devReadParam(parameters->name, &val);

						sprintf(line, "%s(0x%04x): 0x%02x/%d\n", parameters->name, parameters->address, val, val);
						cout << line;
						break;
					}
				}
			}
		}

		if (cmd[0] == 49) // 1
		{
			cout << "paused=>SPI=>wr=>Name=>";
			cin >> cmd;
			cin.ignore();

			if (cmd[0] == 99) // c
				continue;

			if (txDev->getId() != rxDev->getId())
			{
				cout << "paused=>SPI=>wr=>rx(0)/tx(1)=>";
				cin >> is_tx;
				cin.ignore();
			}

			if (cmd[0] == 99) // c
				continue;
			else
			{
				for (auto parameters : LMS7parameterList)
				{
					if (strstr(cmd, parameters->name))
					{
						if (is_tx)
							txDev->devReadParam(parameters->name, &val);
						else
							rxDev->devReadParam(parameters->name, &val);

						sprintf(line, "%s(0x%04x): 0x%02x/%d\n", parameters->name, parameters->address, val, val);
						cout << line;

						cout << "paused=>SPI=>wr=>Value=>";
						cin >> val;
						cin.ignore();

						if (is_tx)
						{
							if (txDev->devWriteParam(parameters->name, val) ||
									txDev->devReadParam(parameters->name, &val))
								cout << "Failed\n";
							else
							{
								sprintf(line, "%s(0x%04x): 0x%02x/%d\n", parameters->name, parameters->address, val, val);
								cout << line;
							}
						}
						else
						{
							if (rxDev->devWriteParam(parameters->name, val) ||
									rxDev->devReadParam(parameters->name, &val))
								cout << "Failed\n";
							else
							{
								sprintf(line, "%s(0x%04x): 0x%02x/%d\n", parameters->name, parameters->address, val, val);
								cout << line;
							}
						}
					}
				}
			}
		}
	}
}

/*
 * plotTxData(int16_t* tx_buffer, size_t tx_size)
 * (If closed) opens a new TX data window and plots all data in buffer until tx_size.
 */
void Stream::plotTxData(int16_t* tx_buffer, int tx_size)
{
#ifdef USE_GNU_PLOT
	gppTx.write("set size square\n set xrange[-35000:35000]\n set yrange[-35000:35000]\n set grid\n"
			" set title 'I/Q constellation tx'\n set xlabel 'I'\n set ylabel 'Q'\n");
	// Print tx data
	gppTx.write("plot '-' with points title 'tx'\n");
	for (int j = 0; j < tx_size; ++j)
		gppTx.writef("%i %i\n", tx_buffer[2*j], tx_buffer[2*j+1]);
	gppTx.write("e\n");
	gppTx.flush();
#endif
}

/*
 * returnStreamStatus(int channel)
 * Returns a string with the status of both tx and rx for the specified channel.
 */
string Stream::returnStreamStatus(int channel)
{
	char retStr[1024];
	sprintf(retStr, "Channel: %d                 TX |         RX\n"
					"Active:            %10d | %10d\n"
					"Dropped Packets:   %10d | %10d\n"
					"FIFO Filled Count: %10d | %10d\n"
					"FIFO Size:         %10d | %10d\n"
					"Link Rate:         %10f | %10f\n"
					"Overruns:          %10d | %10d\n"
					//"Sample Rate:		%10f | %10f\n"
					"Timestamp:         %10lu | %10lu\n"
					"Underruns:         %10d | %10d\n",
					channel,
					tx_status[channel].active, rx_status[channel].active,
					tx_status[channel].droppedPackets, rx_status[channel].droppedPackets,
					tx_status[channel].fifoFilledCount, rx_status[channel].fifoFilledCount,
					tx_status[channel].fifoSize, rx_status[channel].fifoSize,
					tx_status[channel].linkRate, rx_status[channel].linkRate,
					tx_status[channel].overrun, rx_status[channel].overrun,
					//tx_status[channel].sampleRate, rx_status[channel].sampleRate,
					tx_status[channel].timestamp, rx_status[channel].timestamp,
					tx_status[channel].underrun, rx_status[channel].underrun);

	return retStr;
}

/*
 * printTxDataToFile(int16_t* tx_buffer, int tx_size)
 * Take tx_buffer and print it into destination file
 */
bool Stream::printTxDataToFile(int16_t* tx_buffer, int tx_size)
{
	if (fdDestinationFile_ch0 == NULL)
		return true;

	for (int i = 0; i < tx_size; i++)
	{
		fprintf(fdDestinationFile_ch0, "%d, %d", tx_buffer[2*i], tx_buffer[2*i+1]);
		if (i != tx_size)
			fprintf(fdDestinationFile_ch0, "\n");
	}
	return false;
}

/*
 * printRxDataToFile(int16_t* rx_buffer, int rx_size)
 * Take rx_buffer and print it into destination file
 */
bool Stream::printRxDataToFile(int16_t* rx_buffer, int rx_size)
{
	if (fdDestinationFile_ch0 == NULL)
		return true;

	for (int i = 0; i < rx_size; i++)
	{
		fprintf(fdDestinationFile_ch0, "%d, %d", rx_buffer[2*i], rx_buffer[2*i+1]);
		if (i != rx_size)
			fprintf(fdDestinationFile_ch0, "\n");
	}
	return false;
}

// This functions was programmed at a very late stage of the project and is not tested!
int Stream::setIntpAndDeci(int interpolation, int decimation)
{
	if (txDev->devWriteParam(LMS7param(MAC), 3) ||
			txDev->devSetInterpolation(interpolation) || txDev->devSetDecimation(decimation))
	{
		printConsoleAndDebugLine("TX: could not deactivate Decimation and Interpolation.");
		return -1;
	}
	if (txDev->getId() != rxDev->getId())
	{
		if (rxDev->devWriteParam(LMS7param(MAC), 3) ||
				rxDev->devSetInterpolation(interpolation) || rxDev->devSetDecimation(decimation))
		{
			printConsoleAndDebugLine("RX: could not deactivate Decimation and Interpolation.");
			return -1;
		}
	}
	return 0;
}

/*
 * void *streamPause(void *args_)
 * This thread function is for the pause thread. This thread will wait for input
 * while both other threads will stream. If necessary, the thread will set a global
 * variable and therefore, pause the streaming.
 */
void *streamPause(void *args_)
{
	char cCmd[3];
	pauseThreadArgs *args = (pauseThreadArgs*) args_;
	cin.clear();
	while (true)
	{
		if (!pauseStream)
		{
			cin >> cCmd;
			cin.ignore();
			// Input is to pause the stream
			if (cCmd[0] == 112 || cCmd[0] == 80) //== p or P
			{
				args->dev_->toggleAGC(0, false);
				pauseStream = true;
			}
			if (cCmd[0] == 115) // "s" (Space)
			{
				send = true;
			}
			// Input is digit: Toggle AGC with this value.
			if (isdigit(cCmd[0]))
			{
				args->dev_->toggleAGC(atoi(cCmd), true);
			}
		}
		// Stream is paused, just yield.
		else
			this_thread::yield();
	}
	return (void*) 0;
}

