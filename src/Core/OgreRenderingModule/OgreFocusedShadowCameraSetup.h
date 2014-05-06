// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <OgreShadowCameraSetupFocused.h>
#include "FocusedGpuConstants.h"

/// focused shadow camera setup.
/** Mostly copy-paste from Ogre's PSSMShadowCameraSetup, the difference is that we are not using LiSPSM.
    @cond PRIVATE */
class OgreFocusedShadowCameraSetup : public Ogre::FocusedShadowCameraSetup
{
public:
    /// Constructor
    OgreFocusedShadowCameraSetup();

    /// Destructor.
    ~OgreFocusedShadowCameraSetup();

    /// Returns a shadow camera with PSSM splits base on iteration.
    virtual void getShadowCamera(const Ogre::SceneManager *sm, const Ogre::Camera *cam,const Ogre::Viewport *vp, const Ogre::Light *light, Ogre::Camera *texCam, size_t iteration) const;

private:
    Ogre::FocusedGpuConstants* _focusedGpuConstants;
};
/** @endcond */
