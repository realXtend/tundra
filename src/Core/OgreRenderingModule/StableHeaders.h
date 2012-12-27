// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "Math/MathFwd.h"
#include "CoreDefines.h"
#include "LoggingFunctions.h"

// If PCH is disabled, leave the contents of this whole file empty to avoid any compilation unit getting any unnecessary headers.
///\todo Refactor the above #include inside this #ifdef as well.
#ifdef PCH_ENABLED

#include "CoreDefines.h"
#include "Framework.h"

#include <QtCore>
#include <QtGui>

// The following file is a 'include-it-all' convenience utility. Perfect for including it here in the PCH.
#include <Ogre.h>

// Seems the above one contains only the most used headers, so add a few more here in case they weren't contained in above.
#include <OgreMaterial.h>
#include <OgreCompositorInstance.h>
#include <OgreAnimationState.h>
#include <OgreColourValue.h>
#include <OgreVector3.h>
#include <OgreTexture.h>
#include <OgreMesh.h>
#include <OgreSkeleton.h>
#include <OgreRenderWindow.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreTexture.h>
#include <OgreOverlay.h>

#ifdef ANDROID
    
#endif

#endif
