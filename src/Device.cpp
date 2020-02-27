/* ==================================================================
 * title:		Device.cpp
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 * This object serves as a small interface between the main program and the API.
 * If the program should be used with a different device, make this class abstract and have a
 * inheritance of LimeSDR and the new device.
 * ==================================================================
 */

#include <Device.h>
#include "Bpsk.h"
#include "Qpsk.h"

/*
 * Device:
 * This object serves as a small interface between the main program and the API.
 * If the programm should be used with a different device, modify this to represent
 * LimeSDR and create a new one for the new device. Then let device inherit them both.
 */
Device::Device(int id_, lms_info_str_t deviceName_)
{
	printDebugLine("Device::create ", id_);
	id = id_;
	strcpy(deviceName, deviceName_);

	if (LMS_Open(&devicePointer, deviceName, NULL))
	{
		printConsoleAndDebugLine("Could not open device number: ", id);
		id = -1;
		return;
	}

	numChannels = devGetNumChannels(true);

	if (numChannels > 2)
	{
		printConsoleAndDebugLine("ERROR: Number of channels is not equal to the hard coded amount (2).", id);
		printConsoleAndDebugLine("This will cause problems in the code, so we will set it back to 2.");
		numChannels = 2;
	}

	// BPSK as standard modulation
	constel = new Bpsk();
}

// Destructor
Device::~Device()
{
	printDebugLine("Device::delete ", id);
	this->devDisconnect();
}

/*
 * changeConstellation(string constel_)
 * Will switch the modulation of the device to the ID specified.
 */
bool Device::changeConstellation(int constellationID)
{
	devLck.lock();
	printDebugLine("Device::changeConstellation ", id);

	switch (constellationID)
	{
	case bpskID:
		delete constel;
		constel = new Bpsk();
		break;
	case qpskID:
		delete constel;
		constel = new Qpsk();
		break;
	default:
		printConsoleAndDebugLine("Device::changeConstellation not found. ", constellationID);
		devLck.unlock();
		return true;
	}
	devLck.unlock();
	return false;
}

/*
 * devCalibrate(bool dir_tx, float bandwidth, int channel)
 * calibrate the device for the specified channel and direction with the given bandwidth in MHz.
 */
int Device::devCalibrate(bool dir_tx, float bandwidth, int channel)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devCalibrate ", id);
	retVal = LMS_Calibrate(devicePointer, dir_tx, channel, bandwidth, 0);
	devLck.unlock();
	return retVal;
}

/*
 * devDisconnect()
 * Call the API function to disconnect the device.
 */
bool Device::devDisconnect()
{
	devLck.lock();
	printDebugLine("Device::devDisconnect ", id);
	LMS_Close(this->devicePointer);
	devLck.unlock();
	return false;
}

/*
 * devEnable(bool dir_tx, bool en, int channel)
 * Enable or disable a specific device Channel.
 * All LimeSDR should have 2 channels in both RX and TX.
 */
int Device::devEnable(bool dir_tx, bool en, int channel)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devEnable ", id);
	retVal = LMS_EnableChannel(devicePointer, dir_tx, channel, en);
	devLck.unlock();
	return retVal;
}

/*
 * devGetAntennaPorts(int channel)
 * returns a string with information about the available antenna ports
 * and the currently active ports for the channel.
 */
