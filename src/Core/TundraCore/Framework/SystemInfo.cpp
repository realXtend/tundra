/* Copyright 2011 Jukka Jylänki

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

/** @file   SystemInfo.cpp
    @brief  Utility functions for retrieving system information.
    @note   Code originally from MathGeoLib and modified for Tundra usage. */


#include "StableHeaders.h"
#include "SystemInfo.h"
#include "CoreStringUtils.h"

#include <QString>
#include <sstream>
#include <iostream>

#if defined(WIN32) && !defined(__MINGW32__) && !defined(WIN8RT)

#include "Win.h"
#include <intrin.h>
#include <iphlpapi.h>

#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "IPHLPAPI.lib")

#if defined(DIRECTX_ENABLED)
#if WINVER >= 0x0600
#include <dxgi.h>
typedef HRESULT (WINAPI* DXGICREATEFACTORY)(REFIID, void**);
#endif
#endif

#define BUFSIZE 256

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

BOOL GetOSDisplayString(LPTSTR pszOS)
{
    OSVERSIONINFOEX osvi;
    SYSTEM_INFO si;
    PGNSI pGNSI;
    PGPI pGPI;
    BOOL bOsVersionInfoEx;
    DWORD dwType;

    ZeroMemory(&si, sizeof(SYSTEM_INFO));
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi);
    if (!bOsVersionInfoEx)
        return 1;

    // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
    pGNSI = (PGNSI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
    if (NULL != pGNSI)
        pGNSI(&si);
    else
        GetSystemInfo(&si);

    if ( VER_PLATFORM_WIN32_NT == osvi.dwPlatformId && osvi.dwMajorVersion > 4 )
    {
        StringCchCopy(pszOS, BUFSIZE, TEXT("Microsoft "));

        // Test for the specific product.
        if (osvi.dwMajorVersion == 6)
        {
            if(osvi.dwMinorVersion == 0)
            {
                if(osvi.wProductType == VER_NT_WORKSTATION)
                    StringCchCat(pszOS, BUFSIZE, TEXT("Windows Vista "));
                else
                    StringCchCat(pszOS, BUFSIZE, TEXT("Windows Server 2008 "));
            }

            if (osvi.dwMinorVersion == 1)
            {
                if (osvi.wProductType == VER_NT_WORKSTATION)
                    StringCchCat(pszOS, BUFSIZE, TEXT("Windows 7 "));
                else
                    StringCchCat(pszOS, BUFSIZE, TEXT("Windows Server 2008 R2 "));
            }
         
            if (osvi.dwMinorVersion == 2 && osvi.wProductType == VER_NT_WORKSTATION)
                StringCchCat(pszOS, BUFSIZE, TEXT("Windows 8 "));

            pGPI = (PGPI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");
            pGPI(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);

            switch(dwType)
            {
            case PRODUCT_ULTIMATE:
                StringCchCat(pszOS, BUFSIZE, TEXT("Ultimate Edition" ));
                break;
            case PRODUCT_HOME_PREMIUM:
                StringCchCat(pszOS, BUFSIZE, TEXT("Home Premium Edition" ));
                break;
            case PRODUCT_HOME_BASIC:
                StringCchCat(pszOS, BUFSIZE, TEXT("Home Basic Edition" ));
                break;
            case PRODUCT_ENTERPRISE:
                StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise Edition" ));
                break;
            case PRODUCT_BUSINESS:
                StringCchCat(pszOS, BUFSIZE, TEXT("Business Edition" ));
                break;
            case PRODUCT_STARTER:
                StringCchCat(pszOS, BUFSIZE, TEXT("Starter Edition" ));
                break;
            case PRODUCT_CLUSTER_SERVER:
                StringCchCat(pszOS, BUFSIZE, TEXT("Cluster Server Edition" ));
                break;
            case PRODUCT_DATACENTER_SERVER:
                StringCchCat(pszOS, BUFSIZE, TEXT("Datacenter Edition" ));
                break;
            case PRODUCT_DATACENTER_SERVER_CORE:
                StringCchCat(pszOS, BUFSIZE, TEXT("Datacenter Edition (core installation)" ));
                break;
            case PRODUCT_ENTERPRISE_SERVER:
                StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise Edition" ));
                break;
            case PRODUCT_ENTERPRISE_SERVER_CORE:
                StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise Edition (core installation)" ));
                break;
            case PRODUCT_ENTERPRISE_SERVER_IA64:
                StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise Edition for Itanium-based Systems" ));
                break;
            case PRODUCT_SMALLBUSINESS_SERVER:
                StringCchCat(pszOS, BUFSIZE, TEXT("Small Business Server" ));
                break;
            case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
                StringCchCat(pszOS, BUFSIZE, TEXT("Small Business Server Premium Edition" ));
                break;
            case PRODUCT_STANDARD_SERVER:
                StringCchCat(pszOS, BUFSIZE, TEXT("Standard Edition" ));
                break;
            case PRODUCT_STANDARD_SERVER_CORE:
                StringCchCat(pszOS, BUFSIZE, TEXT("Standard Edition (core installation)" ));
                break;
            case PRODUCT_WEB_SERVER:
                StringCchCat(pszOS, BUFSIZE, TEXT("Web Server Edition" ));
                break;
            }
        }

        if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
        {
            if ( GetSystemMetrics(SM_SERVERR2) )
                StringCchCat(pszOS, BUFSIZE, TEXT( "Windows Server 2003 R2, "));
            else if ( osvi.wSuiteMask==VER_SUITE_STORAGE_SERVER )
                StringCchCat(pszOS, BUFSIZE, TEXT( "Windows Storage Server 2003"));
            else if ( osvi.wSuiteMask==0x00008000 )
                StringCchCat(pszOS, BUFSIZE, TEXT( "Windows Home Server"));
            else if( osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
                StringCchCat(pszOS, BUFSIZE, TEXT( "Windows XP Professional x64 Edition"));
            else
                StringCchCat(pszOS, BUFSIZE, TEXT("Windows Server 2003, "));

            // Test for the server type.
            if ( osvi.wProductType != VER_NT_WORKSTATION )
            {
                if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64 )
                {
                    if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                        StringCchCat(pszOS, BUFSIZE, TEXT( "Datacenter Edition for Itanium-based Systems" ));
                    else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                        StringCchCat(pszOS, BUFSIZE, TEXT( "Enterprise Edition for Itanium-based Systems" ));
                }
                else if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
                {
                    if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                        StringCchCat(pszOS, BUFSIZE, TEXT( "Datacenter x64 Edition" ));
                    else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                        StringCchCat(pszOS, BUFSIZE, TEXT( "Enterprise x64 Edition" ));
                    else StringCchCat(pszOS, BUFSIZE, TEXT( "Standard x64 Edition" ));
                }
                else
                {
                    if ( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER )
                        StringCchCat(pszOS, BUFSIZE, TEXT( "Compute Cluster Edition" ));
                    else if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                        StringCchCat(pszOS, BUFSIZE, TEXT( "Datacenter Edition" ));
                    else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                        StringCchCat(pszOS, BUFSIZE, TEXT( "Enterprise Edition" ));
                    else if ( osvi.wSuiteMask & VER_SUITE_BLADE )
                        StringCchCat(pszOS, BUFSIZE, TEXT( "Web Edition" ));
                    else StringCchCat(pszOS, BUFSIZE, TEXT( "Standard Edition" ));
                }
            }
        }

        if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
        {
            StringCchCat(pszOS, BUFSIZE, TEXT("Windows XP "));
            if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
                StringCchCat(pszOS, BUFSIZE, TEXT( "Home Edition" ));
            else
                StringCchCat(pszOS, BUFSIZE, TEXT( "Professional" ));
        }

        if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
        {
            StringCchCat(pszOS, BUFSIZE, TEXT("Windows 2000 "));
            if ( osvi.wProductType == VER_NT_WORKSTATION )
            {
                StringCchCat(pszOS, BUFSIZE, TEXT( "Professional" ));
            }
            else
            {
                if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                    StringCchCat(pszOS, BUFSIZE, TEXT( "Datacenter Server" ));
                else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                    StringCchCat(pszOS, BUFSIZE, TEXT( "Advanced Server" ));
                else StringCchCat(pszOS, BUFSIZE, TEXT( "Server" ));
            }
        }

        // Include service pack (if any) and build number.

        if( _tcslen(osvi.szCSDVersion) > 0 )
        {
            StringCchCat(pszOS, BUFSIZE, TEXT(" ") );
            StringCchCat(pszOS, BUFSIZE, osvi.szCSDVersion);
        }

        TCHAR buf[80];

        StringCchPrintf( buf, 80, TEXT(" (build %d)"), osvi.dwBuildNumber);
        StringCchCat(pszOS, BUFSIZE, buf);

        if ( osvi.dwMajorVersion >= 6 )
        {
            if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
            StringCchCat(pszOS, BUFSIZE, TEXT( ", 64-bit" ));
            else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL )
            StringCchCat(pszOS, BUFSIZE, TEXT(", 32-bit"));
        }

        return TRUE;
    }

    else
    {
        printf( "This sample does not support this version of Windows.\n");
        return FALSE;
    }
}

