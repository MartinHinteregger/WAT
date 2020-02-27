/* ==================================================================
 * title:		lms7_customAPIs.h
 * author:		mh
 * project:		Masterthesis Martin Hinteregger
 * description:
 * ==================================================================
 */

#ifndef INCLUDE_LMS7_CUSTOMAPIS_H_
#define INCLUDE_LMS7_CUSTOMAPIS_H_

#include "LimeSuite.h"
#include "Logger.h"
#include "lms7_device.h"

API_EXPORT int CALL_CONV LMS_ToogleAGC(lms_device_t *dev, uint32_t wantedRSSI, bool start);
API_EXPORT int CALL_CONV LMS_ReadParam(lms_device_t *dev, const std::string& name, uint16_t *val);
API_EXPORT int CALL_CONV LMS_WriteParam(lms_device_t *dev, const std::string& name, uint16_t val);
API_EXPORT int CALL_CONV LMS_SetIntpAndDeciAndTune(lms_device_t *dev, float_type freqMHz, int interpolation, int decimation);

#endif /* INCLUDE_LMS7_CUSTOMAPIS_H_ */
