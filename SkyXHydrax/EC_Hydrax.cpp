/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Hydrax.cpp
 *  @brief  A photorealistic water plane component using Hydrax, http://www.ogre3d.org/tikiwiki/Hydrax
 */

#include "DebugOperatorNew.h"

#include "EC_Hydrax.h"

#include "Scene.h"
#include "Framework.h"
#include "FrameAPI.h"
#include "OgreWorld.h"
#include "Renderer.h"
#include "EC_Camera.h"
#include "Entity.h"
#ifdef SKYX_ENABLED
#include "EC_SkyX.h"
#endif

// The evil Windows min & max defs seems to leak here
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include "OgreConversionUtils.h"

#include <Hydrax.h>
#include <Noise/Perlin/Perlin.h>
#include <Modules/ProjectedGrid/ProjectedGrid.h>

#include "MemoryLeakCheck.h"

struct EC_HydraxImpl
{
    EC_HydraxImpl() : hydrax(0), module(0) {}
    ~EC_HydraxImpl()
    {
        if (hydrax)
            hydrax->remove();
        SAFE_DELETE(hydrax);
        //SAFE_DELETE(module); ///< @todo Possible mem leak?
    }

    Hydrax::Hydrax *hydrax;
    Hydrax::Module::Module *module;
};

EC_Hydrax::EC_Hydrax(Scene* scene) :
    IComponent(scene),
    visible(this, "Visible", true),
    position(this, "Position")
{
    OgreWorldPtr w = scene->GetWorld<OgreWorld>();

    impl = new EC_HydraxImpl();
    impl->hydrax = new Hydrax::Hydrax(w->GetSceneManager(), static_cast<EC_Camera *>(w->GetRenderer()->GetActiveCamera())->GetCamera(),
        w->GetRenderer()->GetViewport());

    // Create our projected grid module
    Hydrax::Module::ProjectedGrid *module = new Hydrax::Module::ProjectedGrid(
        impl->hydrax, // Hydrax parent pointer
        new Hydrax::Noise::Perlin(/*Generic one*/), // Noise module
        Ogre::Plane(Ogre::Vector3(0.f,1.f,0.f), Ogre::Vector3(0.f,0.f,0.f)), // Base plane
        Hydrax::MaterialManager::NM_VERTEX, // Normal mode
        Hydrax::Module::ProjectedGrid::Options(/*264 /*Generic one*/)); // Projected grid options

    // Set our module
    impl->hydrax->setModule(module);
    impl->module = module;

    // Load all parameters from config file
    // Remarks: The config file must be in Hydrax resource group.
    // All parameters can be set/updated directly by code(Like previous versions),
    // but due to the high number of customizable parameters, since 0.4 version, Hydrax allows save/load config files.
    impl->hydrax->loadCfg("HydraxDemo.hdx");

    position.Set(OgreRenderer::ToCoreVector(impl->hydrax->getPosition()), AttributeChange::Disconnected);

    // Create water
    impl->hydrax->create();

    connect(framework->Frame(), SIGNAL(Updated(float)), SLOT(Update(float)));
    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(UpdateAttribute(IAttribute*)));
}

EC_Hydrax::~EC_Hydrax()
{
    delete impl;
}

void EC_Hydrax::UpdateAttribute(IAttribute *attr)
{
    if (!impl->hydrax)
        return;
    if (attr == &visible)
    {
        impl->hydrax->setVisible(visible.Get());
    }
    else if (attr == &position)
    {
        impl->hydrax->setPosition(OgreRenderer::ToOgreVector3(position.Get()));
    }
}

void EC_Hydrax::Update(float frameTime)
{
    if (impl->hydrax)
    {
#ifdef SKYX_ENABLED
        EntityList entities = ParentEntity()->ParentScene()->GetEntitiesWithComponent(EC_SkyX::TypeNameStatic());
        if (!entities.empty())
            impl->hydrax->setSunPosition(OgreRenderer::ToOgreVector3((*entities.begin())->GetComponent<EC_SkyX>()->SunPosition()));
#endif
        impl->hydrax->update(frameTime);
    }
}
