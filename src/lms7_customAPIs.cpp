/* ==================================================================
 * title:		lms7_customAPIs.cpp
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 * ==================================================================
 */

#include "lms7_customAPIs.h"

API_EXPORT int CALL_CONV LMS_ToogleAGC(lms_device_t *dev, uint32_t wantedRSSI, bool start)
{
	// See function int LMS7_Device::MCU_AGCStart(uint32_t wantedRSSI) in lms7_device.cpp
    if (dev == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

	lime::LMS7_Device* lms = (lime::LMS7_Device*)dev;

	if (start)
	{
		double wantedRSSIdouble = (double) wantedRSSI;
		uint32_t wantedRSSIreal = 87330 / pow(10.0, (3+wantedRSSIdouble)/20);
		return lms->MCU_AGCStart(wantedRSSIreal);
	}
	else
		return lms->MCU_AGCStop();
}

// LMS_ReadParam alternative passing the name instead of the parameter
API_EXPORT int CALL_CONV LMS_ReadParam(lms_device_t *dev, const std::string& name, uint16_t *val)
{
    if (dev == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
    lime::LMS7_Device* lms = (lime::LMS7_Device*)dev;
    *val = lms->ReadParam(name);
    return LMS_SUCCESS;
}

// LMS_WriteParam alternative passing the name instead of the parameter
API_EXPORT int CALL_CONV LMS_WriteParam(lms_device_t *dev, const std::string& name, uint16_t val)
{
    if (dev == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
    lime::LMS7_Device* lms = (lime::LMS7_Device*)dev;

    return lms->WriteParam(name, val);
}

// Probably should not use this. This was coded at a late stage of the project and is not tested.
/*API_EXPORT int CALL_CONV LMS_SetIntpAndDeciAndTune(lms_device_t *dev, float_type freqMHz, int interpolation, int decimation)
{
	int retVal;
    if (dev == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
    lime::LMS7_Device* lms_dev = (lime::LMS7_Device*)dev;

    lime::LMS7002M* lms = lms_dev->GetLMS();

    retVal = lms->TuneVCO(lime::LMS7002M::VCO_CGEN);
    if (retVal)
    	return retVal;

    retVal = lms->SetInterfaceFrequency(freqMHz*1e6, interpolation, decimation);
    if (retVal)
    	return retVal;

    return lms_dev->SetFPGAInterfaceFreq(interpolation, decimation);
}*/
