// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "OgreFocusedShadowCameraSetup.h"

OgreFocusedShadowCameraSetup::OgreFocusedShadowCameraSetup() :
    Ogre::FocusedShadowCameraSetup()
{
    _focusedGpuConstants = new Ogre::FocusedGpuConstants();
}

OgreFocusedShadowCameraSetup::~OgreFocusedShadowCameraSetup()
{
    SAFE_DELETE(_focusedGpuConstants);
}

void OgreFocusedShadowCameraSetup::getShadowCamera(const Ogre::SceneManager *sm, const Ogre::Camera *cam,const Ogre::Viewport *vp, const Ogre::Light *light, Ogre::Camera *texCam, size_t iteration) const
{
    // Use default focused shadow camera.
    Ogre::FocusedShadowCameraSetup::getShadowCamera(sm, cam, vp, light, texCam, iteration);

    // Update shadow matrix to shader uniforms
    _focusedGpuConstants->update(*texCam);
}
