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

/// Returns the operating system information, for example "Microsoft Windows 8 (build 9200), 64-bit".
/** @note Currently only implemented on Windows. */
QString TUNDRACORE_API OsDisplayString();

/// Returns the total amount of available physical memory in bytes.
/** @note Currently only implemented on Windows. */
unsigned long long TUNDRACORE_API TotalSystemPhysicalMemory();

/// Returns A readable string of the processor as given by cpuid, like "AMD Turion(tm) X2 Dual-Core Mobile RM-74".
/** @note Currently only implemented on Windows. */
QString TUNDRACORE_API ProcessorBrandName();

/// Returns "GenuineIntel" or "AuthenticAMD" or something similar.
/** @note Currently only implemented on Windows. */
QString TUNDRACORE_API ProcessorCpuIdString();

/// Returns technical information about the processor.
/** @note Currently only implemented on Windows. */
QString TUNDRACORE_API ProcessorExtendedCpuIdInfo();

/// Returns the clock speed of the given core, in MHz.
/** @note This is nominal, not actual. The value is read from registry.
    @param coreIndex Core index, [0, NumberOfCores[
    @note Currently only implemented on Windows. */
unsigned long TUNDRACORE_API CpuSpeedFromRegistry(unsigned long coreIndex);

/// Returns the total amount of available graphics card memory in bytes.
/** @note Currently only implemented on Windows.
 ** @note Windows implementation will return 0 if DIRECTX_ENABLED is not defined or if platform < Windows Vista. */
unsigned long TUNDRACORE_API TotalVideoMemory();
