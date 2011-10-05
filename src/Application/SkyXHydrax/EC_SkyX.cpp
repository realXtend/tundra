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
#include "Entity.h"
#include "LoggingFunctions.h"
#include "Math/MathFunc.h"
#include "Profiler.h"
#include "Color.h"

#include <Ogre.h>

#include <SkyX.h>

#include "MemoryLeakCheck.h"

/// @cond PRIVATE
struct EC_SkyXImpl
{
    EC_SkyXImpl() : skyX(0), sunlight(0), cloudLayer(0) {}
    ~EC_SkyXImpl()
    {
        if (skyX)
        {
            skyX->remove();
            Ogre::SceneManager *sm = skyX->getSceneManager();
            if (sm)
            {
                if (sunlight)
                    sm->destroyLight(sunlight);
                sm->setAmbientLight(originalAmbientColor);
            }
        }

        sunlight = 0;
        SAFE_DELETE(skyX)
    }

    operator bool () const { return skyX != 0; }

    SkyX::SkyX *skyX;
    SkyX::CloudLayer *cloudLayer; ///< Currently just one cloud layer used.
    Ogre::Light *sunlight;
    Color originalAmbientColor;
};
/// @endcond

EC_SkyX::EC_SkyX(Scene* scene) :
    IComponent(scene),
    volumetricClouds(this, "Volumetric clouds", false),
    timeMultiplier(this, "Time multiplier", 0.0f),
    time(this, "Time", float3(14.f, 7.5f, 20.5f)),
    weather(this, "Weather (volumetric clouds only)", float2(1.f, 1.f)),
//    windSpeed(this, "Wind speed (volumetric clouds only)", 800.f),
    windDirection(this, "Wind direction", 0.0f),
    impl(0)
{
    OgreWorldPtr w = scene->GetWorld<OgreWorld>();
    if (!w)
    {
        LogError("EC_SkyX: no OgreWorld available. Cannot be created.");
        return;
    }

    connect(w->GetRenderer(), SIGNAL(MainCameraChanged(Entity *)), SLOT(OnActiveCameraChanged(Entity *)));
    connect(this, SIGNAL(ParentEntitySet()), SLOT(Create()));
}

EC_SkyX::~EC_SkyX()
{
    SAFE_DELETE(impl);
}

float3 EC_SkyX::SunPosition() const
{
    if (impl)
        return impl->skyX->getAtmosphereManager()->getSunPosition();
    return float3();
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

        if (!w->GetRenderer() || !w->GetRenderer()->MainCamera())
            return; // Can't create SkyX just yet, no main camera set.

        Ogre::SceneManager *sm = w->GetSceneManager();
        Entity *mainCamera = w->GetRenderer()->MainCamera();
        if (!mainCamera)
        {
            LogError("Cannot create SkyX: No main camera set!");
            return;
        }
        impl = new EC_SkyXImpl();
        impl->skyX = new SkyX::SkyX(sm, mainCamera->GetComponent<EC_Camera>()->GetCamera());
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

        CreateSunlight();
    }
    catch(Ogre::Exception &e)
    {
        // Currently if we try to create more than one SkyX component we end up here due to Ogre internal name collision.
        LogError("Could not create EC_SkyX: " + std::string(e.what()));
    }
}

void EC_SkyX::CreateSunlight()
{
    if (impl)
    {
        // Ambient and sun diffuse color copied from EC_EnvironmentLight
        OgreWorldPtr w = ParentScene()->GetWorld<OgreWorld>();
        Ogre::SceneManager *sm = w->GetSceneManager();
        impl->originalAmbientColor = sm->getAmbientLight();
        sm->setAmbientLight(Color(0.364f, 0.364f, 0.364f, 1.f));

        impl->sunlight = sm->createLight(w->GetRenderer()->GetUniqueObjectName("SkyXSunlight"));
        impl->sunlight->setType(Ogre::Light::LT_DIRECTIONAL);
        impl->sunlight->setDiffuseColour(Color(0.639f,0.639f,0.639f));
        impl->sunlight->setSpecularColour(0.f,0.f,0.f);
        impl->sunlight->setDirection(impl->skyX->getAtmosphereManager()->getSunDirection());
        impl->sunlight->setCastShadows(true);
    }
}

void EC_SkyX::OnActiveCameraChanged(Entity *newActiveCamera)
{
    if (!newActiveCamera)
    {
        SAFE_DELETE(impl);
        return;
    }
    // If we haven't yet initialized, do a full init.
    if (!impl)
        Create();
    else // Otherwise, update the camera to an existing initialized SkyX instance.
        if (impl)
            impl->skyX->setCamera(newActiveCamera->GetComponent<EC_Camera>()->GetCamera());
}

void EC_SkyX::UpdateAttribute(IAttribute *attr)
{
    if (!impl)
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
        // Clamp value to [0.0, 1.0], otherwise crashes occur.
        float2 w = Clamp(weather.Get(), 0.f, 1.f);
        if (volumetricClouds.Get())
            impl->skyX->getVCloudsManager()->getVClouds()->setWheater(w.x, w.y, 2);
        weather.Set(w, AttributeChange::Disconnected);
    }
/*    else if (attr == &windSpeed)
    {
       if (volumetricClouds.Get())
           impl->skyX->getVCloudsManager()->setWindSpeed(windSpeed.Get());
    }
*/    else if (attr == &windDirection)
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
    if (impl)
    {
        PROFILE(EC_SkyX_Update);
        if (impl->sunlight)
            impl->sunlight->setDirection(impl->skyX->getAtmosphereManager()->getSunDirection());
        impl->skyX->update(frameTime);
        // Do not replicate constant time attribute updates as SkyX internals are authoritative for it.
        time.Set(impl->skyX->getAtmosphereManager()->getOptions().Time, AttributeChange::LocalOnly);
    }
}
