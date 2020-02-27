/* ==================================================================
 * title:		commands.cpp
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 * This file contains all the available commands used by the tool in the main routine.
 * Moved here to not blow up the main file.
 * D&C: this file uses no API Calls.
 * Attention: There is not much variable checking going on here.
 * Make sure that values are valid.
 * ==================================================================
 */

#include "commands.h"

/*
 * calibrate(deviceVector& deviceVec, int devID, float_type bandwidth, const char *dir, int channel)
 * This function will first call it self again for every device if devID is -1.
 * Also call itself with "tx" and "rx" if dir is "both".
 * Also if channel is -1, call itself for every channel.
 * Will call the calibration member function of device object.
 */
bool calibrate(deviceVector& deviceVec, int devID, float_type bandwidth, const char *dir, int channel)
{
	bool retVal, dir_tx = false;

	if (devID == -1)
	{
		for (int i = 0; i < (int)deviceVec.size(); i++)
		{
			if (calibrate(deviceVec, i, bandwidth, dir, channel))
				return true;
		}
		return false;
	}

	if (!strcmp(dir, "both"))
	{
		retVal = calibrate(deviceVec, devID, bandwidth, "tx", channel);
		if (retVal)
			return true;
		retVal = calibrate(deviceVec, devID, bandwidth, "rx", channel);
		if (retVal)
			return true;

		return false;
	}

	if (!strcmp(dir, "tx"))
		dir_tx = true;
	if (!strcmp(dir, "rx"))
		dir_tx = false;

	for (const auto& device : deviceVec)
	{
		if (devID == device->getId())
		{
			if (channel == -1)
			{
				int numCh = device->getNumChannels();
				for (int i = 0; i < numCh; i++)
				{
					retVal = device->devCalibrate(dir_tx, bandwidth*1e6, i);
					if (retVal)
						return true;
				}
				return false;
			}
			else
			{
				return device->devCalibrate(dir_tx, bandwidth*1e6, channel);
			}
		}
	}
	printConsoleAndDebugLine("Device ID not found.");
	return true;
}

/*
 * int connect(deviceVector& deviceVec, int *nConnected)
 * Will close all active connections and open connection to nConnected devices.
 * Device objects will be saved into deviceVec. Return number of opened devices.
 */
int connect(deviceVector& deviceVec, int nConnected)
{
	int nOpened = 0;

	printConsoleAndDebugLine("Found number of devices: ", nConnected);

	lms_info_str_t* list = new lms_info_str_t[nConnected];

	if (LMS_GetDeviceList(list) < 0)
	{
		printConsoleAndDebugLine("Could not populate device list.");
		delete [] list;
		return false;
	}

	disconnect(deviceVec);

	for (int i = 0; i < nConnected; i++)
	{
		printDebugLine(list[i], i);
		Device *tmpDev = new Device(i, list[i]);
		if (tmpDev->getId() == -1)
		{
			printConsoleAndDebugLine("Error opening devices.");
			disconnect(deviceVec);
			return -1;
		}
		deviceVec.push_back(tmpDev);
		nOpened++;
	}

	delete []list;

	printOpenedDevices(deviceVec);
	return nOpened;
}

/*
 * constellation(deviceVector& deviceVec, int devID, const char *constel)
 * This function will change the constellation object for a specific device ID (-1: all).
 */
bool constellation(deviceVector& deviceVec, int devID, const char *constel)
{
	int constelID;

	if (devID == -1)
	{
		for (int i = 0; i < (int)deviceVec.size(); i++)
		{
			if (constellation(deviceVec, i, constel))
				return true;
		}
		return false;
	}

	if (!strcmp(constel, "bpsk"))
		constelID = bpskID;
	else if (!strcmp(constel, "qpsk"))
		constelID = qpskID;
	else
	{
		printConsoleAndDebugLine("Constellation not found.");
		return true;
	}

	for (const auto& device : deviceVec)
	{
		if (devID == device->getId())
		{
			return device->changeConstellation(constelID);
		}
	}
	printConsoleAndDebugLine("Device ID not found.");
	return true;
}

/*
 * disconnect(deviceVector& deviceVec)
 * This function will delete all devices in the device Vector, therefore also
 * calling LMS_Close indirectly.
 */
