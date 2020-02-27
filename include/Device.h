/* ==================================================================
 * title:		Device.h
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 * This object serves as a small interface between the main program and the API.
 * If the program should be used with a different device, make this class abstract and have a
 * inheritance of LimeSDR and the new device.
 * ==================================================================
 */

#ifndef INCLUDE_DEVICE_H_
#define INCLUDE_DEVICE_H_

#include "LimeSuite.h"

#include "debug_logger.h"
#include "lms7_customAPIs.h"
#include "Constellation.h"

#include <iostream>
#include <cstring>
#include <mutex>

using namespace std;
class Constellation;

class Device {
public:
	Device(int id_, lms_info_str_t deviceName_);
	~Device();

	int devCalibrate(bool dir_tx, float bandwidth, int channel);
	bool devDisconnect();
	int devEnable(bool dir_tx, bool en, int channel);
	void devInit();
	void devPrintInfo();
	void devPrintInfoNoConsole();
	void devReset();

	// Load/Save Configuration
	int devLoadConfig(const char *filename);
	int devSaveConfig(const char *filename);

	// Parameter getter / setter
	string devGetAntennaPorts(int channel);
	string devGetSynthesiserFrequency(size_t clk_id);
	string devGetGain(int channel);
	string devGetLOFreq(int channel);
	string devGetLOFreqRange();
	string devGetLPBW(int channel);
	string devGetLPBWRange();
	int devGetNumChannels(bool dir_tx);
	string devGetSamplingRate(int channel);
	string devGetSamplingRateRange();
	int devSetAntennaPorts(int channel, bool dir_tx, int port);
	int devSetSynthesiserFrequency(size_t clk_id, float_type freq);
	int devSetGain(int channel, bool dir_tx, unsigned int gain);
	int devSetLOFreq(int channel, bool dir_tx, float_type freq);
	int devSetLPBW(int channel, bool dir_tx, float_type bandwidth);
	int devSetSamplingRate(float_type rate, size_t sampling, bool dir_tx);
	int devSetSamplingRate(float_type rate, size_t sampling);

	// Streaming calls
	int devDestroyStream(lms_stream_t *streamObj);
	int devGetStreamStatus(lms_stream_t *stream, lms_stream_status_t* status);
	int devSendStream(lms_stream_t *streamObj, const void *samples, size_t sample_count,
			lms_stream_meta_t *meta, unsigned timeout_ms);
	int devReceiveStream(lms_stream_t *streamObj, void *samples, size_t sample_count,
			lms_stream_meta_t *meta, unsigned timeout_ms);
	int devSetupStream(lms_stream_t *streamObj);
	int devStartStream(lms_stream_t *streamObj);
	int devStopStream(lms_stream_t *streamObj);

	// Transmission specific
	bool changeConstellation(int constellationID);

	// Member variables Getter / Setter
	int getId() const;
	void setId(int id);
	const char *getDeviceName() const;
	int getNumChannels() const;

	// Toggle functions
	int toggleAGC(uint32_t wantedRSSI, bool start);

	// GFIR
	int devSetGFIRLPF(bool dir_tx, size_t chan, bool enabled, float_type bandwidth);

	// SPI
	int devReadParam(const std::string& name, uint16_t *val);
	int devReadParam(struct LMS7Parameter param, uint16_t *val);
	int devWriteParam(const std::string& name, uint16_t val);
	int devWriteParam(struct LMS7Parameter param, uint16_t val);

	// Functions setting SPI values.
	int devSetInterpolation(uint16_t val);
	int devSetDecimation(uint16_t val);
	int devSetIntpAndDeciAndTune(float_type freqMHz, int interpolation, int decimation);

	// Play waveform
	//int devUploadWFM()

	Constellation *constel;
protected:

private:
	int id;
	int numChannels;
	lms_info_str_t deviceName;
	lms_device_t *devicePointer;

	mutex devLck;
};



#endif /* INCLUDE_DEVICE_H_ */
