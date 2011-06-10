/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_SkyX.cpp
 *  @brief  A sky component using SkyX, http://www.ogre3d.org/tikiwiki/SkyX
 */

#include "DebugOperatorNew.h"

#include "EC_SkyX.h"

#include "Scene.h"
#include "Framework.h"
#include "FrameAPI.h"
#include "OgreWorld.h"
#include "Renderer.h"
#include "EC_Camera.h"
#include "LoggingFunctions.h"

// The evil Windows min & max defs seems to leak here
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include "OgreConversionUtils.h"

#include <Ogre.h>

#include <SkyX.h>

#include "MemoryLeakCheck.h"

struct EC_SkyXImpl
{
    EC_SkyXImpl() : skyX(0) {}
    ~EC_SkyXImpl()
    {
        if (skyX)
            skyX->remove();
        SAFE_DELETE(skyX)
    }

    SkyX::SkyX *skyX;
};

EC_SkyX::EC_SkyX(Scene* scene) :
    IComponent(scene),
    volumetricClouds(this, "Volumetric clouds", false),
    timeMultiplier(this, "Time multiplier", 0.25f),
    time(this, "Time")
{
    try
    {
        OgreWorldPtr w = scene->GetWorld<OgreWorld>();
        // Create Sky
        impl = new EC_SkyXImpl();
        impl->skyX = new SkyX::SkyX(w->GetSceneManager(),
            static_cast<EC_Camera *>(w->GetRenderer()->GetActiveCamera())->GetCamera());
        impl->skyX->create();

        UpdateAttribute(&volumetricClouds);
        UpdateAttribute(&timeMultiplier);

        // A little change to default atmosphere settings
        SkyX::AtmosphereManager::Options atOpt = impl->skyX->getAtmosphereManager()->getOptions();
        atOpt.RayleighMultiplier = 0.0045f;
        impl->skyX->getAtmosphereManager()->setOptions(atOpt);

        connect(framework_->Frame(), SIGNAL(Updated(float)), SLOT(Update(float)));
        connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(UpdateAttribute(IAttribute*)));
    }
    catch(Ogre::Exception &e)
    {
        // Currently if we try to create more than one SkyX component we end up here due to Ogre internal name collision.
        LogError("Could not create EC_SkyX: " + std::string(e.what()));
    }
}

EC_SkyX::~EC_SkyX()
{
    delete impl;
}

Vector3df EC_SkyX::SunPosition() const
{
    if (impl->skyX)
        return OgreRenderer::ToCoreVector(impl->skyX->getAtmosphereManager()->getSunPosition());
    return Vector3df();
}

void EC_SkyX::UpdateAttribute(IAttribute *attr)
{
    if (!impl->skyX)
        return;

    if (attr == &volumetricClouds)
    {
        if (volumetricClouds.Get())
        {
            impl->skyX->getCloudsManager()->removeAll();
            impl->skyX->getVCloudsManager()->create();
        }
        else
        {
            impl->skyX->getVCloudsManager()->remove();
            impl->skyX->getCloudsManager()->add(SkyX::CloudLayer::Options());
        }
    }
    else if (attr == &timeMultiplier)
    {
        impl->skyX->setTimeMultiplier((Ogre::Real)timeMultiplier.Get());
    }
    else if (attr == &time)
    {
        SkyX::AtmosphereManager::Options atOpt = impl->skyX->getAtmosphereManager()->getOptions();
        atOpt.Time = OgreRenderer::ToOgreVector3(time.Get());
        impl->skyX->getAtmosphereManager()->setOptions(atOpt);
    }
}

void EC_SkyX::Update(float frameTime)
{
    if (impl->skyX)
    {
        impl->skyX->update(frameTime);
        // Do not trigger AttributeChanged for time as SkyX internals are authorative for it.
        settime(OgreRenderer::ToCoreVector(impl->skyX->getAtmosphereManager()->getOptions().Time));
    }
}
