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
#include <string>

#include "SystemInfo.h"

#if defined(LINUX) || defined(__APPLE__) || defined(ANDROID)

#include <string>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

std::string TrimLeft(std::string str)
{
    return str.substr(str.find_first_not_of(" \n\r\t"));
}

std::string TrimRight(std::string str)
{
    str.erase(str.find_last_not_of(" \n\r\t")+1);
    return str;
}

std::string Trim(std::string str)
{
    return TrimLeft(TrimRight(str));
}

// http://stackoverflow.com/questions/646241/c-run-a-system-command-and-get-output
std::string RunProcess(const char *cmd)
{
    FILE *fp = popen(cmd, "r");
    if (!fp)
        return std::string();

    std::stringstream ss;
    char str[1035];
    while(fgets(str, sizeof(str)-1, fp))
        ss << str;

    pclose(fp);

    return TrimRight(ss.str()); // Trim the end of the result to remove \n.
}

std::string FindLine(const std::string &inStr, const char *lineStart)
{
    int lineStartLen = strlen(lineStart);
    size_t idx = inStr.find(lineStart);
    if (idx == std::string::npos)
        return std::string();
    idx += lineStartLen;
    size_t lineEnd = inStr.find("\n", idx);
    if (lineEnd == std::string::npos)
        return inStr.substr(idx);
    else
        return inStr.substr(idx, lineEnd-idx);
}

#endif

#if defined(WIN32) && !defined(WIN8RT)

#include <windows.h>
#include <iphlpapi.h>

#include <tchar.h>
#include <stdio.h>

#include <sstream>
#include <iostream>

#ifdef _MSC_VER
#include <intrin.h>

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#elif defined (__GNUC__)
#include <cpuid.h>
#endif

#if defined(DIRECTX_ENABLED)
#if WINVER >= 0x0600
#include <dxgi.h>
typedef HRESULT (WINAPI* DXGICREATEFACTORY)(REFIID, void**);
#endif
#endif

std::string ReadRegistryKeyString(const char *registryKey, const char *registryValue)
{
    // Open the key
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, registryKey, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
        return 0;

    char str[256] = {};
    DWORD dwLen = 255;
    LONG ret = RegQueryValueExA(hKey, registryValue, NULL, NULL, (LPBYTE)str, &dwLen);
    RegCloseKey(hKey);

    if (ret == ERROR_SUCCESS)
        return str;
    else
        return std::string();
}

unsigned int ReadRegistryKeyU32(const char *registryKey, const char *registryValue)
{
    // Open the key
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, registryKey, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
        return 0;

    unsigned int value = 0;
    DWORD dwLen = 4;
    LONG ret = RegQueryValueExA(hKey, registryValue, NULL, NULL, (LPBYTE)&value, &dwLen);
    RegCloseKey(hKey);

    if (ret == ERROR_SUCCESS)
        return value;
    else
        return 0;
}

