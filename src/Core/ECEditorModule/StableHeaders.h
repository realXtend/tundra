// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#define OGRE_INTEROP

#include "CoreTypes.h"
#include "LoggingFunctions.h"

// If PCH is disabled, leave the contents of this whole file empty to avoid any compilation unit getting any unnecessary headers.
///\todo Refactor the above #include inside this #ifdef as well.
#ifdef PCH_ENABLED

#include "CoreDefines.h"

#include "Framework.h"

#include <QtCore>
#include <QtGui>

#endif


