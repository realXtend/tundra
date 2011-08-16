/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_SkyX.cpp
 *  @brief  A sky component using SkyX, http://www.ogre3d.org/tikiwiki/SkyX
 */

#define OGRE_INTEROP

#include "DebugOperatorNew.h"

#include "EC_SkyX.h"

#include "Scene.h"
#include "Framework.h"
#include "FrameAPI.h"
#include "OgreWorld.h"
#include "Renderer.h"
#include "EC_Camera.h"
#include "LoggingFunctions.h"
#include "OgreConversionUtils.h"

#include <Ogre.h>

#include <SkyX.h>

#include "MemoryLeakCheck.h"

struct EC_SkyXImpl
{
    EC_SkyXImpl() : skyX(0), sunlight(0), cloudLayer(0) {}
    ~EC_SkyXImpl()
    {
        if (skyX)
        {
            skyX->remove();
            if (sunlight && skyX->getSceneManager())
                skyX->getSceneManager()->destroyLight(sunlight);
        }

        sunlight = 0;
        SAFE_DELETE(skyX)
    }

    SkyX::SkyX *skyX;
    Ogre::Light *sunlight;
    SkyX::CloudLayer *cloudLayer; ///< Currently just one cloud layer used.
};

EC_SkyX::EC_SkyX(Scene* scene) :
    IComponent(scene),
    volumetricClouds(this, "Volumetric clouds", false),
    timeMultiplier(this, "Time multiplier", 0.0f),
    time(this, "Time", float3(14.f, 7.5f, 20.5f)),
    weather(this, "Weather (volumetric clouds only)", float2(1.f, 1.f)),
//    windSpeed(this, "Wind speed", 0.0f),
    windDirection(this, "Wind direction", 0.0f),
    impl(0)
{
    OgreWorldPtr w = scene->GetWorld<OgreWorld>();
    if (!w)
    {
        LogError("EC_SkyX: no OgreWorld available. Cannot be created.");
        return;
    }

    connect(w.get(), SIGNAL(ActiveCameraChanged(EC_Camera *)), SLOT(OnActiveCameraChanged(EC_Camera*)));
    connect(this, SIGNAL(ParentEntitySet()), SLOT(Create()));
}

EC_SkyX::~EC_SkyX()
{
    SAFE_DELETE(impl);
}

float3 EC_SkyX::SunPosition() const
{
    if (impl->skyX)
        return impl->skyX->getAtmosphereManager()->getSunPosition();
    return float3();
}

void EC_SkyX::CreateSunlight()
{
    if (impl)
    {
        OgreWorldPtr w = ParentScene()->GetWorld<OgreWorld>();
        Ogre::SceneManager *sm = w->GetSceneManager();
        impl->sunlight = sm->createLight(w->GetRenderer()->GetUniqueObjectName("SkyXSunlight"));
        impl->sunlight->setType(Ogre::Light::LT_DIRECTIONAL);
        impl->sunlight->setDiffuseColour(1.f, 1.f, 1.f);
        impl->sunlight->setSpecularColour(0.f,0.f,0.f);
        impl->sunlight->setDirection(-1.f, -1.f, -1.f);
        impl->sunlight->setCastShadows(true);

        sm->setAmbientLight(Ogre::ColourValue(1.f, 1.f, 1.f));
    }
}

void EC_SkyX::Create()
{
    SAFE_DELETE(impl);

    try
    {
        if (!ParentScene())
        {
            LogError("EC_SkyX: no parent scene. Cannot be created.");
            return;
        }

        OgreWorldPtr w = ParentScene()->GetWorld<OgreWorld>();
        assert(w);

        if (!w->GetRenderer() || !w->GetRenderer()->GetActiveCamera())
            return; // Can't create SkyX just yet, no main camera set.

        Ogre::SceneManager *sm = w->GetSceneManager();

        impl = new EC_SkyXImpl();

        // Create Sky
        impl->skyX = new SkyX::SkyX(sm, static_cast<EC_Camera *>(w->GetRenderer()->GetActiveCamera())->GetCamera());
        impl->skyX->create();

        // A little change to default atmosphere settings.
        SkyX::AtmosphereManager::Options atOpt = impl->skyX->getAtmosphereManager()->getOptions();
        atOpt.RayleighMultiplier = 0.0045f;
        impl->skyX->getAtmosphereManager()->setOptions(atOpt);

        UpdateAttribute(&volumetricClouds);
        UpdateAttribute(&timeMultiplier);
        UpdateAttribute(&time);

        connect(framework->Frame(), SIGNAL(PostFrameUpdate(float)), SLOT(Update(float)), Qt::UniqueConnection);
        connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(UpdateAttribute(IAttribute*)), Qt::UniqueConnection);
    }
    catch(Ogre::Exception &e)
    {
        // Currently if we try to create more than one SkyX component we end up here due to Ogre internal name collision.
        LogError("Could not create EC_SkyX: " + std::string(e.what()));
    }
}

void EC_SkyX::OnActiveCameraChanged(EC_Camera *newActiveCamera)
{
    // If we haven't yet initialized, do a full init.
    if (!impl)
        Create();
    else // Otherwise, update the camera to an existing initialized SkyX instance.
        if (impl && impl->skyX)
            impl->skyX->setCamera(newActiveCamera->GetCamera());
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
            impl->skyX->getCloudsManager()->removeAll();
            impl->cloudLayer = impl->skyX->getCloudsManager()->add(SkyX::CloudLayer::Options());
        }
    }
    else if (attr == &timeMultiplier)
    {
        impl->skyX->setTimeMultiplier((Ogre::Real)timeMultiplier.Get());
    }
    else if (attr == &time)
    {
        SkyX::AtmosphereManager::Options atOpt = impl->skyX->getAtmosphereManager()->getOptions();
        atOpt.Time = time.Get();
        impl->skyX->getAtmosphereManager()->setOptions(atOpt);
    }
    else if (attr == &weather)
    {
        if (volumetricClouds.Get())
        {
            //Negative value for humidity causes crash within SkyX, so clamp value to min 0.
            float humidity = (weather.Get().x >= 0) ? weather.Get().x : 0.f;
            impl->skyX->getVCloudsManager()->getVClouds()->setWheater(humidity, weather.Get().y, 2);
        }
    }
/*
    else if (attr == &windSpeed)
    {
       if (volumetricClouds.Get())
            impl->skyX->getVCloudsManager()->setWindSpeed(0.f);
        else
            ;
    }
*/
    else if (attr == &windDirection)
    {
        if (volumetricClouds.Get())
        {
            impl->skyX->getVCloudsManager()->getVClouds()->setWindDirection(Ogre::Radian(DegToRad(windDirection.Get())));
        }
        else
        {
            SkyX::CloudLayer::Options options = impl->cloudLayer->getOptions();
            Ogre::Radian r1(Ogre::Degree(windDirection.Get())), r2(Ogre::Degree(windDirection.Get()));
            options.WindDirection = Ogre::Vector2(Ogre::Math::Cos(r1), Ogre::Math::Sin(r2));
            impl->skyX->getCloudsManager()->removeAll();
            impl->skyX->getCloudsManager()->add(options);
        }
    }
}

void EC_SkyX::Update(float frameTime)
{
    if (impl && impl->skyX)
    {
        if (impl->sunlight)
            impl->sunlight->setDirection(impl->skyX->getAtmosphereManager()->getSunDirection());
        impl->skyX->update(frameTime);
        // Do not trigger AttributeChanged for time as SkyX internals are authorative for it.
        settime(impl->skyX->getAtmosphereManager()->getOptions().Time);
    }
}