QString OsDisplayString()
{
    TCHAR szOS[BUFSIZE];
    if (GetOSDisplayString(szOS))
         return WStringToQString(szOS).simplified();
     else
         return "Unknown OS";
}

unsigned long long TotalSystemPhysicalMemory()
{
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    int ret = GlobalMemoryStatusEx(&statex);
    if (ret == 0)
        return 0;

    return (unsigned long long)statex.ullTotalPhys;
}

QString ProcessorBrandName()
{
    int CPUInfo[4] = {-1};

    // Calling __cpuid with 0x80000000 as the InfoType argument
    // gets the number of valid extended IDs.
    __cpuid(CPUInfo, 0x80000000);
    unsigned int nExIds = CPUInfo[0];

    if (nExIds < 0x80000004)
         return "Unknown";

     char CPUBrandString[0x40];
    memset(CPUBrandString, 0, sizeof(CPUBrandString));

    // Get the information associated with each extended ID.
    for (unsigned int i = 0x80000002; i <= nExIds && i <= 0x80000004; ++i)
    {
        __cpuid(CPUInfo, i);
        // Interpret CPU brand string and cache information.
        if  (i == 0x80000002)
            memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
        else if  (i == 0x80000003)
            memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
        else if  (i == 0x80000004)
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
    }

    return QString(CPUBrandString).simplified();
}