void disconnect(deviceVector& deviceVec)
{
	if (!deviceVec.empty())
	{
		for (auto it = deviceVec.begin(); it != deviceVec.end();)
		{
			auto device = *it;
			device->devPrintInfoNoConsole();
			device->~Device();
			deviceVec.erase(it);

			if (deviceVec.empty())
				break;
			else
				it = deviceVec.begin();
		}
	}
}

/*
 * enable(deviceVector& deviceVec, int devID, bool en, const char *dir, int channel)
 * This function will first call it self again for every device if devID is -1.
 * Also call itself with "tx" and "rx" if dir is "both".
 * Also if channel is -1, call itself for every channel.
 * Will enable (en = true) or disable the specified channel.
 */
bool enable(deviceVector& deviceVec, int devID, bool en, const char *dir, int channel)
{
	bool retVal, dir_tx = false;

	if (devID == -1)
	{
		for (int i = 0; i < (int)deviceVec.size(); i++)
		{
			if (enable(deviceVec, i, en, dir, channel))
				return true;
		}
		return false;
	}

	if (!strcmp(dir, "both"))
	{
		retVal = enable(deviceVec, devID, en, "tx", channel);
		if (retVal)
			return true;
		retVal = enable(deviceVec, devID, en, "rx", channel);
		if (retVal)
			return true;

		return false;
	}

	if (!strcmp(dir, "tx"))
		dir_tx = true;
	if (!strcmp(dir, "rx"))
		dir_tx = false;

	for (const auto& device : deviceVec)
	{
		if (devID == device->getId())
		{
			if (channel == -1)
			{
				int numCh = device->getNumChannels(); // Should be the same for tx and rx, so just pass true.
				for (int i = 0; i < numCh; i++)
				{
					retVal = device->devEnable(dir_tx, en, i);
					if (retVal)
						return true;
				}
				return false;
			}
			else
			{
				return device->devEnable(dir_tx, en, channel);
			}
		}
	}
	printConsoleAndDebugLine("Device ID not found.");
	return true;
}

/*
 * getAntennaPorts(deviceVector& deviceVec, int devID, int channel)
 * This function will first call it self again for every device if devID is -1.
 * Also if channel is -1, call itself for every channel.
 * Will call the device member function to get the antenna ports string and print this text.
 */
bool getAntennaPorts(deviceVector& deviceVec, int devID, int channel)
{
	string toPrint;

	if (devID == -1)
	{
		for (int i = 0; i < (int)deviceVec.size(); i++)
		{
			if (getAntennaPorts(deviceVec, i, channel))
				return true;
		}
		return false;
	}

	for (const auto& device : deviceVec)
	{
		if (devID == device->getId())
		{
			if (channel == -1)
			{
				int numCh = device->devGetNumChannels(true); // Should be the same for tx and rx, so just pass true.
				for (int i = 0; i < numCh; i++)
				{
					toPrint = device->devGetAntennaPorts(i);
					if (toPrint.empty())
						return true;
					else
						printConsoleAndDebugLine(toPrint.c_str());
				}
				return false;
			}
			else
			{
				toPrint = device->devGetAntennaPorts(channel);
				if (toPrint.empty())
					return true;
				else
				{
					printConsoleAndDebugLine(toPrint.c_str());
					return false;
				}
			}
		}
	}
	printConsoleAndDebugLine("Device ID not found.");
	return true;
}

/*
 * getGain(deviceVector& deviceVec, int devID, int channel)
 * This function will first call it self again for every device if devID is -1.
 * Also if channel is -1, call itself for every channel.
 * Will call the device member function to get the gain value string and print this text
 */
bool getGain(deviceVector& deviceVec, int devID, int channel)
{
	string toPrint;

	if (devID == -1)
	{
		for (int i = 0; i < (int)deviceVec.size(); i++)
		{
			if (getGain(deviceVec, i, channel))
				return true;
		}
		return false;
	}

	for (const auto& device : deviceVec)
	{
		if (devID == device->getId())
		{
			if (channel == -1)
			{
				int numCh = device->devGetNumChannels(true); // Should be the same for tx and rx, so just pass true.
				for (int i = 0; i < numCh; i++)
				{
					toPrint = device->devGetGain(i);
					if (toPrint.empty())
						return true;
					else
						printConsoleAndDebugLine(toPrint.c_str());
				}
				return false;
			}
			else
			{
				toPrint = device->devGetGain(channel);
				if (toPrint.empty())
					return true;
				else
				{
					printConsoleAndDebugLine(toPrint.c_str());
					return false;
				}
			}
		}
	}
	printConsoleAndDebugLine("Device ID not found.");
	return true;
}

