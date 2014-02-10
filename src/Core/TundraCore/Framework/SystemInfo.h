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

/** @file   SystemInfo.h
    @brief  Utility functions for retrieving system information.
    @note   Code originally from MathGeoLib and modified for Tundra usage. */

#pragma once

#include "TundraCoreApi.h"

#include <QString>

#ifdef __APPLE__
/// Retrieves Mac OS X version info
/** @param majorVersion OSX major version
    @param minorVersion OSX minor version
    @param bugfixVersion OSX bugfix version */
void TUNDRACORE_API OSXVersionInfo(s32 *majorVersion, s32 *minorVersion, s32 *bugfixVersion);
#endif

/// Returns the operating system information, for example "Microsoft Windows 8 (build 9200), 64-bit" or "Mac OS X 10.8.0 (Mountain Lion)".
QString TUNDRACORE_API OsDisplayString();

/// Returns the total amount of available physical memory in bytes.
unsigned long long TUNDRACORE_API TotalSystemPhysicalMemory();

/// Returns A readable string of the processor as given by cpuid, like "AMD Turion(tm) X2 Dual-Core Mobile RM-74".
QString TUNDRACORE_API ProcessorBrandName();

/// Returns "GenuineIntel" or "AuthenticAMD" or something similar.
QString TUNDRACORE_API ProcessorCpuIdString();

/// Returns technical information about the processor.
QString TUNDRACORE_API ProcessorExtendedCpuIdInfo();

/// Returns the clock speed of the given core, in MHz.
/** @note This is nominal, not actual. The value is read from registry.
    @param coreIndex Core index, [0, NumberOfCores[ */
unsigned long TUNDRACORE_API CpuSpeedFromRegistry(unsigned long coreIndex);

/// Returns the total amount of available graphics card memory in bytes.
/** @note Currently only implemented on Windows.
 ** @note Windows implementation will return 0 if DIRECTX_ENABLED is not defined or if platform < Windows Vista. */
unsigned long TUNDRACORE_API TotalVideoMemory();

/// Returns the maximum number of threads the CPU can simultaneously accommodate.
int TUNDRACORE_API MaxSimultaneousThreads();