string Device::devGetAntennaPorts(int channel)
{
	devLck.lock();
	printDebugLine("Device::devGetAntennaPorts ", id);
	string text;
	lms_name_t antennaRX_list[10], antennaTX_list[10];
	int n, nRX, nTX;

	nRX = LMS_GetAntennaList(devicePointer, LMS_CH_RX, channel, antennaRX_list);
	nTX = LMS_GetAntennaList(devicePointer, LMS_CH_TX, channel, antennaTX_list);

	if ((nRX < 0) | (nTX < 0))
		return "";

	n = max(nRX, nTX);

	text = "ID| RX            | TX\n";
	for (int i = 0; i < n; i++)
	{
		text += to_string(i) + ": ";
		if (nRX > i)
		{
			text.append(antennaRX_list[i]);
			text += "           | ";
		}
		else
			text += "               | ";

		if (nTX > i)
		{
			text.append(antennaTX_list[i]);
			text += "\n";
		}
		else
			text += "                 \n";
	}

	nRX = LMS_GetAntenna(devicePointer, LMS_CH_RX, 0);
	nTX = LMS_GetAntenna(devicePointer, LMS_CH_TX, 0);

    text += "Device: " + to_string(id) + ". " +
			"Acitve ports: RX " + to_string(nRX) + ", TX " + to_string(nTX) + ". Channel: " + to_string(channel) +
			". DeviceID: " + to_string(id);
	devLck.unlock();
	return text;
}

/*
 * devGetSynthesiserFrequency(size_t clk_id)
 * Will return the current value of these three clocks:
 * LMS_CLOCK_SXT / LMS_CLOCK_SXR / LMS_CLOCK_CGEN
 */
string Device::devGetSynthesiserFrequency(size_t clk_id)
{
	devLck.lock();
	printDebugLine("Device::devGetSynthesiserFrequency ", id);
	float_type freq;
	int retVal;
	retVal = LMS_GetClockFreq(devicePointer, clk_id, &freq);
	if (retVal)
	{
		devLck.unlock();
		return "";
	}
	else
	{
		string result;
		if (clk_id == LMS_CLOCK_SXT)
			result = "SXT  (2): ";
		else if (clk_id == LMS_CLOCK_SXR)
			result = "SXR  (1): ";
		else if (clk_id == LMS_CLOCK_CGEN)
			result = "CGEN (3): ";
		else
			result = "?: ";

		result += to_string(freq);
		devLck.unlock();
		return result;
	}
}

/*
 * devGetGain(int channel)
 * returns a string with information about the currently set gain value for the specified channel.
 */
string Device::devGetGain(int channel)
{
	devLck.lock();
	printDebugLine("Device::devGetGain ", id);
	string text;

	unsigned int gainRX, gainTX;
    bool retVal;

    retVal = LMS_GetGaindB(devicePointer, LMS_CH_TX, channel, &gainTX);
    if (retVal)
    	return "";

    retVal = LMS_GetGaindB(devicePointer, LMS_CH_RX, channel, &gainRX);
    if (retVal)
    	return "";

    text = "Device: " + to_string(id) + ". " +
    		"Gain: RX = " + to_string((signed int)gainRX) + "dB (73 max), TX = "
			+ to_string((signed int)gainTX) + "dB (78 max). Channel: " + to_string(channel);
	devLck.unlock();
    return text;
}

/*
 * devGetLOFreq(int channel)
 * returns a string with information about the currently set LO frequency for the specified channel.
 */
string Device::devGetLOFreq(int channel)
{
	devLck.lock();
	printDebugLine("Device::devGetLOFreq ", id);
	string text;

	devLck.unlock();
	text = this->devGetLOFreqRange();
	devLck.lock();
	if (!text.empty())
		printConsoleAndDebugLine(text.c_str());

    float_type freqRX, freqTX;
    bool retVal;

    retVal = LMS_GetLOFrequency(devicePointer, LMS_CH_TX, channel, &freqTX);
    if (retVal)
    	return "";

    retVal = LMS_GetLOFrequency(devicePointer, LMS_CH_RX, channel, &freqRX);
    if (retVal)
    	return "";

    text = "Device: " + to_string(id) + ". " +
    		"LO Frequency: RX = " + to_string(freqRX / 1e6) + "MHz, TX = "
			+ to_string(freqTX / 1e6) + "MHz. Channel: " + to_string(channel);

	devLck.unlock();
    return text;
}

/*
 * devGetLOFreqRange()
 * returns a string with information about the possible LO Frequencies.
 */