/*
 * getLOFreq(deviceVector& deviceVec, int devID, int channel)
 * This function will first call it self again for every device if devID is -1.
 * Also if channel is -1, call itself for every channel.
 * Will call the device member function to get the LO frequency string and print this text
 */
bool getLOFreq(deviceVector& deviceVec, int devID, int channel)
{
	string toPrint;

	if (devID == -1)
	{
		for (int i = 0; i < (int)deviceVec.size(); i++)
		{
			if (getLOFreq(deviceVec, i, channel))
				return true;
		}
		return false;
	}

	for (const auto& device : deviceVec)
	{
		if (devID == device->getId())
		{
			if (channel == -1)
			{
				int numCh = device->devGetNumChannels(true); // Should be the same for tx and rx, so just pass true.
				for (int i = 0; i < numCh; i++)
				{
					toPrint = device->devGetLOFreq(i);
					if (toPrint.empty())
						return true;
					else
						printConsoleAndDebugLine(toPrint.c_str());
				}
				return false;
			}
			else
			{
				toPrint = device->devGetLOFreq(channel);
				if (toPrint.empty())
					return true;
				else
				{
					printConsoleAndDebugLine(toPrint.c_str());
					return false;
				}
			}
		}
	}
	printConsoleAndDebugLine("Device ID not found.");
	return true;
}

/*
 * getLPBW(deviceVector& deviceVec, int devID, int channel)
 * This function will first call it self again for every device if devID is -1.
 * Also if channel is -1, call itself for every channel.
 * Will call the device member function to get the LP bandwidth string and print this text
 */
bool getLPBW(deviceVector& deviceVec, int devID, int channel)
{
	string toPrint;

	if (devID == -1)
	{
		for (int i = 0; i < (int)deviceVec.size(); i++)
		{
			if (getLPBW(deviceVec, i, channel))
				return true;
		}
		return false;
	}

	for (const auto& device : deviceVec)
	{
		if (devID == device->getId())
		{
			if (channel == -1)
			{
				int numCh = device->devGetNumChannels(true); // Should be the same for tx and rx, so just pass true.
				for (int i = 0; i < numCh; i++)
				{
					toPrint = device->devGetLPBW(i);
					if (toPrint.empty())
						return true;
					else
						printConsoleAndDebugLine(toPrint.c_str());
				}
				return false;
			}
			else
			{
				toPrint = device->devGetLPBW(channel);
				if (toPrint.empty())
					return true;
				else
				{
					printConsoleAndDebugLine(toPrint.c_str());
					return false;
				}
			}
		}
	}
	printConsoleAndDebugLine("Device ID not found.");
	return true;
}

/*
 * getSamplingRate(deviceVector& deviceVec, int devID, int channel)
 * This function will first call it self again for every device if devID is -1.
 * Also if channel is -1, call itself for every channel.
 * Will call the device member function to get the sampling rate string and print this text
 */
bool getSamplingRate(deviceVector& deviceVec, int devID, int channel)
{
	string toPrint;

	if (devID == -1)
	{
		for (int i = 0; i < (int)deviceVec.size(); i++)
		{
			if (getSamplingRate(deviceVec, i, channel))
				return true;
		}
		return false;
	}

	for (const auto& device : deviceVec)
	{
		if (devID == device->getId())
		{
			if (channel == -1)
			{
				int numCh = device->devGetNumChannels(true); // Should be the same for tx and rx, so just pass true.
				for (int i = 0; i < numCh; i++)
				{
					toPrint = device->devGetSamplingRate(i);
					if (toPrint.empty())
						return true;
					else
						printConsoleAndDebugLine(toPrint.c_str());
				}
				return false;
			}
			else
			{
				toPrint = device->devGetSamplingRate(channel);
				if (toPrint.empty())
					return true;
				else
				{
					printConsoleAndDebugLine(toPrint.c_str());
					return false;
				}
			}
		}
	}
	printConsoleAndDebugLine("Device ID not found.");
	return true;
}

/*
 * init(deviceVector& deviceVec, int devID)
 * Will initialize the device with standard values. If devID is -1, all devices will be init.
 */