QString ProcessorCpuIdString()
{
    int CPUInfo[4] = {-1};

    // __cpuid with an InfoType argument of 0 returns the number of
    // valid Ids in CPUInfo[0] and the CPU identification string in
    // the other three array elements. The CPU identification string is
    // not in linear order. The code below arranges the information 
    // in a human readable form.
    __cpuid(CPUInfo, 0);
//    unsigned nIds = CPUInfo[0];
    char CPUString[0x20];
    memset(CPUString, 0, sizeof(CPUString));
    *((int*)CPUString) = CPUInfo[1];
    *((int*)(CPUString+4)) = CPUInfo[3];
    *((int*)(CPUString+8)) = CPUInfo[2];

    return CPUString;
}

QString ProcessorExtendedCpuIdInfo()
{
    int CPUInfo[4] = {-1};

    // __cpuid with an InfoType argument of 0 returns the number of
    // valid Ids in CPUInfo[0] and the CPU identification string in
    // the other three array elements. The CPU identification string is
    // not in linear order. The code below arranges the information 
    // in a human readable form.
    __cpuid(CPUInfo, 0);
    unsigned nIds = CPUInfo[0];
    char CPUString[0x20];
    memset(CPUString, 0, sizeof(CPUString));
    *((int*)CPUString) = CPUInfo[1];
    *((int*)(CPUString+4)) = CPUInfo[3];
    *((int*)(CPUString+8)) = CPUInfo[2];

     if (nIds == 0)
         return CPUString;

    __cpuid(CPUInfo, 1);

    int nSteppingID = CPUInfo[0] & 0xf;
    int nModel = (CPUInfo[0] >> 4) & 0xf;
    int nFamily = (CPUInfo[0] >> 8) & 0xf;
//    int nProcessorType = (CPUInfo[0] >> 12) & 0x3;
    int nExtendedmodel = (CPUInfo[0] >> 16) & 0xf;
    int nExtendedfamily = (CPUInfo[0] >> 20) & 0xff;
//    int nBrandIndex = CPUInfo[1] & 0xff;

    std::stringstream ss;
    ss << CPUString << ", " << "Stepping: " << nSteppingID << ", Model: " << nModel <<
        ", Family: " << nFamily << ", Ext.model: " << nExtendedmodel << ", Ext.family: " << nExtendedfamily << ".";

    return QString::fromStdString(ss.str());
}