string Device::devGetLOFreqRange()
{
	devLck.lock();
	printDebugLine("Device::devGetLOFreq ", id);
	string text;

	lms_range_t rangeRX, rangeTX;
    bool retVal;

    retVal = LMS_GetLOFrequencyRange(devicePointer, LMS_CH_TX, &rangeTX);
    if (retVal)
    	return "";

    retVal = LMS_GetLOFrequencyRange(devicePointer, LMS_CH_RX, &rangeRX);
    if (retVal)
    	return "";

    text = "Device: " + to_string(id) + ". " +
    		"LO Frequency range: RX = " + to_string(rangeRX.min / 1e6) + "MHz - " + to_string(rangeRX.max / 1e6) +
    		"MHz, step " + to_string(rangeRX.step) + ". TX = " + to_string(rangeTX.min / 1e6) + "MHz - " +
			to_string(rangeTX.max / 1e6) + "MHz, step " + to_string(rangeTX.step);
	devLck.unlock();
    return text;
}

/*
 * devGetLPBW(int channel)
 * returns a string with information about the current LP bandwidth for the specified channel.
 */
string Device::devGetLPBW(int channel)
{
	devLck.lock();
	printDebugLine("Device::devGetLPBW ", id);
	string text;

    float_type bwRX, bwTX;
    bool retVal;

	devLck.unlock();
	text = this->devGetLPBWRange();
	devLck.lock();
	if (!text.empty())
		printConsoleAndDebugLine(text.c_str());

	retVal = LMS_GetLPFBW(devicePointer, LMS_CH_RX, channel, &bwRX);
	if (retVal)
		return "";

	retVal = LMS_GetLPFBW(devicePointer, LMS_CH_TX, channel, &bwTX);
	if (retVal)
		return "";

    text = "Device: " + to_string(id) + ". " +
    		"Currently LPBW set to RX = " + to_string(bwRX / 1e6) + "MHz, TX = " +
    		to_string(bwTX / 1e6) + "MHz. Channel: " + to_string(channel);
	devLck.unlock();
    return text;
}

/*
 * devGetLPBWRange()
 * returns a string with information about the possible LP bandwidth range.
 */
string Device::devGetLPBWRange()
{
	devLck.lock();
	printDebugLine("Device::devGetLPBWRange ", id);
	string text;

    lms_range_t rangeRX, rangeTX;
    bool retVal;

    retVal = LMS_GetLPFBWRange(devicePointer, LMS_CH_RX, &rangeRX);
	if (retVal)
		return "";

    retVal = LMS_GetLPFBWRange(devicePointer, LMS_CH_TX, &rangeTX);
	if (retVal)
		return "";

    text = "Device: " + to_string(id) + ". " +
    		"LPBW Range: RX = " + to_string(rangeRX.min / 1e6) + "MHz - " + to_string(rangeRX.max / 1e6) +
			"MHz, step " + to_string(rangeRX.step) + ". TX = " + to_string(rangeTX.min / 1e6) + "MHz - " +
			to_string(rangeTX.max / 1e6) + "MHz, step " + to_string(rangeTX.step);
	devLck.unlock();
    return text;
}

/*
 * devGetNumChannels(bool dir_tx)
 * returns the number of channels for the specified direction.
 */
int Device::devGetNumChannels(bool dir_tx)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devGetNumChannels ", id);
	retVal = LMS_GetNumChannels(devicePointer, dir_tx);
	devLck.unlock();
	return retVal;
}

/*
 * devGetSamplingRate(int channel)
 * returns a string with information about the currently active sampling rate for the specified channel.
 */
