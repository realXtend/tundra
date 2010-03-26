// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Core_h
#define incl_Core_h

// Add all includes here that contain interfaces for the Core
#include "CoreCompileConfig.h"
#include "CoreDefines.h"
#include "CoreTypes.h"

///\todo C4996 warnings caused by strcpy disabled for now. Fix usage of strcpy in CoreException.h.
#pragma warning( push )
#pragma warning( disable : 4996 )
#include "CoreException.h"
#pragma warning( pop )

//#include "CoreAnyIterator.h"

#include "CoreMath.h"
#include "Vector3D.h"
#include "Matrix4.h"
#include "Quaternion.h"
#include "Color.h"
#include "CoreStringUtils.h"
#include "CoreThread.h"


//! Core contains functionality and definitions that are common to all subprojects in the viewer.
/*! It contains common standard includes so you shouldn't need to include specific
    C++ standard library includes in your own code. It also contains
    includes for some of the external libraries used by the viewer, such as
    Boost or PoCo.

    Useful macros are defined, such as LOG(), so adding log statements is
    possible even if we might still lack an actual logger.

    For more information about useful defines, see CoreDefines.h

    Some common types are also specified by the Core, such as shortcuts
    to unsigned int or boost mutex lock guard.

    For more information see CoreTypes.h
*/
namespace Core {}

#endif

