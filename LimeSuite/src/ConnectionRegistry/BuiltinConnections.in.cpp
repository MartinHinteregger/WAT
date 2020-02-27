/***********************************************************************
 * This is a collection of all built-in connections that are included
 * with the library. Additional connections can be added dynamically.
 **********************************************************************/

// WAT: Modified this file to only allow FX3 communication.
//#cmakedefine ENABLE_EVB7COM
//#cmakedefine ENABLE_FX3
//#cmakedefine ENABLE_STREAM_UNITE
//#cmakedefine ENABLE_NOVENARF7
//#cmakedefine ENABLE_FTDI
//#cmakedefine ENABLE_PCIE_XILLYBUS
//#cmakedefine ENABLE_REMOTE

//void __loadConnectionEVB7COMEntry(void);
void __loadConnectionFX3Entry(void);
//void __loadConnectionSTREAM_UNITEEntry(void);
//void __loadConnectionNovenaRF7Entry(void);
//void __loadConnectionFT601Entry(void);
//void __loadConnectionXillybusEntry(void);
//void __loadConnectionRemoteEntry(void);

void __loadAllConnections(void)
{
//    #ifdef ENABLE_EVB7COM
//    __loadConnectionEVB7COMEntry();
//    #endif

    //#ifdef ENABLE_FX3
    __loadConnectionFX3Entry();
    //#endif

//    #ifdef ENABLE_STREAM_UNITE
//    __loadConnectionSTREAM_UNITEEntry();
//    #endif
//
//    #ifdef ENABLE_FTDI
//    __loadConnectionFT601Entry();
//    #endif
//
//    #ifdef ENABLE_NOVENARF7
//    __loadConnectionNovenaRF7Entry();
//    #endif
//
//    #ifdef ENABLE_PCIE_XILLYBUS
//    __loadConnectionXillybusEntry();
//    #endif
//
//    #ifdef ENABLE_REMOTE
//    __loadConnectionRemoteEntry();
//    #endif
}