string Device::devGetSamplingRate(int channel)
{
	devLck.lock();
	printDebugLine("Device::devGetSamplingRate ", id);
	string text = "";

    float_type rateRX, rf_rateRX;
    float_type rateTX, rf_rateTX;
    bool retVal;

    devLck.unlock();
	text = this->devGetSamplingRateRange();
	devLck.lock();
	if (!text.empty())
		printConsoleAndDebugLine(text.c_str());

    retVal = LMS_GetSampleRate(devicePointer, LMS_CH_RX, channel, &rateRX, &rf_rateRX);
    if (retVal)
    	return "";

    retVal = LMS_GetSampleRate(devicePointer, LMS_CH_TX, channel, &rateTX, &rf_rateTX);
    if (retVal)
    	return "";

    text = "Device: " + to_string(id) + ". " +
    		"Sampling rate: RX host = " + to_string(rateRX / 1e6) + "MHz with DAC RF rate " + to_string(rf_rateRX / 1e6) + "MHz. TX host = " +
			to_string(rateTX / 1e6) + "MHz with ADC RF rate " + to_string(rf_rateTX / 1e6) + "MHz. Channel: " + to_string(channel);
	devLck.unlock();
    return text;
}

/*
 * devGetSamplingRateRange()
 * returns a string of the possible sampling rate.
 */
string Device::devGetSamplingRateRange()
{
	devLck.lock();
	printDebugLine("Device::devGetSamplingRateRange ", id);
	string text;

    lms_range_t rangeRX, rangeTX;
	bool retVal;

	retVal = LMS_GetSampleRateRange(devicePointer, LMS_CH_RX, &rangeRX);
	if (retVal)
		return "";

	retVal = LMS_GetSampleRateRange(devicePointer, LMS_CH_TX, &rangeTX);
	if (retVal)
		return "";

    text = "Device: " + to_string(id) + ". " +
    		"Samplingrate Range: RX = " + to_string(rangeRX.min / 1e6) + "MHz - " + to_string(rangeRX.max / 1e6) +
			"MHz, step " + to_string(rangeRX.step) + ". TX = " + to_string(rangeTX.min / 1e6) + "MHz - " +
			to_string(rangeTX.max / 1e6) + "MHz, step " + to_string(rangeTX.step);
	devLck.unlock();
    return text;
}

/*
 * devInit()
 * Will initialize the device to default values (1250MHz)
 */
void Device::devInit()
{
	devLck.lock();
	printDebugLine("Device::devInit ", id);
	LMS_Init(devicePointer);
	devLck.unlock();
}

/*
 * devLoadConfig(const char *filename)
 * Will try to load the devices with the configuration specified by filename (actually pth + filename).
 */
int Device::devLoadConfig(const char *filename)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devLoadConfig ", id);
	retVal = LMS_LoadConfig(devicePointer, filename);
	devLck.unlock();
	return retVal;
}

/*
 * devPrintInfo()
 * Will print out the device info to both the console and the log file.
 */
void Device::devPrintInfo()
{
	devLck.lock();
	string info = to_string(id) + ": " + deviceName;
	printConsoleAndDebugLine(info.c_str());
	devLck.unlock();
}

/*
 * devPrintInfoNoConsole()
 * Will print out the device info, only into the log file.
 */
void Device::devPrintInfoNoConsole()
{
	devLck.lock();
	string info = to_string(id) + ": " + deviceName;
	printDebugLine(info.c_str());
	devLck.unlock();
}

/*
 * devReset()
 * Will reset the device, which also means no output by disabling all channels.
 */
void Device::devReset()
{
	devLck.lock();
	printDebugLine("Device::devReset ", id);
	LMS_Reset(devicePointer);
	devLck.unlock();
}

/*
 * devSaveConfig(const char *filename)
 * Will try to save the current configuration of the device into the path & filename specified by filename.
 */
int Device::devSaveConfig(const char *filename)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devSaveConfig ", id);
	retVal = LMS_SaveConfig(devicePointer, filename);
	devLck.unlock();
	return retVal;
}

/*
 * devSetAntennaPorts(int channel, bool dir_tx, int port)
 * Will set the antenna port with for direction dir_tx for the specified channel to the value of port.
 */
int Device::devSetAntennaPorts(int channel, bool dir_tx, int port)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devSetAntennaPorts ", id);
	retVal = LMS_SetAntenna(devicePointer, dir_tx, channel, port);
	devLck.unlock();
	return retVal;
}