void init(deviceVector& deviceVec, int devID)
{
	if (devID == -1)
	{
		for (int i = 0; i < (int)deviceVec.size(); i++)
		{
			init(deviceVec, i);
		}
		return;
	}

	for (const auto& device : deviceVec)
	{
		if (devID == device->getId())
		{
			device->devInit();
			return;
		}
	}
	printConsoleAndDebugLine("init: Device ID not found: ", devID);
}

/*
 * This function was planned, but never implemented.
 *
int playWaveform(deviceVector& deviceVec, int devID, const char *filename)
{
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) == NULL)
	{
		printConsoleAndDebugLine("Could not get current directory.");
		return -1;
	}

	strcat(cwd, "/");
	strcat(cwd, filename);

	fstream fsFile;
	fsFile.open(cwd, fstream::in);

	if (fsFile.fail())
	{
		printConsoleAndDebugLine("Could not open wfm file.");
		printConsoleAndDebugLine(cwd);
		return -1;
	}

	int16_t num;
	vector<int16_t> isamples;
	vector<int16_t> qsamples;

	while (!fsFile.eof())
	{
		num = 0;
		num |= (unsigned char)fsFile.get();
		num |= fsFile.get() << 8;
		isamples.push_back(num);

		num = 0;
		num |= (unsigned char)fsFile.get();
		num |= fsFile.get() << 8;
		qsamples.push_back(num);
	}

    complex16_t* src[globalNumChannels];

    for(int i=0; i<globalNumChannels; ++i)
    	src[i] = new lime::complex16_t[isamples.size()];

    for (int i = 0; i < isamples.size(); i++)
    {
    	for (int ch = 0; ch < globalNumChannels; ch++)
    	{
            src[ch][i].i = isamples[i];
            src[ch][i].q = qsamples[i];
    	}
    }

	for (const auto& device : deviceVec)
	{
		if (devID == device->getId())
		{
			//device->
			return 0;
		}
	}
	printConsoleAndDebugLine("wfm: Device ID not found: ", devID);
	return -1;

	/*
lime::complex16_t* src[chCount];
    for(int i=0; i<chCount; ++i)
        src[i] = new lime::complex16_t[isamples.size()];

    for(size_t i=0; i<isamples.size(); ++i)
    {
        for(int c=0; c<chCount; ++c)
        {
            if(c == 1 && !MIMO)
            {
                src[c][i].i = 0;
                src[c][i].q = 0;
            }
            else
            {
                src[c][i].i = isamples[i];
                src[c][i].q = qsamples[i];
            }
        }
    }

    int status = LMS_UploadWFM(lmsControl, (const void**)src, 2+cmbDevice->GetSelection()*2, isamples.size(), 1);

    progressBar->SetValue(progressBar->GetRange());
    lblProgressPercent->SetLabelText(_("100%"));

    LMS_EnableTxWFM(lmsControl, cmbDevice->GetSelection()*2, true);



}*/

/*
 * loadConfiguration(deviceVector& deviceVec, int devID, const char *filename)
 * Will first call itself for every device if devID is -1.
 * Then try to open the file specified by filename (path & filename) and load
 * the configuration into the device.
 */
bool loadConfiguration(deviceVector& deviceVec, int devID, const char *filename)
{
	bool fileExists;

	if (devID == -1)
	{
		for (int i = 0; i < (int)deviceVec.size(); i++)
		{
			if (loadConfiguration(deviceVec, i, filename))
				return true;
		}
		return false;
	}

	fileExists = access(filename, F_OK);
	if (fileExists)
	{
		printConsoleAndDebugLine("Can't access configuration file.");
		return true;
	}

	for (const auto& device : deviceVec)
	{
		if (devID == device->getId())
		{
			return device->devLoadConfig(filename);
		}
	}
	printConsoleAndDebugLine("Device ID not found.");
	return true;
}

/*
 * printConnectedDevices(int nConnected)
 * Poll for all connected devices and print their information
 */
void printConnectedDevices(int nConnected)
{
	lms_info_str_t* list = new lms_info_str_t[nConnected];

	if (LMS_GetDeviceList(list) < 0)
	{
		printConsoleAndDebugLine("Could not populate device list.");
		delete [] list;
		return;
	}

	printConsoleAndDebugLine("Connected devices:");
	for (int i = 0; i < nConnected; i++)
	{
		printConsoleAndDebugLine(list[i]);
	}
	delete []list;
}

/*
 * printHelp()
 * print all available commands into console.
 */
