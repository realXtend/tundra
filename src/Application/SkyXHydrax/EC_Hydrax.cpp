/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Hydrax.cpp
 *  @brief  A photorealistic water plane component using Hydrax, http://www.ogre3d.org/tikiwiki/Hydrax
 */

#define OGRE_INTEROP

#include "DebugOperatorNew.h"

#include "EC_Hydrax.h"

#include "Scene.h"
#include "Framework.h"
#include "FrameAPI.h"
#include "OgreWorld.h"
#include "Renderer.h"
#include "EC_Camera.h"
#include "Entity.h"
#include "OgreConversionUtils.h"
#include "LoggingFunctions.h"
#ifdef SKYX_ENABLED
#include "EC_SkyX.h"
#endif

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
    position(this, "Position"),
    impl(0)
{
    OgreWorldPtr w = scene->GetWorld<OgreWorld>();
    if (!w)
    {
        LogError("EC_SkyX: no OgreWorld available. Cannot be created.");
        return;
    }

    connect(w.get(), SIGNAL(ActiveCameraChanged(EC_Camera *)), SLOT(OnActiveCameraChanged(EC_Camera *)));
    connect(this, SIGNAL(ParentEntitySet()), SLOT(Create()));
}

EC_Hydrax::~EC_Hydrax()
{
    SAFE_DELETE(impl);
}

void EC_Hydrax::Create()
{
    SAFE_DELETE(impl);

    if (!ParentScene())
    {
        LogError("EC_Hydrax: no parent scene. Cannot be created.");
        return;
    }

    OgreWorldPtr w = ParentScene()->GetWorld<OgreWorld>();
    assert(w);

    if (!w->GetRenderer() || !w->GetRenderer()->GetActiveCamera())
        return; // Can't create Hydrax just yet, no main camera set.

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

    position.Set(impl->hydrax->getPosition(), AttributeChange::Disconnected);

    // Create water
    impl->hydrax->create();

    connect(framework->Frame(), SIGNAL(PostFrameUpdate(float)), SLOT(Update(float)), Qt::UniqueConnection);
    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(UpdateAttribute(IAttribute*)), Qt::UniqueConnection);
}

void EC_Hydrax::OnActiveCameraChanged(EC_Camera *newActiveCamera)
{
#if defined(_WINDOWS)
    // If we haven't yet initialized, do a full init.
    if (!impl)
        Create();
    else // Otherwise, update the camera to an existing initialized Hydrax instance.
        if (impl && impl->hydrax)
            impl->hydrax->setCamera(newActiveCamera->GetCamera());
#else
    /// \todo No setCamera method in my copy of HydraX -erno
    Create();
#endif
}

void EC_Hydrax::UpdateAttribute(IAttribute *attr)
{
    if (!impl->hydrax)
        return;
    if (attr == &visible)
        impl->hydrax->setVisible(visible.Get());
    else if (attr == &position)
        impl->hydrax->setPosition(position.Get());
}

void EC_Hydrax::Update(float frameTime)
{
    if (impl->hydrax)
    {
#ifdef SKYX_ENABLED
        ///\todo Store weak_ptr to EC_SkyX
        EntityList entities = ParentEntity()->ParentScene()->GetEntitiesWithComponent(EC_SkyX::TypeNameStatic());
        if (!entities.empty())
            impl->hydrax->setSunPosition((*entities.begin())->GetComponent<EC_SkyX>()->SunPosition());
#endif
        impl->hydrax->update(frameTime);
    }
}