QString OsDisplayString()
{
    std::string productName = ReadRegistryKeyString("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\", "ProductName");
    std::string servicePack = ReadRegistryKeyString("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\", "CSDVersion");
    std::string bitness = ReadRegistryKeyString("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\", "BuildLabEx");
    if (bitness.find("amd64") != std::string::npos)
        bitness = "64-bit";
    else
        bitness = "32-bit";

    return QString::fromStdString(productName + " " + bitness + " " + servicePack);
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

static void CpuId(int *outInfo, int infoType)
{
#ifdef _MSC_VER
    __cpuid(outInfo, infoType);
#elif defined(__GNUC__)
    __get_cpuid((unsigned int)infoType, (unsigned int*)outInfo, (unsigned int*)outInfo+1, (unsigned int*)outInfo+2, (unsigned int*)outInfo+3);
#else
#warning CpuId not implemented for this compiler!
#endif
}

QString ProcessorBrandName()
{
    int CPUInfo[4] = {-1};

    // Calling __cpuid with 0x80000000 as the InfoType argument
    // gets the number of valid extended IDs.
    CpuId(CPUInfo, 0x80000000);
    unsigned int nExIds = CPUInfo[0];

    if (nExIds < 0x80000004)
         return "Unknown";

    char CPUBrandString[0x40];
    memset(CPUBrandString, 0, sizeof(CPUBrandString));

    // Get the information associated with each extended ID.
    for (unsigned int i = 0x80000002; i <= nExIds && i <= 0x80000004; ++i)
    {
        CpuId(CPUInfo, i);

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
    CpuId(CPUInfo, 0);
    char CPUString[13] = {};
    memcpy(CPUString, CPUInfo+1, sizeof(int));
    memcpy(CPUString+4, CPUInfo+3, sizeof(int));
    memcpy(CPUString+8, CPUInfo+2, sizeof(int));

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
    CpuId(CPUInfo, 0);
    unsigned nIds = CPUInfo[0];
    char CPUString[13] = {};
    memcpy(CPUString, CPUInfo+1, sizeof(int));
    memcpy(CPUString+4, CPUInfo+3, sizeof(int));
    memcpy(CPUString+8, CPUInfo+2, sizeof(int));

    if (nIds == 0)
        return CPUString;

    CpuId(CPUInfo, 1);

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

int MaxSimultaneousThreads()
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}

unsigned long CpuSpeedFromRegistry(unsigned long dwCPU)
{
    char str[256];
    sprintf(str, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\%lu\\", dwCPU);
    return (unsigned long)ReadRegistryKeyU32(str, "~MHz");
}

#elif defined(LINUX)

QString OsDisplayString()
{
    return QString::fromStdString(RunProcess("lsb_release -ds") + " " + RunProcess("uname -mrs"));
}

unsigned long long TotalSystemPhysicalMemory()
{
    std::string r = RunProcess("cat /proc/meminfo");
    std::string memTotal = FindLine(r, "MemTotal:");
    int mem = 0;
    int n = sscanf(memTotal.c_str(), "%d", &mem);
    if (n == 1)
        return (unsigned long long)mem * 1024;
    else
        return 0;
}

QString ProcessorBrandName()
{
    std::string r = RunProcess("cat /proc/cpuinfo");
    return QString::fromStdString(Trim(FindLine(FindLine(r, "vendor_id"),":")));
}

QString ProcessorCpuIdString()
{
    std::string r = RunProcess("cat /proc/cpuinfo");
    return QString::fromStdString(Trim(FindLine(FindLine(r, "model name"),":")));
}

QString ProcessorExtendedCpuIdInfo()
{
    std::string r = RunProcess("cat /proc/cpuinfo");
    std::string stepping = Trim(FindLine(FindLine(r, "stepping"),":"));
    std::string model = Trim(FindLine(FindLine(r, "model"),":"));
    std::string family = Trim(FindLine(FindLine(r, "cpu family"),":"));

    std::stringstream ss;
    ss << ProcessorBrandName() << ", " << "Stepping: " << stepping << ", Model: " << model <<
        ", Family: " << family;
    return QString::fromStdString(ss.str());
}

int MaxSimultaneousThreads()
{
    std::string r = RunProcess("lscpu");
    r = TrimRight(FindLine(r, "CPU(s):"));
    int numCPUs = 0;
    int n = sscanf(r.c_str(), "%d", &numCPUs);
    return (n == 1) ? numCPUs : 0;
}

unsigned long CpuSpeedFromRegistry(unsigned long /*dwCPU*/)
{
    std::string r = RunProcess("lscpu");
    r = TrimRight(FindLine(r, "CPU MHz:"));
    int mhz = 0;
    int n = sscanf(r.c_str(), "%d", &mhz);
    return (n == 1) ? (unsigned long)mhz : 0;
}

#elif defined(__APPLE__)

QString OsDisplayString()
{
    std::string uname = RunProcess("uname -mrs");

    // http://stackoverflow.com/questions/11072804/mac-os-x-10-8-replacement-for-gestalt-for-testing-os-version-at-runtime/11697362#11697362
    std::string systemVer = RunProcess("cat /System/Library/CoreServices/SystemVersion.plist");
    size_t idx = systemVer.find("<key>ProductVersion</key>");
    if (idx == std::string::npos)
        return uname;
    idx = systemVer.find("<string>", idx);
    if (idx == std::string::npos)
        return uname;
    idx += strlen("<string>");
    size_t endIdx = systemVer.find("</string>", idx);
    if (endIdx == std::string::npos)
        return uname;
    std::string marketingVersion = Trim(systemVer.substr(idx, endIdx-idx));

    uname += " Mac OS X " + marketingVersion;
    int majorVer = 0, minorVer = 0;
    int n = sscanf(marketingVersion.c_str(), "%d.%d", &majorVer, &minorVer);
    if (n != 2)
        return uname;
    switch (majorVer * 100 + minorVer)
    {
    case 1001: uname = uname + " Puma"; break;
    case 1002: uname = uname + " Jaguar"; break;
    case 1003: uname = uname + " Panther"; break;
    case 1004: uname = uname + " Tiger"; break;
    case 1005: uname = uname + " Leopard"; break;
    case 1006: uname = uname + " Snow Leopard"; break;
    case 1007: uname = uname + " Lion"; break;
    case 1008: uname = uname + " Mountain Lion"; break;
    }
    return QString::fromStdString(uname);
}

#include <sys/types.h>
#include <sys/sysctl.h>

std::string sysctl_string(const char *sysctl_name)
{
    char str[128] = {};
    size_t size = sizeof(str)-1;
    sysctlbyname(sysctl_name, str, &size, NULL, 0);
    return str;
}

int sysctl_int32(const char *sysctl_name)
{
    int32_t val = 0;
    size_t size = sizeof(val);
    sysctlbyname(sysctl_name, &val, &size, NULL, 0);
    return (int)val;
}

int64_t sysctl_int64(const char *sysctl_name)
{
    int64_t val = 0;
    size_t size = sizeof(val);
    sysctlbyname(sysctl_name, &val, &size, NULL, 0);
    return val;
}

unsigned long long TotalSystemPhysicalMemory()
{
    return (unsigned long long)sysctl_int64("hw.memsize");
}

QString ProcessorBrandName()
{
    return QString::fromStdString(sysctl_string("machdep.cpu.vendor"));
}

QString ProcessorCpuIdString()
{
    return QString::fromStdString(sysctl_string("machdep.cpu.brand_string"));
}

QString ProcessorExtendedCpuIdInfo()
{
    char str[1024];
    sprintf(str, "%s, Stepping: %d, Model: %d, Family: %d, Ext.model: %d, Ext.family: %d.", ProcessorCpuIdString().c_str(), sysctl_int32("machdep.cpu.stepping"), sysctl_int32("machdep.cpu.model"), sysctl_int32("machdep.cpu.family"), sysctl_int32("machdep.cpu.extmodel"), sysctl_int32("machdep.cpu.extfamily"));
    return str;
}

unsigned long CpuSpeedFromRegistry(unsigned long /*dwCPU*/)
{
    int64_t freq = sysctl_int64("hw.cpufrequency");
    return (unsigned long)(freq / 1000 / 1000);
}

// Returns the maximum number of threads the CPU can simultaneously accommodate.
// E.g. for dualcore hyperthreaded Intel CPUs, this returns 4.
int MaxSimultaneousThreads()
{
    return (int)sysctl_int32("machdep.cpu.thread_count");
}

#elif defined(ANDROID)

unsigned long long TotalSystemPhysicalMemory()
{
    std::string r = RunProcess("cat /proc/meminfo");
    std::string memTotal = FindLine(r, "MemTotal:");
    int mem = 0;
    int n = sscanf(memTotal.c_str(), "%d", &mem);
    if (n == 1)
        return (unsigned long long)mem * 1024;
    else
        return 0;
}

QString OsDisplayString()
{
    std::string ver = Trim(RunProcess("getprop ro.build.version.release"));
    std::string apiLevel = Trim(RunProcess("getprop ro.build.version.sdk"));
    if (apiLevel.empty())
        apiLevel = Trim(RunProcess("getprop ro.build.version.sdk_int"));

    std::string os;
    if (!ver.empty())
        os = "Android " + ver + " ";
    if (!apiLevel.empty())
        os += "SDK API Level " + apiLevel + " ";
    os += Trim(RunProcess("getprop ro.build.description"));
    return QString::fromStdString(os);
}

QString ProcessorBrandName()
{
    std::string r = RunProcess("cat /proc/cpuinfo");
    return QString::fromStdString(Trim(FindLine(FindLine(r, "Processor"),":")));
}

QString ProcessorCpuIdString()
{
    // Note: This is actually HW identifier, not CPU ID.
    std::string manufacturer = RunProcess("getprop ro.product.manufacturer");
    std::string brand = RunProcess("getprop ro.product.brand");
    std::string model = RunProcess("getprop ro.product.model");
    std::string board = RunProcess("getprop ro.product.board");
    std::string device = RunProcess("getprop ro.product.device");
    std::string name = RunProcess("getprop ro.product.name");
    return QString::fromStdString(manufacturer + " " + brand + " " + model + " " + board + " " + device + " " + name);
}

QString ProcessorExtendedCpuIdInfo()
{
    std::string r = RunProcess("cat /proc/cpuinfo");
    std::string implementer = Trim(FindLine(FindLine(r, "CPU implementer"),":"));
    std::string arch = Trim(FindLine(FindLine(r, "CPU architecture"),":"));
    std::string variant = Trim(FindLine(FindLine(r, "CPU variant"),":"));
    std::string part = Trim(FindLine(FindLine(r, "CPU part"),":"));
    std::string rev = Trim(FindLine(FindLine(r, "CPU revision"),":"));
    std::string hw = Trim(FindLine(FindLine(r, "Hardware"),":"));

    std::stringstream ss;
    ss << "Hardware: " << hw << ", CPU implementer: " << implementer << ", arch: " << arch << ", variant: " << variant 
        << ", part: " << part << ", revision: " << rev;
    return QString::fromStdString(ss.str());
}

unsigned long CpuSpeedFromRegistry(unsigned long dwCPU)
{
    std::stringstream ss;
    ss << "cat /sys/devices/system/cpu/cpu" << dwCPU << "/cpufreq/cpuinfo_max_freq";
    std::string r = RunProcess(ss.str().c_str());
    int freq = 0;
    int n = sscanf(r.c_str(), "%d", &freq);
    if (n == 1)
        return freq / 1000; // cpuinfo_max_freq seems to be in kHz. Output MHz.
    else
        return 0;
}

int MaxSimultaneousThreads()
{
    std::string r = RunProcess("cat /sys/devices/system/cpu/present");
    int nCoresMin = 0, nCoresMax = 0;
    int n = sscanf(r.c_str(), "%d-%d", &nCoresMin, &nCoresMax);
    if (n == 2)
        return nCoresMax - nCoresMin + 1; // The min-max numbers are indices to cpu cores, so +1 for the count.
    else
        return 1;
}

#else

#ifdef _MSC_VER
#pragma WARNING("SystemInfo.cpp not implemented for the current platform!")
#else
#warning SystemInfo.cpp not implemented for the current platform!
#endif

QString OsDisplayString() { return ""; }
unsigned long long TotalSystemPhysicalMemory() { return 0; }
QString ProcessorBrandName() { return ""; }
QString ProcessorCpuIdString() { return ""; }
QString ProcessorExtendedCpuIdInfo() { return ""; }
unsigned long CpuSpeedFromRegistry(unsigned long /*dwCPU*/) { return 0; }
int MaxSimultaneousThreads() { return 0; }

#endif