void printHelp()
{
	printConsoleLine("Available commands (case sensitive):");
	printConsoleLine("antenna:       Get / Set specific Antenna ports active.");
	printConsoleLine("calibrate:     Calibrate a device for a specified bandwidth.");
	printConsoleLine("connect:       Open to all connected devices. Already opened ones will be disconnected first.");
	printConsoleLine("constellation: Swap the constellation (modulation scheme) of a opened device.");
	printConsoleLine("devices:       List all connected and, if available, all opened devices.");
	printConsoleLine("disconnect:    Disconnect all opened devices.");
	printConsoleLine("enable:        Enable specific TX / RX channel.");
	printConsoleLine("exit / quit:   Will clean up and exit the program.");
	printConsoleLine("gain:          Get / Set relative TX or RX gain.");
	printConsoleLine("init:          Load opened devices with default configuration.");
	printConsoleLine("lo:            Get / Set LO Frequency.");
	printConsoleLine("load / save:   Load / Save a configuration file.");
	printConsoleLine("lpbw:          Configure low-pass bandwidth.");
	printConsoleLine("reset:         Resets opened devices.");
	printConsoleLine("sample:        Get / Set sampling rate.");
	printConsoleLine("stream:        Will set up a stream object and start stream procedure between user defined devices and \n"
			         "               with a defined test file. User can pause stream by pressing \"p\" and issue commands.");
	printConsoleLine("wfm:           Specify a device to send a waveform through the FPGA waveform player.");
	printConsoleLine("\nTip: Use -1 when prompted with deviceID, channel or similar to select all available.");
}

/*
 * printOpenedDevices(deviceVector& deviceVec)
 * Print out all opened devices saved in deviceVec.
 */
void printOpenedDevices(deviceVector& deviceVec)
{
	if (deviceVec.empty())
		return;

    printConsoleAndDebugLine("Opened devices:");
	for (const auto& device : deviceVec)
	{
		device->devPrintInfo();
	}
}

/*
 * reset(deviceVector& deviceVec, int devID)
 * First, call itself for every device if devID is -1.
 * Then call the reset member function of device class.
 */
void reset(deviceVector& deviceVec, int devID)
{
	if (devID == -1)
	{
		for (int i = 0; i < (int)deviceVec.size(); i++)
		{
			reset(deviceVec, i);
		}
		return;
	}

	for (const auto& device : deviceVec)
	{
		if (devID == device->getId())
		{
			device->devReset();
			return;
		}
	}
	printConsoleAndDebugLine("reset: Device ID not found: ", devID);

}

/*
 * saveConfiguration(deviceVector& deviceVec, int devID, const char *filename)
 * First, call itself for every device if devID is -1.
 * Check if the file already exists.
 * Then save the device configuration into the specified filename (path & filename).
 */
bool saveConfiguration(deviceVector& deviceVec, int devID, const char *filename)
{
	bool fileExists;

	fileExists = access(filename, F_OK);
	if (!fileExists)
	{
		printConsoleAndDebugLine("Configuration File already exists.");
		return true;
	}

	for (const auto& device : deviceVec)
	{
		if (devID == device->getId())
		{
			return device->devSaveConfig(filename);
		}
	}
	printConsoleAndDebugLine("Device ID not found.");
	return true;
}

/*
 * setAntennaPorts(deviceVector& deviceVec, int devID, const char *dir, int channel)
 * First, call itself for every device if devID is -1.
 * Next, call itself with "tx" and "rx" if direction is "both".
 * Next, call itself again for every channel if channel is -1.
 * Then set the antenna port ( will be polled inside the function).
 */
bool setAntennaPorts(deviceVector& deviceVec, int devID, const char *dir, int channel)
{
	bool retVal, dir_tx = false;

	if (devID == -1)
	{
		for (int i = 0; i < (int)deviceVec.size(); i++)
		{
			if (setAntennaPorts(deviceVec, i, dir, channel))
				return true;
		}
		return false;
	}

	if (!strcmp(dir, "both"))
	{
		retVal = setAntennaPorts(deviceVec, devID, "tx", channel);
		if (retVal)
			return true;
		retVal = setAntennaPorts(deviceVec, devID, "rx", channel);
		if (retVal)
			return true;

		return false;
	}

	retVal = getAntennaPorts(deviceVec, devID, channel);
	if (retVal)
		return true;

	if (!strcmp(dir, "tx"))
		dir_tx = true;
	if (!strcmp(dir, "rx"))
		dir_tx = false;

	int port;

	for (const auto& device : deviceVec)
	{
		if (devID == device->getId())
		{
			if (channel == -1)
			{
				int numCh = device->getNumChannels();
				for (int i = 0; i < numCh; i++)
				{
					cout << "Specify antenna port for channel "<< i << ".\n=>antenna=>set=>" << dir << "=>";
					cin >> port;
					cin.ignore();
					retVal = device->devSetAntennaPorts(i, dir_tx, port);
					if (retVal)
						return true;
				}
				return false;
			}
			else
			{
				cout << "Specify antenna port for channel "<< channel << ".\n=>antenna=>set=>" << dir << "=>";
				cin >> port;
				cin.ignore();

				return device->devSetAntennaPorts(channel, dir_tx, port);
			}
		}
	}
	printConsoleAndDebugLine("Device ID not found.");
	return true;
}