/*
 * devSetSynthesiserFrequency(size_t clk_id, float_type freq)
 * Will set the clock frequency specified to the clocks
 * LMS_CLOCK_SXT / LMS_CLOCK_SXR / LMS_CLOCK_CGEN / ...
 */
int Device::devSetSynthesiserFrequency(size_t clk_id, float_type freq)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devSetSynthesiserFrequency ", id);
	retVal = LMS_SetClockFreq(devicePointer, clk_id, freq);
	devLck.unlock();
	return retVal;
}

/*
 * devSetGain(int channel, bool dir_tx, unsigned int gain)
 * Will set the relative gain for the direction dir_tx for the specified channel to the value gain.
 */
int Device::devSetGain(int channel, bool dir_tx, unsigned int gain)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devSetGain ", id);
	retVal = LMS_SetGaindB(devicePointer, dir_tx, channel, gain);
	devLck.unlock();
	return retVal;
}

/*
 * devSetLOFreq(int channel, bool dir_tx, float_type freq)
 * Will set the LO frequency for the direction dir_tx for the specified channel to the given frequency in MHz.
 */
int Device::devSetLOFreq(int channel, bool dir_tx, float_type freq)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devSetLOFreq ", id);
	retVal = LMS_SetLOFrequency(devicePointer, dir_tx, channel, (float_type)freq);
	devLck.unlock();
	return retVal;
}

/*
 * devSetLPBW(int channel, bool dir_tx, float_type bandwidth)
 * Will set the LP bandwidth for the direction dir_tx for the specified channel to the value bandwidth in MHz.
 */
int Device::devSetLPBW(int channel, bool dir_tx, float_type bandwidth)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devSetLPBW ", id);
	retVal = LMS_SetLPFBW(devicePointer, dir_tx, channel, bandwidth);
	devLck.unlock();
	return retVal;
}

/*
 * devSetSamplingRate(float_type rate, float_type sampling, bool dir_tx)
 * Set the host sampling rate for direction dir_tx to rate (MHz) and the ADC/DAC sampling
 */
int Device::devSetSamplingRate(float_type rate, size_t sampling, bool dir_tx)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devSetSamplingRate ", id);
	retVal = LMS_SetSampleRateDir(devicePointer, dir_tx, rate, sampling);
	devLck.unlock();
	return retVal;
}

/*
 * devSetSamplingRate(float_type rate, float_type sampling)
 * Set the host sampling rate for both directions to rate (MHz) and the ADC/DAC sampling
 */
int Device::devSetSamplingRate(float_type rate, size_t sampling)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devSetSamplingRate ", id);
	retVal = LMS_SetSampleRate(devicePointer, rate, sampling);
	devLck.unlock();
	return retVal;
}

//////////////////////////////////////////////////////////////////////7
// Member varibles Getter / Setter
///////////////////////////////////////////////////////////////////////
// Do these need to be thread safe? Probably not
int Device::getId() const
{
	return id;
}

void Device::setId(int id)
{
	this->id = id;
}

const char *Device::getDeviceName() const
{
	return deviceName;
}

int Device::getNumChannels() const
{
	return max(LMS_GetNumChannels(devicePointer, LMS_CH_RX),
			LMS_GetNumChannels(devicePointer, LMS_CH_TX));
}

//////////////////////////////////////////////////////////////////////7
// Streaming functions
///////////////////////////////////////////////////////////////////////
// Destroy Stream
int Device::devDestroyStream(lms_stream_t *streamObj)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devDestroyStream ", id);
	retVal = LMS_DestroyStream(devicePointer, streamObj);
	devLck.unlock();
	return retVal;
}

// Safe stream status object into status
int Device::devGetStreamStatus(lms_stream_t *stream, lms_stream_status_t* status)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devGetStreamStatus ", id);
	retVal = LMS_GetStreamStatus(stream, status);
	devLck.unlock();
	return retVal;
}

