/**
    @author Adminotech Ltd.

    Copyright Adminotech Ltd.
    All rights reserved.

    @file   EC_MeshmoonShadowSetup.cpp
    @brief   */

#include "StableHeaders.h"
#include "EC_OgreShadowSetup.h"

#include "AttributeMetadata.h"
#include "Framework.h"
#include "Scene.h"

#include "OgreCascadedShadows/ShadowCameraSetupStableCSM.h"

#include <Ogre.h>
#include <OgreGpuProgramManager.h>

EC_OgreShadowSetup::EC_OgreShadowSetup(Scene* scene) :
    IComponent(scene),
    INIT_ATTRIBUTE_VALUE(splitLambda, "Split lambda", 0.93f),
    INIT_ATTRIBUTE_VALUE(firstSplitDist, "First split distance", 8.5f),
    INIT_ATTRIBUTE_VALUE(farDist, "Far distance", 500.0f),
    INIT_ATTRIBUTE_VALUE(fadeDist, "Fade distance", 300.0f),
    INIT_ATTRIBUTE_VALUE(depthBias1, "Depth bias 1", 0.05f),
    INIT_ATTRIBUTE_VALUE(depthBias2, "Depth bias 2", 0.05f),
    INIT_ATTRIBUTE_VALUE(depthBias3, "Depth bias 3", 0.05f),
    INIT_ATTRIBUTE_VALUE(depthBias4, "Depth bias 4", 0.05f)
{
    if (framework->IsHeadless())
        return;

    static AttributeMetadata depthBiasMetadata;
    depthBiasMetadata.step = "0.01";
    depthBias1.SetMetadata(&depthBiasMetadata);
    depthBias2.SetMetadata(&depthBiasMetadata);
    depthBias3.SetMetadata(&depthBiasMetadata);
    depthBias4.SetMetadata(&depthBiasMetadata);

    try
    {
        shadowParams_ = Ogre::GpuProgramManager::getSingletonPtr()->getSharedParameters("params_shadowParams");
    }
    catch(const Ogre::Exception& e)
    {
        LogError("[EC_OgreShadowSetup]: Failed to get shader shadow parameters from Ogre: " + e.getFullDescription());
        return;
    }

    if (scene)
        world_ = scene->Subsystem<OgreWorld>();
    else
        connect(this, SIGNAL(ParentEntitySet()), SLOT(OnParentEntitySet()));
}

EC_OgreShadowSetup::~EC_OgreShadowSetup()
{
}

void EC_OgreShadowSetup::OnParentEntitySet()
{
    if (world_.expired())
        world_ = ParentScene()->Subsystem<OgreWorld>();
}

Ogre::SceneManager* EC_OgreShadowSetup::OgreSceneManager() const
{
    OgreWorld *ogreWorld = world_.lock().get();
    return (ogreWorld ? ogreWorld->OgreSceneManager() : 0);
}

void EC_OgreShadowSetup::AttributesChanged()
{
    if (framework->IsHeadless() || !shadowParams_.get())
        return;

    if (fadeDist.ValueChanged())
    {
        if (fadeDist.Get() < 0.0f)
            fadeDist.Set(0.0f, AttributeChange::LocalOnly);
        else if (fadeDist.Get() >= farDist.Get())
            fadeDist.Set(farDist.Get() * 0.04, AttributeChange::LocalOnly);
        shadowParams_->setNamedConstant("shadowFadeDist", fadeDist.Get());
    }
    if (depthBias1.ValueChanged() || depthBias2.ValueChanged() || depthBias3.ValueChanged() || depthBias4.ValueChanged())
    {
        // Adjust the range back from UI values to what the shadow params use
        Ogre::Vector4 value(depthBias1.Get(), depthBias2.Get(), depthBias3.Get(), depthBias4.Get());
        shadowParams_->setNamedConstant("fixedDepthBias", value * 0.0001f);
    }

    if (farDist.ValueChanged() || firstSplitDist.ValueChanged() || splitLambda.ValueChanged())
        UpdateShadowSetup();
}

void EC_OgreShadowSetup::UpdateShadowSetup()
{
    Ogre::SceneManager* sceneManager = OgreSceneManager();
    if (!sceneManager)
        return;

    Ogre::StableCSMShadowCameraSetup* shadowSetup = dynamic_cast<Ogre::StableCSMShadowCameraSetup*>(sceneManager->getShadowCameraSetup().get());
    if (!shadowSetup)
        return;
    
    // Calculate new split points
    float lambda = splitLambda.Get();
    shadowSetup->calculateSplitPoints(sceneManager->getShadowTextureCount(), firstSplitDist.Get(), farDist.Get(), lambda);

    // Sanity check on fade distance. This will trigger an attribute 
    // change and be handled in AttributesChanged().
    if (fadeDist.Get() >= farDist.Get())
        fadeDist.Set(farDist.Get() * 0.04, AttributeChange::LocalOnly);

    // Set far distance
    shadowParams_->setNamedConstant("shadowMaxDist", farDist.Get());

    // Set far distance
    sceneManager->setShadowFarDistance(farDist.Get());
}
