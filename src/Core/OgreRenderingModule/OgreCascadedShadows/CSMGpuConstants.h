/*  Copyright 2010-2012 Matthew Paul Reid
    This file is subject to the terms and conditions defined in
    file 'License.txt', which is part of this source code package.
*/
#ifndef CSM_GPU_CONSTANTS_H
#define CSM_GPU_CONSTANTS_H

#include "Ogre.h"

namespace Ogre
{

class CSMGpuConstants
{
public:
    CSMGpuConstants(size_t cascadeCount);
    void updateCascade(const Ogre::Camera &texCam, size_t index);

private:
    Ogre::GpuSharedParametersPtr mParamsScaleBias;
    Ogre::GpuSharedParametersPtr mParamsShadowMatrix;

    Ogre::Matrix4 mFirstCascadeViewMatrix;
    Ogre::Real mFirstCascadeCamWidth;
    Ogre::Real mViewRange;
};

} // namespace Ogre

#endif // CSM_GPU_CONSTANTS_H