// Send data to TX thread, stored in samples with length sample_count.
// Mmta and timeout is optional.
int Device::devSendStream(lms_stream_t *streamObj, const void *samples, size_t sample_count,
		lms_stream_meta_t *meta, unsigned timeout_ms)
{
	int retVal;
	devLck.lock();
	//printDebugLine("Device::devSendStream ", id);
	retVal = LMS_SendStream(streamObj, samples, sample_count, meta, timeout_ms);
	devLck.unlock();
	return retVal;
}

// Receive data from RX thread, stored in samples with length sample_count.
// meta and timeout is optional.
int Device::devReceiveStream(lms_stream_t *streamObj, void *samples, size_t sample_count,
		lms_stream_meta_t *meta, unsigned timeout_ms)
{
	int retVal;
	//devLck.lock();
	//printDebugLine("Device::devReceiveStream ", id);
	retVal = LMS_RecvStream(streamObj, samples, sample_count, meta, timeout_ms);
	//devLck.unlock();
	return retVal;
}

// Setup stream (threads, buffer, ...)
int Device::devSetupStream(lms_stream_t *streamObj)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devSetupStream ", id);
	retVal = LMS_SetupStream(devicePointer, streamObj);
	devLck.unlock();
	return retVal;
}

// Start stream by setting thread to active
int Device::devStartStream(lms_stream_t *streamObj)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devStartStream ", id);
	retVal = LMS_StartStream(streamObj);
	devLck.unlock();
	return retVal;
}

// Stop stream by stopping active thread
int Device::devStopStream(lms_stream_t *streamObj)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devStopStream ", id);
	retVal = LMS_StopStream(streamObj);
	devLck.unlock();
	return retVal;
}

///////////////////////////////////////////////////////////////////////
// Toggle functions
///////////////////////////////////////////////////////////////////////
int Device::toggleAGC(uint32_t wantedRSSI, bool start)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::toggleAGC ", id);
	retVal = LMS_ToogleAGC(devicePointer, wantedRSSI, start);
	devLck.unlock();
	return retVal;
}

///////////////////////////////////////////////////////////////////////
// SPI
///////////////////////////////////////////////////////////////////////
int Device::devReadParam(const std::string& name, uint16_t *val)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devReadParam ", id);
	retVal = LMS_ReadParam(devicePointer, name, val);
	devLck.unlock();
	return retVal;
}

int Device::devReadParam(struct LMS7Parameter param, uint16_t *val)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devReadParam ", id);
	retVal = LMS_ReadParam(devicePointer, param, val);
	devLck.unlock();
	return retVal;
}

int Device::devWriteParam(const std::string& name, uint16_t val)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devWriteParam ", id);
	retVal = LMS_WriteParam(devicePointer, name, val);
	devLck.unlock();
	return retVal;
}

int Device::devWriteParam(struct LMS7Parameter param, uint16_t val)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devWriteParam ", id);
	retVal = LMS_WriteParam(devicePointer, param, val);
	devLck.unlock();
	return retVal;
}

// GFIR
int Device::devSetGFIRLPF(bool dir_tx, size_t chan, bool enabled, float_type bandwidth)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devWriteParam ", id);
	retVal = LMS_SetGFIRLPF(devicePointer, dir_tx, chan, enabled, bandwidth);
	devLck.unlock();
	return retVal;
}

// Functions setting specific SPI values.
int Device::devSetInterpolation(uint16_t val)
{
	return this->devWriteParam("HBI_OVR_TXTSP", val);
}

int Device::devSetDecimation(uint16_t val)
{
	return this->devWriteParam("HBD_OVR_RXTSP", val);
}

// Probably should not use this directly! This was coded at a late stage of the project and is not tested.
/*int Device::devSetIntpAndDeciAndTune(float_type freqMHz, int interpolation, int decimation)
{
	int retVal;
	devLck.lock();
	printDebugLine("Device::devSetIntpAndDeci ", id);
	retVal = LMS_SetIntpAndDeciAndTune(devicePointer, freqMHz, interpolation, decimation);
	devLck.unlock();
	return retVal;
}*/
