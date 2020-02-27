/* ==================================================================
 * title:		randomStream.cpp
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 * These functions should transmit a random symbol stream of 9 symbols.
 * However, it never worked well. Only one device was available, so since
 * the device class was not thread safe by then, lot's of problems occurred.
 * Also, the function startRandomStream creates two threads, which was not
 * necessary at all.
 * One thing was positive: writing this code teached me a lot of things.
 * ==================================================================
 */

#include <randomStream.h>

// For synchronization, very basic since we don't need more for now.
bool txReady, rxReady;

/*
 * startRandomStream(deviceVector& deviceVec, int destID, int sourceID)
 * This function will create two separate threads, tx and rx. It will pass both threads
 * a pointer to the tx and rx device (depending on the passed IDs. They can also be the same).
 */
bool startRandomStream(deviceVector& deviceVec, int destID, int sourceID)
{
	if (deviceVec.empty())
		return true;

	printConsoleAndDebugLine("WARNING: Deprecated function. Will most likely crash or not work as intended.");

	bool foundTX = false, foundRX = false;
	arguments thTxArgs, thRxArgs;

	for (const auto& device : deviceVec)
	{
		if (destID == device->getId())
		{
			foundRX = true;
			thRxArgs.dev_ = device;
		}
		if (sourceID == device->getId())
		{
			foundTX = true;
			thTxArgs.dev_ = device;
		}
	}

	if (!(foundRX & foundTX))
	{
		printConsoleAndDebugLine("Stream: At least one of the specified devices is not in the device list.");
		return true;
	}

	pthread_t rxThread, txThread;
	void *vretRX, *vretTX;

	if (pthread_create(&txThread, NULL, streamRandomTX, (void*)&thTxArgs))
	{
		printConsoleAndDebugLine("Failed to create TX thread.");
		return true;
	}
	if (pthread_create(&rxThread, NULL, streamRandomRX, (void*)&thRxArgs))
	{
		printConsoleAndDebugLine("Failed to create RX thread.");
		return true;
	}
	pthread_join(txThread, &vretTX);
	pthread_join(rxThread, &vretRX);

	printConsoleAndDebugLine("Parent: Children are done streaming, will return.");

	return false;
}

/*
 * streamRandomRX(void *args_)
 * The thread function for RX. Pass it the argument struct with a pointer to the device object, see header.
 */
void *streamRandomRX(void *args_)
{
	printConsoleAndDebugLine("Hello, I am RX. I will try to receive samples. Let's go!");
	rxReady = false;

	arguments *args = (arguments*) args_;
#ifdef USE_GNU_PLOT
	GNUPlotPipe gpprx;
#endif

	// Set up stream object, configure device is in TX thread!
	int numChannels = args->dev_->getNumChannels();
	lms_stream_t rx_stream[numChannels];
	for (int i = 0; i < numChannels; i++)
	{
		rx_stream[i].channel = i;
		rx_stream[i].fifoSize = 16 * 1024;
		rx_stream[i].throughputVsLatency = 0.5;
		rx_stream[i].dataFmt = lms_stream_t::LMS_FMT_I16;
		rx_stream[i].isTx = false;

		if (args->dev_->devSetupStream(&rx_stream[i]))
		{
			printConsoleAndDebugLine("RX: Could not setup stream.", i);
			return (void*) -1;
		}
		// Device class is not thread safe yet (31.08.2018). So call these in TX
//		if (args->dev_->devEnable(LMS_CH_RX, true, i) ||
////				args->dev_->devSetAntennaPorts(i, LMS_CH_RX, 1) ||
//				args->dev_->devSetSamplingRate(4*1e6, 8*1e6, LMS_CH_RX) ||
//				args->dev_->devSetLOFreq(i, LMS_CH_RX, 2450*1e6) ||
//				args->dev_->devSetLPBW(i, LMS_CH_RX, 10*1e6))// ||
////				args->dev_->devCalibrate(LMS_CH_RX, 5*1e6, i))
//		{
//			printConsoleAndDebugLine("RX: Could not set up device.", i);
//			return (void*) -1;
//		}
	}

	// Get filename.
	string filename = createTestFilename("rx", "random");
	if (filename.empty())
	{
		printConsoleAndDebugLine("RX: Could not set up test file.");
		return (void*) -1;
	}

	FILE* fdRX = fopen(filename.c_str(), "w");
	if (fdRX == NULL)
	{
	    printConsoleAndDebugLine("RX: Open File failed.");
	    printConsoleAndDebugLine(filename.c_str());
		return (void*) -1;
	}

	// Set up variable to receive
    const int rx_size = 1024*4;
    int16_t rx_buffer[2*rx_size];

	//lms_stream_meta_t rx_meta[numChannels];
	lms_stream_status_t rx_status[numChannels];

	// Set up GNU Plot
#ifdef USE_GNU_PLOT
//	// normalized
//    gpp.write("set size square\n set xrange[-1.1:1.1]\n set yrange[-1.1:1.1]\n set grid\n"
    // or maximized
    gpprx.write("set size square\n set xrange[-33000:33000]\n set yrange[-33000:33000]\n set grid\n"
    		" set title 'I/Q constellation rx'\n set xlabel 'I'\n set ylabel 'Q'\n");
#endif

    // Wait for TX
    auto t1 = chrono::high_resolution_clock::now();
    while (!txReady)
    {
    	// Wait for a maximum of 10 seconds.
    	if (chrono::high_resolution_clock::now() - t1 > chrono::seconds(10))
    	{
    	    printConsoleAndDebugLine("TX: Timeout.");
    		return (void*) -1;
    	}
    	this_thread::yield();
    }

	// Start stream thread.
	for (int i = 0; i < numChannels; i++)
	{
		args->dev_->devStartStream(&rx_stream[i]);
		//rx_meta[i].waitForTimestamp = true;
		//rx_meta[i].flushPartialPacket = true;
	}

    // Notify TX
    rxReady = true;

	int samplesRead;
    t1 = chrono::high_resolution_clock::now();
    // Do until tx exits
    while(txReady)
    {
    	for (int i = 0; i < numChannels; i++)
    	{
    		samplesRead = args->dev_->devReceiveStream(&rx_stream[i], rx_buffer, rx_size, NULL, 1000);

    	    for (int i = 0; i <rx_size; i++) {
    	        fprintf(fdRX, "%d, %d, %d\n", i, rx_buffer[2*i], rx_buffer[2*i+1]);
    	    }
    	    // Plot every 1 second.
#ifdef USE_GNU_PLOT
    	    if ((chrono::high_resolution_clock::now() - t1 > chrono::seconds(1))
    	    		&& samplesRead) {
    	    	gpprx.write("plot '-' with points title 'rx'\n");
    	    	for (int j = 0; j < samplesRead; ++j)
    	    		gpprx.writef("%i %i\n", rx_buffer[2 * j], rx_buffer[2 * j + 1]);
    	    	gpprx.write("e\n");
    	    	gpprx.flush();
    	    	t1 = chrono::high_resolution_clock::now();
    	    }
#endif
    	}
    }

    // Finished, stop stream
	for (int i = 0; i < numChannels; i++)
	{
		args->dev_->devStopStream(&rx_stream[i]);
		args->dev_->devDestroyStream(&rx_stream[i]);
	}

    printConsoleAndDebugLine("RX done, bye.");
    fclose(fdRX);
	return (void*) 0;
}

