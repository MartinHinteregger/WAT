/* ==================================================================
 * title:		debug_logger.cpp
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 * This is a simple, custom debug logger.
 * It includes functions for printing into a log file, printing to console or both.
 * ==================================================================
 */

#include "debug_logger.h"

const char* logFileName = "log.txt";
mutex lckFile;

// Filedescriptor
FILE* fdLogFile;

// initialize the logger by checking the file
bool debug_logger()
{
	bool fileExists;
	char oldLogFileName[32] = "old_";
	strcat(oldLogFileName, logFileName);

	fileExists = access(logFileName, F_OK);

	if (!fileExists)
	{
		if (rename(logFileName, oldLogFileName))
			cout << "Could not move previous log file, will overwrite.\n";
	}

	if (openLogFile(true))
		return true;

	writeHeader();

	fclose(fdLogFile);
	return false;
}

// Write log file header information
void writeHeader()
{
	if (fdLogFile == NULL)
		return;

	fprintf(fdLogFile, "Wireless Analysis Tool by mh. Software version: %.2f\n", swVersion);
}

// Call this function to open the log file, either new or appended (newFile is default false).
bool openLogFile(bool newFile)
{
	if (newFile)
		fdLogFile = fopen(logFileName, "w");
	else
		fdLogFile = fopen(logFileName, "a");

	if (fdLogFile == NULL)
	{
		cout << "debug_logger: Could not open log file.\n";
		return true;
	}
	else
		return false;
}

/*
 * The following functions will call both functions to print text and the additional variables
 * into the console and the log file.
 */
void printConsoleAndDebugLine(const char* text, int addInt)
{
	printDebugLine(text, addInt);
	printConsoleLine(text, addInt);
}

void printConsoleAndDebugLine(const char* text, float addFloat)
{
	printDebugLine(text, addFloat);
	printConsoleLine(text, addFloat);
}

void printConsoleAndDebugLine(const char* text)
{
	printDebugLine(text);
	printConsoleLine(text);
}

/*
 * These functions will print text into the console. When called with additional argument they
 * will print it as well.
 */
void printConsoleLine(const char* text, int addInt)
{
	cout << text << addInt << "\n";
}


void printConsoleLine(const char* text, float addFloat)
{
	cout << text << addFloat << "\n";
}

void printConsoleLine(const char* text)
{
	cout << text << "\n";
}

/*
 * The following functions will print a line into the text file.
 * Either call the function with a single text, or with an additional parameter.
 * When called with additional parameter, the function will format the text and call the native function.
 */
void printDebugLine(const char* text, int addInt)
{
	string newText;
	newText = text + to_string(addInt);

	printDebugLine(newText.c_str());
}

void printDebugLine(const char* text, float addFloat)
{
	string newText;
	newText = text + to_string(addFloat);

	printDebugLine(newText.c_str());
}


bool printDebugLine(const char* text)
{
	lckFile.lock();

	if (text == NULL)
		return false;

	if (openLogFile())
		return false;

	// Get time
	time_t timer;
	struct tm * timeinfo;

	timer = time(NULL);
	timeinfo = localtime(&timer);
	char textTime[32];
	string textCat;

	strftime(textTime,sizeof(textTime),"%d-%m-%Y %H:%M:%S: ",timeinfo);
	fprintf(fdLogFile, "%s", textTime);
	fprintf(fdLogFile, "%s", text);
	fprintf(fdLogFile, "\n");

	fclose(fdLogFile);
	lckFile.unlock();
	return true;
}
