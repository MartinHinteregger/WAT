/* ==================================================================
 * title:		WAT.cpp
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 * This is a console based program to control LimeSDRs. It uses available code where available.
 * LimeSuite, gnuplot, ...
 * ==================================================================
 */

#include "WAT.h"

/*
 * main(int argc, char** argv)
 * This function will wait in a endless loop for commands via the console.
 */
int main(int argc, char** argv)
{
	string sCmd;
	deviceVector deviceVec;

	int nCmd, nConnected = 0, nOpened = 0;
	bool quit, retVal, en, firstrun = true;
	int destID, sourceID, channel;
	float_type bandwidth;

	cout << "WAT version " << swVersion << ", will init and start prompt.\n";
	cout << "Warning: Most inputs will not be checked for type or validity.\n";

	// Init the logger
	if (debug_logger())
	{
		cout << "Initialization of logger failed.\n";
		return -1;
	}

	printDebugLine("Initialization complete, will start prompt loop.");

	quit = false;
	while(!quit)
	{
		// On the first run: check if devices are already connected and if these
		// connected devices are already opened.
		if (firstrun)
		{
			nConnected = LMS_GetDeviceList(NULL);
			if (nConnected > 0)
			{
				nOpened = connect(deviceVec, nConnected);
				if (nOpened == -1)
					nOpened = 0;
			}
			firstrun = false;
		}
		// Prompt
		cin.clear();
		cout << "=>";
		getline(cin, sCmd);
		printDebugLine(sCmd.c_str());

		// Check if command was valid, i.e. it is in the list
		try
		{
			nCmd = -1;
			for (int i = 0; i < (int)NUMBEROFCOMMANDS; i++)
			{
				if (!sCmd.compare(commands[i]))
				{
					nCmd = i;
					break;
				}
			}
			if (nCmd == -1)
			{
				printConsoleLine("Command not understood.");
				continue;
			}
		}
		catch (exception& E)
		{
			printDebugLine(E.what());
			break;
		}

		// First, commands that don't need connected devices.
		switch (nCmd) {
		case EXIT:
			quit = true;
			continue;
		case HELP:
			printHelp();
			continue;
		case QUIT:
			quit = true;
			continue;
		default:
			break;
		}

		// Check for connected devices.
		nConnected = LMS_GetDeviceList(NULL);
		if (nConnected <= 0)
		{
			printConsoleAndDebugLine("No connected devices.");
			continue;
		}

		// Next, all commands that need connected, but not opened devices.
		switch (nCmd) {
		case CONNECT:
			nOpened = connect(deviceVec, nConnected);
			if (nOpened == -1)
				nOpened = 0;
			continue;
		case DEVICES:
			printConnectedDevices(nConnected);
			if (deviceVec.empty())
				printConsoleAndDebugLine("No opened devices.");
			else
				printOpenedDevices(deviceVec);
			continue;
		default:
			break;
		}

		// Check for opened devices
		if (deviceVec.empty())
		{
			printConsoleAndDebugLine("No opened devices.");
			nOpened = 0;
			continue;
		}

		retVal = checkIfOpenedStillConnected(deviceVec, nConnected);
		if (retVal)
		{
			printConsoleAndDebugLine("Error: At least one of the opened devices is no longer connected.");
			printConsoleAndDebugLine("Consider calling connect again.");
			continue;
		}

		// Lastly, all commands that need opened devices.
		switch (nCmd) {
		case ANTENNA:
			cout << "Get/Set antenna ports? (get, set)\n=>antenna=>";
			getline(cin, sCmd);

			cout << "Specify device ID.\n=>antenna=>";
			cin >> destID;
			cin.ignore();

			cout << "Specify Channel (0/1).\n=>antenna=>";
			cin >> channel;
			cin.ignore();

			if (!strcmp(sCmd.c_str(), "get"))
			{
				retVal = getAntennaPorts(deviceVec, destID, channel);
				if(retVal)
					printConsoleAndDebugLine("Could not get antenna ports.");
			}
			else if (!strcmp(sCmd.c_str(), "set"))
			{
				cout << "TX, RX or both? (tx, rx, both)\n=>antenna=>set=>";
				getline(cin, sCmd);

				retVal = setAntennaPorts(deviceVec, destID, sCmd.c_str(), channel);
				if(retVal)
					printConsoleAndDebugLine("Could not set antenna ports.");
			}
			else
				printConsoleAndDebugLine("Antenna: Wrong input.");
			continue;
		case CALIBRATE:
			cout << "Specify device ID to calibrate.\n=>calibrate=>";
			cin >> destID;
			cin.ignore();

			cout << "Specify bandwidth in MHz.\n=>calibrate=>";
			cin >> bandwidth;
			cin.ignore();

			cout << "Specify Channel (0/1).\n=>calibrate=>";
			cin >> channel;
			cin.ignore();

			cout << "TX, RX or both? (tx, rx, both)\n=>calibrate=>";
			getline(cin, sCmd);

			retVal = calibrate(deviceVec, destID, bandwidth, sCmd.c_str(), channel);
			if (retVal)
				printConsoleAndDebugLine("Calibration failed.");
			continue;
		case CONSTELLATION:
			cout << "Specify device ID to switch constellation.\n=>constellation=>";
			cin >> destID;
			cin.ignore();

			cout << "Specify the name of the constellation to switch to (lower case, abbreviation. ex: bpsk).\n=>constellation=>";
			getline(cin, sCmd);

			retVal = constellation(deviceVec, destID, sCmd.c_str());
			if (retVal)
				printConsoleAndDebugLine("Constellation change failed.");
			continue;
		case DISCONNECT:
			disconnect(deviceVec);
			nOpened = 0;
			continue;
		case ENABLE:
			cout << "Specify device ID to enable channel.\n=>enable=>";
			cin >> destID;
			cin.ignore();

			cout << "TX, RX or both? (tx, rx, both)\n=>enable=>";
			getline(cin, sCmd);

			cout << "Enable or disable? (1, 0).\n=>enable=>";
			cin >> en;
			cin.ignore();

			cout << "Specify Channel (0/1).\n=>enable=>";
			cin >> channel;
			cin.ignore();

			retVal = enable(deviceVec, destID, en, sCmd.c_str(), channel);
			if (retVal)
				printConsoleAndDebugLine("Enable failed.");
			continue;
		case GAIN:
			cout << "Get or set gain? (get, set)\n=>gain=>";
			getline(cin, sCmd);

			cout << "Specify device ID to modify gain.\n=>gain=>";
			cin >> destID;
			cin.ignore();

			cout << "Specify Channel (0/1).\n=>gain=>";
			cin >> channel;
			cin.ignore();

			if (!strcmp(sCmd.c_str(), "get"))
			{
				retVal = getGain(deviceVec, destID, channel);
				if (retVal)
					printConsoleAndDebugLine("Could not get gain.");
			}
			else if (!strcmp(sCmd.c_str(), "set"))
			{
				printConsoleAndDebugLine(
						"Software does not check gain value inputs on maximum values. However, API does.");

				cout << "TX or RX? (tx, rx)\n=>gain=>set=>";
				getline(cin, sCmd);

				retVal = setGain(deviceVec, destID, sCmd.c_str(), channel);
				if (retVal)
					printConsoleAndDebugLine("Could not set gain.");
			}
			else
				printConsoleAndDebugLine("Gain: Wrong input.");
			continue;
		case INIT:
			cout << "Init all devices? (y/n)\n=>init=>";
			getline(cin, sCmd);

			if (!strcmp(sCmd.c_str(), "y"))
				for (int i = 0; i < nOpened; i++)
					init(deviceVec, i);
			else if (!strcmp(sCmd.c_str(), "n"))
			{
				cout << "Specify device ID to init.\n=>init=>";
				cin >> destID;
				cin.ignore();
				init(deviceVec, destID);
			}
			else
				printConsoleAndDebugLine("Command not understood.");
			continue;
		case LO:
			cout << "Get/Set LO frequency? (get, set)\n=>lo=>";
			getline(cin, sCmd);

			cout << "Specify device ID.\n=>lo=>";
			cin >> destID;
			cin.ignore();

			cout << "Specify Channel (0/1).\n=>lo=>";
			cin >> channel;
			cin.ignore();

			if (!strcmp(sCmd.c_str(), "get"))
			{
				retVal = getLOFreq(deviceVec, destID, channel);
				if(retVal)
					printConsoleAndDebugLine("Could not get LO frequency.");
			}
			else if (!strcmp(sCmd.c_str(), "set"))
			{
				cout << "TX, RX or both? (tx, rx, both)\n=>lo=>set=>";
				getline(cin, sCmd);

				retVal = setLOFreq(deviceVec, destID, sCmd.c_str(), channel);
				if(retVal)
					printConsoleAndDebugLine("Could not set LO frequency.");
			}
			else
				printConsoleAndDebugLine("LO: Wrong input.");
			continue;
		case LOAD:
			cout << "Load Config into which device?\n=>load=>";
			cin >> destID;
			cin.ignore();

			cout << "Specify Config File.\n=>load=>";
			getline(cin, sCmd);

			retVal = loadConfiguration(deviceVec, destID, sCmd.c_str());

			if (retVal)
				printConsoleAndDebugLine("Load Configuration failed.");
			continue;
		case LPBW:
			cout << "Set low-pass bandwidth of which device?\n=>lpbw=>";
			cin >> destID;
			cin.ignore();

			cout << "TX, RX or both? (tx, rx, both)\n=>lpbw=>";
			getline(cin, sCmd);

			cout << "Specify Channel (0/1).\n=>lpbw=>";
			cin >> channel;
			cin.ignore();

			retVal = setLPBW(deviceVec, destID, sCmd.c_str(), channel);
			if (retVal)
				printConsoleAndDebugLine("Set LPBW failed.");
			continue;
		case RESET:
			cout << "Reset all devices? (y/n)\n=>reset=>";
			getline(cin, sCmd);

			if (!strcmp(sCmd.c_str(), "y"))
				reset(deviceVec, -1);
			else if (!strcmp(sCmd.c_str(), "n"))
			{
				cout << "Specify device ID to reset.\n=>reset=>";
				cin >> destID;
				cin.ignore();
				reset(deviceVec, destID);
			}
			else
				printConsoleAndDebugLine("Command not understood.");
			continue;
		case SAMPLE:
			cout << "Get or set? (get, set)\n=>sample=>";
			getline(cin, sCmd);

			cout << "Specify device ID.\n=>sample=>";
			cin >> destID;
			cin.ignore();

			if (!strcmp(sCmd.c_str(), "get"))
			{
				cout << "Specify Channel (0/1).\n=>sample=>get=>";
				cin >> channel;
				cin.ignore();

				retVal = getSamplingRate(deviceVec, destID, channel);
				if (retVal)
					printConsoleAndDebugLine("Get sampling rate failed.");
			}
			else if (!strcmp(sCmd.c_str(), "set"))
			{
				cout << "TX, RX or both? (tx, rx, both)\n=>lpbw=>set=>";
				getline(cin, sCmd);

				retVal = setSamplingRate(deviceVec, destID, sCmd.c_str());
				if (retVal)
					printConsoleAndDebugLine("Set sampling rate failed.");
			}
			else
				printConsoleAndDebugLine("Command not understood.");
			continue;
		case SAVE:
			cout << "Save Config from which device?\n=>save=>";
			cin >> destID;
			cin.ignore();

			cout << "Where to save the Config File?\n=>save=>";
			getline(cin, sCmd);

			retVal = saveConfiguration(deviceVec, destID, sCmd.c_str());

			if (retVal)
				printConsoleAndDebugLine("Save Configuration failed.");
			continue;
		case STREAM:
			cout << "RX device?\n=>stream=>";
			cin >> destID;
			cin.ignore();

			cout << "TX device?\n=>stream=>";
			cin >> sourceID;
			cin.ignore();

			cout << "Specify file to stream in local folder.\n=>stream=>";
			getline(cin, sCmd);

			Stream *stream;
			try
			{
				stream = new Stream(deviceVec, destID, sourceID);
			}
			catch (exception& E)
			{
				printConsoleAndDebugLine(E.what());
				continue;
			}
			if (stream->setupStream())
			{
				printConsoleAndDebugLine("Could not setup stream.");
				delete stream;
				continue;
			}
			if (stream->fileManagement(sCmd.c_str()))
			{
				printConsoleAndDebugLine("File management failed.");
				delete stream;
				continue;
			}
			printConsoleAndDebugLine("Setup stream complete.");

			cout << "Continuous stream? (0/1)\n=>stream=>";
			cin >> nCmd;
			cin.ignore();

			if (stream->startStream((bool)nCmd))
			{
				printConsoleAndDebugLine("Stream failed.");
				delete stream;
				continue;
			}
			delete stream;

			continue;
		case WFMPLAYER:
			printConsoleAndDebugLine("Play Waveform: This feature was planned, but never implemented.");
			//cout << "Specify device to send the waveform.\n=>wfm=>";
			//cin >> sourceID;
			//cin.ignore();

			//cout << "Specify waveform to stream in local folder.\n=>wfm=>";
			//getline(cin, sCmd);

			//if (playWaveform(deviceVec, destID, sCmd.c_str()))
			//	printConsoleAndDebugLine("Play Waveform failed.");
			continue;
		default:
			break;
		}

		printConsoleLine("Main: ERROR. This line should not be reached.");
	}

	disconnect(deviceVec);
	return 0;
}

/*
 * checkIfOpenedStillConnected(const deviceVector& deviceVec, int nConnected)
 * This function will check if all opened devices are still connected and available.
 */
bool checkIfOpenedStillConnected(const deviceVector& deviceVec, int nConnected)
{
	bool found;

	if (deviceVec.empty())
		return false;

	lms_info_str_t* list = new lms_info_str_t[nConnected];

	if (LMS_GetDeviceList(list) < 0)
	{
		printConsoleAndDebugLine("Could not populate device list.");
		delete [] list;
		return true;
	}

	for (const auto& device : deviceVec)
	{
		found = false;
		for (int i = 0; i < nConnected; i++)
		{
			if (!strcmp(list[i], device->getDeviceName()))
			{
				found = true;
				break;
			}
		}
		if (!found)
			return true;
	}
	return false;
}