unsigned long CpuSpeedFromRegistry(unsigned long dwCPU)
{
    HKEY hKey;
    DWORD dwSpeed;

    // Get the key name
    TCHAR szKey[256];
    _sntprintf_s(szKey, sizeof(szKey)/sizeof(TCHAR), TEXT("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\%d\\"), dwCPU);

    // Open the key
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,szKey, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
        return 0;

    // Read the value
    DWORD dwLen = 4;
    if(RegQueryValueEx(hKey, TEXT("~MHz"), NULL, NULL, (LPBYTE)&dwSpeed, &dwLen) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return 0;
    }

    // Cleanup and return
    RegCloseKey(hKey);
    return dwSpeed;
}

unsigned long TotalVideoMemory()
{
    unsigned long availableMemory = 0;
#if defined(DIRECTX_ENABLED)
// Windows Vista and up implementation with dxgi.
/// @todo Add <= Windows XP support.
#if WINVER >= 0x0600
    HINSTANCE dxgi = LoadLibrary(L"dxgi.dll");
    if (dxgi)
    {
        DXGICREATEFACTORY CreateFactoryFunction = (DXGICREATEFACTORY)GetProcAddress(dxgi, "CreateDXGIFactory");
        if (CreateFactoryFunction)
        {
            IDXGIFactory* factory = NULL;
            CreateFactoryFunction(__uuidof(IDXGIFactory), (LPVOID*)&factory);
            if (factory)
            {
                IDXGIAdapter* adapter = NULL;
                HRESULT hr = factory->EnumAdapters(0, &adapter);
                if (SUCCEEDED(hr) && adapter)
                {
                    DXGI_ADAPTER_DESC desc;
                    ZeroMemory(&desc, sizeof(DXGI_ADAPTER_DESC));
                    if (SUCCEEDED(adapter->GetDesc(&desc)))
                        availableMemory = desc.DedicatedVideoMemory;
                }
            }
        }
        FreeLibrary(dxgi);
    }
#endif
#endif
    return availableMemory;
}

#elif defined(__APPLE__)
#include <CoreServices/CoreServices.h>
#include <ApplicationServices/ApplicationServices.h>
#include <sys/sysctl.h>

QString OsDisplayString()
{
    s32 majorVersion, minorVersion, bugfixVersion;

    Gestalt(gestaltSystemVersionMajor, &majorVersion);
    Gestalt(gestaltSystemVersionMinor, &minorVersion);
    Gestalt(gestaltSystemVersionBugFix, &bugfixVersion);

    QString codename;
    switch(minorVersion)
    {
        case 5:
            codename = "Leopard";
            break;
        case 6:
            codename = "Snow Leopard";
            break;
        case 7:
            codename = "Lion";
            break;
        case 8:
            codename = "Mountain Lion";
            break;
        default:
            codename = "";
            break;
    }

    return QString("Mac OS X %1.%2.%3 (%4)").arg(majorVersion).arg(minorVersion).arg(bugfixVersion).arg(codename);
}