/*
 * streamRandomTX(void *args_)
 * The thread function for TX. Pass it the argument struct with a pointer to the device object, see header.
 */
void *streamRandomTX(void *args_)
{
	printConsoleAndDebugLine("Hello, I am TX. I will create some random source, map it to 9 symbols and transmit them. Let's go!");
	txReady = false;

	// Let's hope this gets called before RX start's setting up.
	arguments *args = (arguments*) args_;
	args->dev_->devReset();
	args->dev_->devInit();
	cout << "dtx";

#ifdef USE_GNU_PLOT
	GNUPlotPipe gpp;
#endif

	// Set up stream object, configure device.
	int numChannels = args->dev_->getNumChannels();
	lms_stream_t tx_stream[numChannels];
	for (int i = 0; i < numChannels; i++)
	{
		tx_stream[i].channel = i;
		tx_stream[i].fifoSize = 16 * 1024;
		tx_stream[i].throughputVsLatency = 0.5;
		tx_stream[i].dataFmt = lms_stream_t::LMS_FMT_I16;
		tx_stream[i].isTx = true;

		if (args->dev_->devSetupStream(&tx_stream[i]))
		{
			printConsoleAndDebugLine("TX: Could not setup stream.", i);
			return (void*) -1;
		}
		if (args->dev_->devEnable(LMS_CH_TX, true, i) ||
//				args->dev_->devSetAntennaPorts(i, LMS_CH_TX, 1) ||
				args->dev_->devSetSamplingRate(4*1e6, 4, LMS_CH_TX) ||
				args->dev_->devSetLOFreq(i, LMS_CH_TX, 2450*1e6) ||
				args->dev_->devSetLPBW(i, LMS_CH_TX, 10*1e6))// ||
//				args->dev_->devCalibrate(LMS_CH_TX, 5*1e6, i))
		{
			printConsoleAndDebugLine("TX: Could not set up device.", i);
			return (void*) -1;
		}
		// RX since Device is not yet thread safe
		if (args->dev_->devEnable(LMS_CH_RX, true, i) ||
//				args->dev_->devSetAntennaPorts(i, LMS_CH_RX, 1) ||
				args->dev_->devSetSamplingRate(4 * 1e6, 4, LMS_CH_RX) ||
				args->dev_->devSetLOFreq(i, LMS_CH_RX, 2450 * 1e6) ||
				args->dev_->devSetLPBW(i, LMS_CH_RX, 10 * 1e6))	// ||
//				args->dev_->devCalibrate(LMS_CH_RX, 5*1e6, i))
		{
			printConsoleAndDebugLine("RX: Could not set up device.", i);
			return (void*) -1;
		}
	}

	// Get filename.
	string filename = createTestFilename("tx", "random");
	if (filename.empty())
	{
		printConsoleAndDebugLine("TX: Could not set up test file.");
		return (void*) -1;
	}

	FILE* fdTX = fopen(filename.c_str(), "w");
	if (fdTX == NULL)
	{
	    printConsoleAndDebugLine("TX: Open File failed.");
	    printConsoleAndDebugLine(filename.c_str());
		return (void*) -1;
	}

	// Set up variable to transmit
    const int tx_size = 1024*4;
    int16_t tx_buffer[2*tx_size];
    for (int i = 0; i <tx_size; i++) {
        tx_buffer[2*i] = 0x7fff*((int16_t) random() / 0x4fff); // I Part
        tx_buffer[2*i+1] = 0x7fff*((int16_t) random() / 0x4fff); // Q Part
//        tx_buffer[2*i] = (int16_t) random(); // I Part
//        tx_buffer[2*i+1] = (int16_t) random(); // Q Part
    	//tx_buffer[2*i] = (int16_t) 0x7fff*pow((-1), (i)); // I Part
    	//tx_buffer[2*i+1] = (int16_t) 0; // Q Part
        fprintf(fdTX, "%d, %d, %d\n", i, tx_buffer[2*i], tx_buffer[2*i+1]);
    }
    fclose(fdTX);

	//lms_stream_meta_t tx_meta[numChannels];
	lms_stream_status_t tx_status[numChannels];

	// Set up GNU Plot
#ifdef USE_GNU_PLOT
	// normalized
    gpp.write("set size square\n set xrange[-1.1:1.1]\n set yrange[-1.1:1.1]\n set grid\n"
    // or maximized
//    gpp.write("set size square\n set xrange[-33000:33000]\n set yrange[-33000:33000]\n set grid\n"
    		" set title 'I/Q constellation tx'\n set xlabel 'I'\n set ylabel 'Q'\n");
#endif

    // Notify RX
    txReady = true;
    // Wait for RX
    auto t1 = chrono::high_resolution_clock::now();
    while (!rxReady)
    {
    	// Wait for a maximum of 10 seconds.
    	if (chrono::high_resolution_clock::now() - t1 > chrono::seconds(10))
    	{
    	    printConsoleAndDebugLine("TX: Timeout.");
    		return (void*) -1;
    	}
    	this_thread::yield();
    }
    // Yield one more time to give RX time to prepare.
	this_thread::yield();

	// Start stream thread.
	for (int i = 0; i < numChannels; i++)
	{
		args->dev_->devStartStream(&tx_stream[i]);
		//tx_meta[i].waitForTimestamp = true;
		//tx_meta[i].flushPartialPacket = true;
	}

	args->dev_->toggleAGC(8, true);
	int samplesSent;
    t1 = chrono::high_resolution_clock::now();
    // Do for 10 seconds.
    while (chrono::high_resolution_clock::now() - t1 < chrono::seconds(10))
    {
    	for (int i = 0; i < numChannels; i++)
    	{
    		samplesSent = args->dev_->devSendStream(&tx_stream[i], tx_buffer, tx_size, NULL, 1000);
//        	args->dev_->devGetStreamStatus(&tx_stream[i], &tx_status[i]);
//        	cout << "TX data rate: " << tx_status[i].linkRate / 1e6 << " MB/s. Timestamp: " << tx_status[i].timestamp <<
//        			", channel: " << i << ".\n"; //link data rate
//    		cout << "TX Meta: " << tx_meta[i].timestamp << ", channel: " << i << ".\n";
    	}
    }

	args->dev_->toggleAGC(1, false);
    // Finished, stop stream
	for (int i = 0; i < numChannels; i++)
	{
		args->dev_->devStopStream(&tx_stream[i]);
		args->dev_->devDestroyStream(&tx_stream[i]);
	}

    printConsoleAndDebugLine("TX done, bye.");
    // Notify RX thread
    txReady = false;
    return (void*) 0;
}

/*
 * createTestFilename(const char* dir, const char *constellation)
 * Will try to set up a folder called "tests" in the local directory.
 * Then construct a filename existing of constellation (f.e. "qpsk"), direction
 * and the current date/time.
 * Returns empty string on error.
 */
string createTestFilename(const char* dir, const char *constellation)
{
	// Set up folder & get current location
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) == NULL)
	{
		printConsoleAndDebugLine("Could not get current directory.");
		return "";
	}
	int ret = mkdir(strcat(cwd, "/tests"), S_IRWXU | S_IRWXG | S_IRWXO);
	if (ret != 0 || ret != EEXIST)
	{
		// Somehow mkdir returns -1 even if EEXIST should be returned.
		// So this line will be called every time.
		printDebugLine("Could not create directory \"/tests\".");
	}
	// Get current date and time for filename.
	time_t timer;
	struct tm * timeinfo;
	timer = time(NULL);
	timeinfo = localtime(&timer);
	char sTime[64];
	strftime(sTime,sizeof(sTime),"_%d-%m_%H-%M-%S.txt",timeinfo);

	string filename = cwd;
	filename += "/";
	filename += constellation;
	filename += dir;
	filename += sTime;

	return filename;
}



