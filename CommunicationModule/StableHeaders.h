// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CommunicationModule_StableHeaders_h
#define incl_CommunicationModule_StableHeaders_h

#include "CoreStdIncludes.h"

// If PCH is disabled, leave the contents of this whole file empty to avoid any compilation unit getting any unnecessary headers.
///\todo Refactor the above #include inside this #ifdef as well.
#ifdef PCH_ENABLED

#include "Core.h"
#include "Foundation.h"

#include "Framework.h"

#include <QtCore>
#include <QtGui>

#endif

#endif // incl_CommunicationModule_StableHeaders_h
