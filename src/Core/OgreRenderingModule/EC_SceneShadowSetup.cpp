/**
    @author Adminotech Ltd.

    Copyright Adminotech Ltd.
    All rights reserved.

    @file   EC_MeshmoonShadowSetup.cpp
    @brief   */

#include "StableHeaders.h"
#include "EC_SceneShadowSetup.h"

#include "AttributeMetadata.h"
#include "Framework.h"
#include "Scene.h"
#include "Math/MathFunc.h"

#include "OgreCascadedShadows/ShadowCameraSetupStableCSM.h"
#include "OgreFocusedShadowCameraSetup.h"

#include <Ogre.h>
#include <OgreGpuProgramManager.h>

EC_SceneShadowSetup::EC_SceneShadowSetup(Scene* scene) :
    IComponent(scene),
    INIT_ATTRIBUTE_VALUE(splitLambda, "Split lambda", 0.93f),
    INIT_ATTRIBUTE_VALUE(firstSplitDist, "First split distance", 8.5f),
    INIT_ATTRIBUTE_VALUE(farDist, "Far distance", 250.0f),
    INIT_ATTRIBUTE_VALUE(fadeDist, "Fade distance", 50.0f),
    INIT_ATTRIBUTE_VALUE(depthBias1, "Depth bias 1", 0.05f),
    INIT_ATTRIBUTE_VALUE(depthBias2, "Depth bias 2", 0.05f),
    INIT_ATTRIBUTE_VALUE(depthBias3, "Depth bias 3", 0.05f),
    INIT_ATTRIBUTE_VALUE(depthBias4, "Depth bias 4", 0.05f)
{
    static AttributeMetadata depthBiasMetadata;
    depthBiasMetadata.step = "0.01";
    depthBias1.SetMetadata(&depthBiasMetadata);
    depthBias2.SetMetadata(&depthBiasMetadata);
    depthBias3.SetMetadata(&depthBiasMetadata);
    depthBias4.SetMetadata(&depthBiasMetadata);

    connect(this, SIGNAL(ParentEntitySet()), SLOT(OnParentEntitySet()));
}

EC_SceneShadowSetup::~EC_SceneShadowSetup()
{
}

void EC_SceneShadowSetup::OnParentEntitySet()
{
    world_ = ParentScene()->Subsystem<OgreWorld>();
    if (!framework->IsHeadless())
    {
        try
        {
            shadowParams_ = Ogre::GpuProgramManager::getSingletonPtr()->getSharedParameters("params_shadowParams");
        }
        catch(const Ogre::Exception& e)
        {
            LogError("[EC_SceneShadowSetup]: Failed to get shader shadow parameters from Ogre: " + e.getFullDescription());
            return;
        }
    }
}

Ogre::SceneManager* EC_SceneShadowSetup::OgreSceneManager() const
{
    return (!world_.expired() ? world_.lock()->OgreSceneManager() : 0);
}

void EC_SceneShadowSetup::AttributesChanged()
{
    if (framework->IsHeadless() || !shadowParams_.get())
        return;

    if (fadeDist.ValueChanged())
        CheckAndSetFadeDistance();
    if (depthBias1.ValueChanged() || depthBias2.ValueChanged() || depthBias3.ValueChanged() || depthBias4.ValueChanged())
    {
        // Adjust the range back from UI values to what the shadow params use
        Ogre::Vector4 value(depthBias1.Get(), depthBias2.Get(), depthBias3.Get(), depthBias4.Get());
        shadowParams_->setNamedConstant("fixedDepthBias", value * 0.0001f);
    }

    if (farDist.ValueChanged() || firstSplitDist.ValueChanged() || splitLambda.ValueChanged())
        UpdateShadowSetup();
}

void EC_SceneShadowSetup::CheckAndSetFadeDistance(bool onlyCheck)
{
    bool changed = false;
    float value = fadeDist.Get();
    if (fadeDist.Get() < 0.0f)
    {
        changed = true;
        value = 0.0f;
        fadeDist.Set(value, AttributeChange::Disconnected);
    }
    else if (fadeDist.Get() > farDist.Get())
    {
        changed = true;
        value = Max<float>(0.000001f, farDist.Get() * 0.04);
        fadeDist.Set(Max<float>(0.000001f, farDist.Get() * 0.04), AttributeChange::Disconnected);
    }
    if (!onlyCheck || changed)
        shadowParams_->setNamedConstant("shadowFadeDist", value);
}

void EC_SceneShadowSetup::UpdateShadowSetup()
{
    Ogre::SceneManager* sceneManager = OgreSceneManager();
    if (!sceneManager)
        return;

    // Set far distance
    sceneManager->setShadowFarDistance(farDist.Get());

    // Set far distance
    shadowParams_->setNamedConstant("shadowMaxDist", farDist.Get());

    CheckAndSetFadeDistance(true);

    Ogre::StableCSMShadowCameraSetup* shadowSetup = dynamic_cast<Ogre::StableCSMShadowCameraSetup*>(sceneManager->getShadowCameraSetup().get());
    if (!shadowSetup)
        return;

    // Calculate new split points
    shadowSetup->calculateSplitPoints(sceneManager->getShadowTextureCount(), firstSplitDist.Get(), farDist.Get(), splitLambda.Get());
}
