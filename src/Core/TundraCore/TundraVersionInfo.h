/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file TundraVersionInfo.h
    @brief Tundra version information definitions. You can overwrite these in the root CMakeLists.txt. */

#pragma once

/** @def TUNDRA_VERSION_MAJOR
    The Tundra application's major version number. */
#ifndef TUNDRA_VERSION_MAJOR
#define TUNDRA_VERSION_MAJOR 2
#endif

/** @def TUNDRA_VERSION_MINOR
    The Tundra application's minor version number. */
#ifndef TUNDRA_VERSION_MINOR
#define TUNDRA_VERSION_MINOR 5
#endif

/** @def TUNDRA_VERSION_MAJOR_PATCH
    The Tundra application's major patch version number. */
#ifndef TUNDRA_VERSION_MAJOR_PATCH
#define TUNDRA_VERSION_MAJOR_PATCH 1
#endif

/** @def TUNDRA_VERSION_MINOR_PATCH
    The Tundra application's minor patch version number. */
#ifndef TUNDRA_VERSION_MINOR_PATCH
#define TUNDRA_VERSION_MINOR_PATCH 0
#endif

/** @def TUNDRA_ORGANIZATION_NAME
    The Tundra application's organization name, "realXtend" by default. */
#ifndef TUNDRA_ORGANIZATION_NAME
#define TUNDRA_ORGANIZATION_NAME "realXtend"
#endif

/** @def TUNDRA_APPLICATION_NAME
    The Tundra application name, "Tundra" by default. */
#ifndef TUNDRA_APPLICATION_NAME
#define TUNDRA_APPLICATION_NAME "Tundra"
#endif

/** @def TUNDRA_VERSION_STRING
    The Tundra application's version as a string, by default this is simply the version numbers concatenated while omitting possible trailing zeros, f.ex. "2.5.1".
    @note This is the authoritative source of the Application's version information.*/
#ifndef TUNDRA_VERSION_STRING
#define TUNDRA_VERSION_STRING "2.5.1"
#endif
