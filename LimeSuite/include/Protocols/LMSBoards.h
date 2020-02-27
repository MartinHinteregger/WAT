/**
@file	LMSBoards.h
@author Lime Microsystems
@brief	enumerations of available LMS7002M hardware
*/
#pragma once

namespace lime {

enum eLMS_DEV
{
    LMS_DEV_UNKNOWN,
    LMS_DEV_EVB6,
    LMS_DEV_DIGIGREEN,
    LMS_DEV_DIGIRED,
    LMS_DEV_EVB7,
    LMS_DEV_ZIPPER,
    LMS_DEV_SOCKETBOARD,
    LMS_DEV_EVB7V2,
    LMS_DEV_STREAM,
    LMS_DEV_NOVENA,
    LMS_DEV_DATASPARK,
    LMS_DEV_RFSPARK,
    LMS_DEV_LMS6002USB,
    LMS_DEV_RFESPARK,
    LMS_DEV_LIMESDR,
    LMS_DEV_LIMESDR_PCIE,
    LMS_DEV_LIMESDR_QPCIE,
    LMS_DEV_LIMESDRMINI,
    LMS_DEV_USTREAM,
    LMS_DEV_LIMESDR_SONY, //stand alone board with Sony PAs, tuners
    LMS_DEV_LIMESDR_USB_SP,
    LMS_DEV_LMS7002M_ULTIMATE_EVB,

    LMS_DEV_COUNT
};

const char LMS_DEV_NAMES[][80] =
{
    "UNKNOWN",
    "EVB6",
    "DigiGreen",
    "DigiRed",
    "EVB7",
    "ZIPPER",
    "Socket Board",
    "EVB7_v2",
    "Stream",
    "Novena",
    "DataSpark",
    "RF-Spark",
    "LMS6002-USB Stick",
    "RF-ESpark",
    "LimeSDR-USB",
    "LimeSDR-PCIe",
    "LimeSDR-QPCIe",
    "LimeSDR-Mini",
    "uStream",
    "LimeSDR SONY PA",
    "LimeSDR-USB SP",
    "LMS7002M Ultimate EVB"
};

static inline const char* GetDeviceName(const eLMS_DEV device)
{
    if (LMS_DEV_UNKNOWN < device && device < LMS_DEV_COUNT)
        return LMS_DEV_NAMES[device];
    else
        return LMS_DEV_NAMES[LMS_DEV_UNKNOWN];
}

enum eEXP_BOARD
{
    EXP_BOARD_UNKNOWN,
    EXP_BOARD_UNSUPPORTED,
    EXP_BOARD_NO,
    EXP_BOARD_MYRIAD1,
    EXP_BOARD_MYRIAD2,
    EXP_BOARD_MYRIAD_NOVENA,
    EXP_BOARD_HPM1000,
    EXP_BOARD_MYRIAD7,
    EXP_BOARD_HPM7,
    EXP_BOARD_MYRIAD7_NOVENA,

    EXP_BOARD_COUNT
};

const char EXP_BOARD_NAMES[][80] =
{
    "UNKNOWN",
    "UNSUPPORTED",
    "NOT AVAILABLE",
    "Myriad1",
    "Myriad2",
    "Novena",
    "HPM1000",
    "Myriad7",
    "HMP7",
    "Myriad7 Novena",
};

static inline const char* GetExpansionBoardName(const eEXP_BOARD board)
{
    if (EXP_BOARD_UNKNOWN < board && board < EXP_BOARD_COUNT)
        return EXP_BOARD_NAMES[board];
    else
        return EXP_BOARD_NAMES[EXP_BOARD_UNKNOWN];
}

}
