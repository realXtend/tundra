/*  Copyright 2010-2012 Matthew Paul Reid
    This file is subject to the terms and conditions defined in
    file 'License.txt', which is part of this source code package.
*/

#include "StableHeaders.h"
#include "FocusedGpuConstants.h"

namespace Ogre
{
const Matrix4 PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE(
    0.5,    0,    0,  0.5, 
    0,   -0.5,    0,  0.5, 
    0,      0,    1,    0,
    0,      0,    0,    1);

FocusedGpuConstants::FocusedGpuConstants()
{
    mParamsShadowMatrix = GpuProgramManager::getSingletonPtr()->getSharedParameters("params_shadowTextureMatrix");

    Ogre::GpuConstantDefinitionMap matrixConstants = mParamsShadowMatrix->getConstantDefinitions().map;

    bool hasConstant = false;
    for(Ogre::GpuConstantDefinitionMap::iterator it =  matrixConstants.begin(); it != matrixConstants.end(); ++it)
    {
        if(it->first == std::string("texMatrix0"))
            hasConstant = true;
    }
    if(!hasConstant)
        mParamsShadowMatrix->addConstantDefinition("texMatrix0", GCT_MATRIX_4X4);
}

void FocusedGpuConstants::update(const Ogre::Camera &texCam)
{
    Matrix4 texMatrix0 = PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE * texCam.getProjectionMatrixWithRSDepth() * texCam.getViewMatrix();
    mParamsShadowMatrix->setNamedConstant("texMatrix0", texMatrix0);
}

} // namespace Ogre