/*
 * setGain(deviceVector& deviceVec, int devID, unsigned int gain, const char *dir, int channel)
 * First, call itself for every device if devID is -1.
 * Next, call itself again for every channel if channel is -1.
 * Then set the gain value specified.
 */
bool setGain(deviceVector& deviceVec, int devID, const char *dir, int channel)
{
	bool retVal, dir_tx = false;
	unsigned int gain;

	if (devID == -1)
	{
		for (int i = 0; i < (int)deviceVec.size(); i++)
		{
			if (setGain(deviceVec, i, dir, channel))
				return true;
		}
		return false;
	}

	retVal = getGain(deviceVec, devID, channel);
	if (retVal)
		return true;

	if (!strcmp(dir, "tx"))
		dir_tx = true;
	if (!strcmp(dir, "rx"))
		dir_tx = false;

	for (const auto& device : deviceVec)
	{
		if (devID == device->getId())
		{
			if (channel == -1)
			{
				int numCh = device->getNumChannels();
				for (int i = 0; i < numCh; i++)
				{
					cout << "Specify desired gain in dB (value is relative) for channel " << i << ".\n=>gain=>set=>"
							<< dir << "=>";
					cin >> gain;
					cin.ignore();
					retVal = device->devSetGain(i, dir_tx, gain);
					if (retVal)
						return true;
				}
				return false;
			}
			else
			{
				cout << "Specify desired gain in dB (value is relative) for channel " << channel << ".\n=>gain=>set=>"
						<< dir << "=>";
				cin >> gain;
				cin.ignore();
				return device->devSetGain(channel, dir_tx, gain);
			}
		}
	}
	printConsoleAndDebugLine("Device ID not found.");
	return true;
}

/*
 * setLOFreq(deviceVector& deviceVec, int devID, float_type freq, const char *dir, int channel)
 * First, call itself for every device if devID is -1.
 * Next, call itself with "tx" and "rx" if direction is "both".
 * Next, call itself again for every channel if channel is -1.
 * Then set the LO frequency value specified. (MHz value devided by 1e6)
 */
bool setLOFreq(deviceVector& deviceVec, int devID, const char *dir, int channel)
{
	bool retVal, dir_tx = false;
	float_type freq;

	if (devID == -1)
	{
		for (int i = 0; i < (int)deviceVec.size(); i++)
		{
			if (setLOFreq(deviceVec, i, dir, channel))
				return true;
		}
		return false;
	}

	if (!strcmp(dir, "both"))
	{
		retVal = setLOFreq(deviceVec, devID, "tx", channel);
		if (retVal)
			return true;
		retVal = setLOFreq(deviceVec, devID, "rx", channel);
		if (retVal)
			return true;

		return false;
	}

	retVal = getLOFreq(deviceVec, devID, channel);
	if (retVal)
		return true;

	if (!strcmp(dir, "tx"))
		dir_tx = true;
	if (!strcmp(dir, "rx"))
		dir_tx = false;

	for (const auto& device : deviceVec)
	{
		if (devID == device->getId())
		{
			if (channel == -1)
			{
				int numCh = device->getNumChannels();
				for (int i = 0; i < numCh; i++)
				{
					cout << "Specify desired LO frequency in MHz for channel " << i <<
							".\n=>lo=>set=>" << dir << "=>";
					cin >> freq;
					cin.ignore();
					retVal = device->devSetLOFreq(i, dir_tx, freq*1e6);
					if (retVal)
						return true;
				}
				return false;
			}
			else
			{
				cout << "Specify desired LO frequency in MHz for channel " << channel <<
						".\n=>lo=>set=>" << dir << "=>";
				cin >> freq;
				cin.ignore();
				return device->devSetLOFreq(channel, dir_tx, freq*1e6);
			}
		}
	}
	printConsoleAndDebugLine("Device ID not found.");
	return true;
}