u64 TotalSystemPhysicalMemory()
{
    u64 memory;
    size_t len = sizeof(memory);
    sysctlbyname("hw.memsize", &memory, &len, NULL, 0);
    return memory;
}

QString ProcessorBrandName()
{
    char cpuBrandName[128];
    size_t len = sizeof(cpuBrandName);
    sysctlbyname("machdep.cpu.brand_string", cpuBrandName, &len, NULL, 0);
    return cpuBrandName;
}

QString ProcessorCpuIdString()
{
    char cpuIdString[128];
    size_t len = sizeof(cpuIdString);
    sysctlbyname("machdep.cpu.vendor", cpuIdString, &len, NULL, 0);
    return cpuIdString;
}

QString ProcessorExtendedCpuIdInfo()
{
    u32 stepping, extmodel, extfamily, model, family;
    size_t len = sizeof(u32);
    sysctlbyname("machdep.cpu.stepping", &stepping, &len, NULL, 0);
    sysctlbyname("machdep.cpu.extmodel", &extmodel, &len, NULL, 0);
    sysctlbyname("machdep.cpu.extfamily", &extfamily, &len, NULL, 0);
    sysctlbyname("machdep.cpu.model", &model, &len, NULL, 0);
    sysctlbyname("machdep.cpu.family", &family, &len, NULL, 0);

    std::stringstream fullString;
    fullString << ProcessorCpuIdString().toStdString() << ", Stepping: " << stepping << ", Model: " << model <<
    ", Family: " << family << ", Ext.model: " << extmodel << ", Ext.family: " << extfamily << ". " << CpuSpeedFromRegistry(0);
    return QString::fromStdString(fullString.str());
}

unsigned long CpuSpeedFromRegistry(unsigned long /*dwCPU*/)
{
    u64 speed;
    size_t len = sizeof(speed);
    sysctlbyname("hw.cpufrequency", &speed, &len, NULL, 0);
    return speed;
}

unsigned long TotalVideoMemory()
{
    long *videoMemory;
    CGError err = CGDisplayNoErr;
    uint i = 0;
    io_service_t *dspPorts = NULL;
    CGDirectDisplayID *displays = NULL;
    CGDisplayCount dspCount = 0;
    CFTypeRef typeCode;

    err = CGGetActiveDisplayList(0, NULL, &dspCount);
    displays = static_cast<CGDirectDisplayID*>(calloc((size_t)dspCount, sizeof(CGDirectDisplayID)));
    videoMemory = static_cast<long*>(calloc((size_t)dspCount, sizeof(long)));
    dspPorts = static_cast<io_service_t*>(calloc((size_t)dspCount, sizeof(io_service_t)));

    err = CGGetActiveDisplayList(dspCount, displays, &dspCount);

    for(i = 0; i < dspCount; i++)
    {
        dspPorts[i] = CGDisplayIOServicePort(displays[i]);
        typeCode = IORegistryEntryCreateCFProperty(dspPorts[i], CFSTR(kIOFBMemorySizeKey), kCFAllocatorDefault, kNilOptions);

        if(typeCode && CFGetTypeID(typeCode) == CFNumberGetTypeID())
        {
            CFNumberGetValue(static_cast<CFNumberRef>(typeCode), kCFNumberSInt32Type, videoMemory);
            if(typeCode)
                CFRelease(typeCode);
        }
    }

    return *videoMemory;
}
#else /// @todo Linux implementation
QString OsDisplayString() { return ""; }
unsigned long long TotalSystemPhysicalMemory() { return 0; }
QString ProcessorBrandName() { return ""; }
QString ProcessorCpuIdString() { return ""; }
QString ProcessorExtendedCpuIdInfo() { return ""; }
unsigned long CpuSpeedFromRegistry(unsigned long dwCPU) { return 0; }
unsigned long TotalVideoMemory() { return 0; }
#endif

