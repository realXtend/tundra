// For conditions of distribution and use, see copyright notice in license.txt
#include "StableHeaders.h"

#include <cassert>
#include <sstream>
#include <iostream>

#include "OpenSimAuth.h"

#ifdef WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#endif

//using namespace std;

namespace ProtocolUtilities
{

///\todo Find a way for other OS's.
std::string GetMACaddressString()
{
//#ifdef WIN32
//    IP_ADAPTER_INFO AdapterInfo[16];
//    
//    DWORD dwBufLen = sizeof(AdapterInfo);
//
//    DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
//    if (dwStatus != ERROR_SUCCESS)
//    {
//        ///\todo Log error.
//        assert(false && "GetAdaptersInfo failed!");
//        return "";
//    }
//
//    PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
//    
//    std::stringstream ss;
//    while(pAdapterInfo)
//    {
//        ss << hex << pAdapterInfo->Address[0] <<
//            hex << pAdapterInfo->Address[1] <<
//            hex << pAdapterInfo->Address[2] <<
//            hex << pAdapterInfo->Address[3] <<
//            hex << pAdapterInfo->Address[4] <<
//            hex << pAdapterInfo->Address[5];
//        pAdapterInfo = pAdapterInfo->Next;
//    }
//
//    return ss.str();
//#else
    return "01234567";
//#endif
}

///\todo Find a way for other OS's
std::string GetId0String()
{
#ifdef WIN32
    std::stringstream serial;
    DWORD dwVolSerial;
    BOOL bIsRetrieved;
    bIsRetrieved = GetVolumeInformation(L"C:\\", 0, 0, &dwVolSerial, 0, 0, 0, 0);
    if (bIsRetrieved)
    {
        serial << std::hex << dwVolSerial;
        return serial.str();
    }
    else
    {
        printf("Error: Could not retrieve serial number of the HDD!");
        return std::string("");
    }
#else
    return "76543210";
#endif
}

std::string GetPlatform()
{
#ifdef Q_WS_WIN
    return "Win";
#endif
#ifdef Q_WS_X11
    return "X11";
#endif
#ifdef Q_WS_MAC
    return "Mac";
#endif
    return "";
}

}