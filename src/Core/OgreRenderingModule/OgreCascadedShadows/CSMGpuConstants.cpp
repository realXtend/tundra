/*  Copyright 2010-2012 Matthew Paul Reid
    This file is subject to the terms and conditions defined in
    file 'License.txt', which is part of this source code package.
*/

#include "StableHeaders.h"

#include "CSMGpuConstants.h"

#include <qdebug.h>

namespace Ogre {

const Matrix4 PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE(
    0.5,    0,    0,  0.5, 
    0,   -0.5,    0,  0.5, 
    0,      0,    1,    0,
    0,      0,    0,    1);


CSMGpuConstants::CSMGpuConstants(size_t cascadeCount)
{
    mParamsScaleBias = GpuProgramManager::getSingletonPtr()->getSharedParameters("params_shadowMatrixScaleBias");
    Ogre::GpuConstantDefinitionMap scaleBiasConstants = mParamsScaleBias->getConstantDefinitions().map;

    for (size_t i=1; i < cascadeCount; i++)
    {
        std::string constantName = "texMatrixScaleBias" + StringConverter::toString(i);
        bool hasConstant = false;
        for(Ogre::GpuConstantDefinitionMap::iterator it =  scaleBiasConstants.begin(); it != scaleBiasConstants.end(); ++it)
        {
            if(it->first == constantName)
                hasConstant = true;
        }
        if(!hasConstant)
            mParamsScaleBias->addConstantDefinition(constantName, GCT_FLOAT4);
    }

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

void CSMGpuConstants::updateCascade(const Ogre::Camera &texCam, size_t index)
{
    if (index == 0)
    {
        mFirstCascadeViewMatrix = texCam.getViewMatrix();
        mFirstCascadeCamWidth = texCam.getOrthoWindowWidth();
        mViewRange = texCam.getFarClipDistance() - texCam.getNearClipDistance();

        Matrix4 texMatrix0 = PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE * texCam.getProjectionMatrixWithRSDepth() * mFirstCascadeViewMatrix;

        mParamsShadowMatrix->setNamedConstant("texMatrix0", texMatrix0);
    }
    else
    {
        Matrix4 mat0 = mFirstCascadeViewMatrix;
        Matrix4 mat1 = texCam.getViewMatrix();

        Vector3 mat0trans = mat0.getTrans();
        Vector3 mat1trans = mat1.getTrans();

        Vector3 offset = mat1trans - mat0trans;

        Real width0 = mFirstCascadeCamWidth;
        Real width1 = texCam.getOrthoWindowWidth();
        
        Real oneOnWidth = 1.0f / width0;
        Real offCenter = width1 / (2.0f * width0) - 0.5f;

        RenderSystem* rs = Ogre::Root::getSingletonPtr()->getRenderSystem();
        float depthRange = Math::Abs(rs->getMinimumDepthInputValue() -  rs->getMaximumDepthInputValue());

        Vector4 result;
        result.x = offset.x * oneOnWidth + offCenter;
        result.y = -offset.y * oneOnWidth + offCenter;
        result.z = -depthRange * offset.z / mViewRange;
        result.w = width0 / width1;

        mParamsScaleBias->setNamedConstant("texMatrixScaleBias" + StringConverter::toString(index), result);
    }
}

} // namespace Ogre
