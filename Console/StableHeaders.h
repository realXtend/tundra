// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Console_StableHeaders_h
#define incl_Console_StableHeaders_h

#include "CoreTypes.h"

// If PCH is disabled, leave the contents of this whole file empty to avoid any compilation unit getting any unnecessary headers.
///\todo Refactor the above #include inside this #ifdef as well.
#ifdef PCH_ENABLED

#include "CoreDefines.h"
#include "Framework.h"

#include <QtCore>
#include <QtGui>

#endif

#endif

