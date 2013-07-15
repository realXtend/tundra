/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file TundraVersionInfo.h
    @brief Tundra version information definitions. You can overwrite these in the root CMakeLists.txt. */

#pragma once

/** @def TUNDRA_VERSION_MAJOR
    The Tundra SDK major version number. */
#ifndef TUNDRA_VERSION_MAJOR
#define TUNDRA_VERSION_MAJOR 2
#endif

/** @def TUNDRA_VERSION_MINOR
    The Tundra SDK minor version number. */
#ifndef TUNDRA_VERSION_MINOR
#define TUNDRA_VERSION_MINOR 5
#endif

/** @def TUNDRA_VERSION_MAJOR_PATCH
    The Tundra SDK major patch version number. */
#ifndef TUNDRA_VERSION_MAJOR_PATCH
#define TUNDRA_VERSION_MAJOR_PATCH 1
#endif

/** @def TUNDRA_VERSION_MINOR_PATCH
    The Tundra SDK minor patch version number. */
#ifndef TUNDRA_VERSION_MINOR_PATCH
#define TUNDRA_VERSION_MINOR_PATCH 0
#endif

/** @def TUNDRA_ORGANIZATION_NAME
    The Tundra organization name, "realXtend" by default. */
#ifndef TUNDRA_ORGANIZATION_NAME
#define TUNDRA_ORGANIZATION_NAME "realXtend"
#endif

/** @def TUNDRA_APPLICATION_NAME
    The Tundra application name, "Tundra" by default. */
#ifndef TUNDRA_APPLICATION_NAME
#define TUNDRA_APPLICATION_NAME "Tundra"
#endif

/** @def TUNDRA_VERSION_STRING
    The full Tundra version as a string, by default this is simply the version numbers concatenated, f.ex. "2.5.1.0". */
#ifndef TUNDRA_VERSION_STRING
#define STRINGIZE_HELPER(x) #x
#define STRINGIZE(x) STRINGIZE_HELPER(x)
#define TUNDRA_VERSION_STRING STRINGIZE(TUNDRA_VERSION_MAJOR)"."STRINGIZE(TUNDRA_VERSION_MINOR)"."STRINGIZE(TUNDRA_VERSION_MAJOR_PATCH)"."STRINGIZE(TUNDRA_VERSION_MINOR_PATCH)
#endif