/*
 * setLPBW(deviceVector& deviceVec, int devID, float_type bandwidth, const char * dir, int channel)
 * First, call itself for every device if devID is -1.
 * Next, call itself with "tx" and "rx" if direction is "both".
 * Next, call itself again for every channel if channel is -1.
 * Then set the LP bandwidth value specified. (MHz value devided by 1e6)
 */
bool setLPBW(deviceVector& deviceVec, int devID, const char * dir, int channel)
{
	bool retVal, dir_tx = false;
	float_type bandwidth;

	if (devID == -1)
	{
		for (int i = 0; i < (int)deviceVec.size(); i++)
		{
			if (setLPBW(deviceVec, i, dir, channel))
				return true;
		}
		return false;
	}

	if (!strcmp(dir, "both"))
	{
		retVal = setLPBW(deviceVec, devID, "tx", channel);
		if (retVal)
			return true;
		retVal = setLPBW(deviceVec, devID, "rx", channel);
		if (retVal)
			return true;

		return false;
	}

	retVal = getLPBW(deviceVec, devID, channel);
	if (retVal)
		return true;

	if (!strcmp(dir, "tx"))
		dir_tx = true;
	if (!strcmp(dir, "rx"))
		dir_tx = false;

	for (const auto& device : deviceVec)
	{
		if (devID == device->getId())
		{
			if (channel == -1)
			{
				int numCh = device->getNumChannels();
				for (int i = 0; i < numCh; i++)
				{
					cout << "Specify bandwidth in MHz (0 to exit) for channel " << i <<
							".\n=>lpbw=>set=>" << dir << "=>";
					cin >> bandwidth;
					cin.ignore();

					if (bandwidth == 0)
						return false;

					retVal = device->devSetLPBW(i, dir_tx, bandwidth*1e6);
					if (retVal)
						return true;
				}
				return false;
			}
			else
			{
				cout << "Specify bandwidth in MHz (0 to exit) for channel " << channel <<
						".\n=>lpbw=>set=>" << dir << "=>";
				cin >> bandwidth;
				cin.ignore();

				if (bandwidth == 0)
					return false;

				return device->devSetLPBW(channel, dir_tx, bandwidth*1e6);
			}
		}
	}
	printConsoleAndDebugLine("Device ID not found.");
	return true;
}

/*
 * setSamplingRate(deviceVector& deviceVec, int devID, float_type rate, int sampling, const char *dir)
 * First, call itself for every device if devID is -1.
 * Next, call itself with "tx" and "rx" if direction is "both".
 * Next, call itself again for every channel if channel is -1.
 * Then set the sampling rate (rate, in MHz devided by 1e6) of the host and the sampling of ADC/DAC.
 */
bool setSamplingRate(deviceVector& deviceVec, int devID, const char *dir)
{
	bool retVal, dir_tx = false;
	float_type rate;
	int sampling;

	if (devID == -1)
	{
		for (int i = 0; i < (int)deviceVec.size(); i++)
		{
			if (setSamplingRate(deviceVec, i, dir))
				return true;
		}
		return false;
	}

	if (!strcmp(dir, "both"))
	{
		retVal = setSamplingRate(deviceVec, devID, "tx");
		if (retVal)
			return true;
		retVal = setSamplingRate(deviceVec, devID, "rx");
		if (retVal)
			return true;

		return false;
	}

	retVal = getSamplingRate(deviceVec, devID, -1);
	if (retVal)
		return true;

	if (!strcmp(dir, "tx"))
		dir_tx = true;
	if (!strcmp(dir, "rx"))
		dir_tx = false;

	for (const auto& device : deviceVec)
	{
		if (devID == device->getId())
		{
			cout << "Specify host sampling rate in MHz.\n=>sample=>" << dir << "=>";
			cin >> rate;
			cin.ignore();

			cout << "Specify preferred ADC oversampling in RF.\n=>sample=>" << dir << "=>";
			cin >> sampling;
			cin.ignore();

			return device->devSetSamplingRate(rate*1e6, sampling, dir_tx);
		}
	}
	printConsoleAndDebugLine("Device ID not found.");
	return true;
}

