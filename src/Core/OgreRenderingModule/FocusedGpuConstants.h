/*  Copyright 2010-2012 Matthew Paul Reid
    This file is subject to the terms and conditions defined in
    file 'License.txt', which is part of this source code package.
*/
#pragma once

#include "Ogre.h"

namespace Ogre
{

class FocusedGpuConstants
{
public:
    FocusedGpuConstants();
    void update(const Ogre::Camera &texCam);

private:
    Ogre::GpuSharedParametersPtr mParamsShadowMatrix;
};

} // namespace Ogre
