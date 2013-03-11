/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   EC_Hydrax.cpp
    @brief  A photorealistic water plane component using Hydrax, http://www.ogre3d.org/tikiwiki/Hydrax */

#define MATH_OGRE_INTEROP

#include "DebugOperatorNew.h"

#include "EC_Hydrax.h"
#ifdef SKYX_ENABLED
#include "EC_SkyX.h"
#endif

#include "Scene/Scene.h"
#include "Framework.h"
#include "FrameAPI.h"
#include "AssetAPI.h"
#include "IAssetTransfer.h"
#include "IAsset.h"
#include "OgreWorld.h"
#include "Renderer.h"
#include "EC_Camera.h"
#include "Entity.h"
#include "Profiler.h"
#include "AttributeMetadata.h"
#include "LoggingFunctions.h"

#include <Hydrax.h>
#include <Noise/Perlin/Perlin.h>
#include <Noise/FFT/FFT.h>
#include <Modules/ProjectedGrid/ProjectedGrid.h>
#include <Modules/RadialGrid/RadialGrid.h>
#include <Modules/SimpleGrid/SimpleGrid.h>

#include "MemoryLeakCheck.h"

static const char *cDefaultConfig = "Ogre Media:HydraxDefault.hdx";

struct EC_Hydrax::EC_HydraxImpl
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
#ifdef SKYX_ENABLED
    weak_ptr<EC_SkyX> skyX;
#endif
};

EC_Hydrax::EC_Hydrax(Scene* scene) :
    IComponent(scene),
    configRef(this, "Config ref", AssetReference(cDefaultConfig)),
    visible(this, "Visible", true),
    position(this, "Position", float3(0.0, 0.0, 0.0)),
//    noiseModule(this, "Noise module", 0),
//    noiseType(this, "Noise type", 0),
//    normalMode(this, "Normal mode", 0),
    impl(0)
{
/*
    static AttributeMetadata noiseTypeMetadata;
    static AttributeMetadata normalModeMetadata;
    static bool metadataInitialized = false;
    if (!metadataInitialized)
    {
        noiseTypeMetadata.enums[Perlin] = "Perlin";
        noiseTypeMetadata.enums[FFT] = "FFT";

        normalModeMetadata.enums[Texture] = "Texture";
        normalModeMetadata.enums[Vertex] = "Vertex";
        normalModeMetadata.enums[RTT] = "RTT";

        metadataInitialized = true;
    }

    noiseType.SetMetadata(&noiseTypeMetadata);
    normalMode.SetMetadata(&normalModeMetadata);
*/
    OgreWorldPtr w = scene->GetWorld<OgreWorld>();
    if (!w)
    {
        LogError("EC_Hydrax: no OgreWorld available. Cannot be created.");
        return;
    }

    connect(w->Renderer(), SIGNAL(MainCameraChanged(Entity *)), SLOT(OnActiveCameraChanged(Entity *)));
    connect(this, SIGNAL(ParentEntitySet()), SLOT(Create()));

    connect(&configRefListener, SIGNAL(Loaded(AssetPtr)), this, SLOT(ConfigLoadSucceeded(AssetPtr)));

}

EC_Hydrax::~EC_Hydrax()
{
    SAFE_DELETE(impl);
}

float EC_Hydrax::HeightAt(const float3 &worldPos) const
{
    return impl && impl->hydrax ? impl->hydrax->getHeigth(worldPos) : -1.f;
}

float EC_Hydrax::HeightAt(float x, float z) const
{
    return HeightAt(float3(x, 0.f, z));
}

void EC_Hydrax::Create()
{
    PROFILE(EC_Hydrax_Create);
    SAFE_DELETE(impl);

    if (framework->IsHeadless())
        return;

    try
    {
        if (!ParentScene())
        {
            LogError("EC_Hydrax: no parent scene. Cannot be created.");
            return;
        }

        OgreWorldPtr w = ParentScene()->GetWorld<OgreWorld>();
        assert(w);

        Entity *mainCamera = w->Renderer()->MainCamera();
        if (!mainCamera)
        {
            // Can't create Hydrax just yet, no main camera set (Hydrax needs a valid camera to initialize).
            // This error is benign, and Hydrax will now postpone its initialization to until a camera is set.
            // (see OnActiveCameraChanged()).
            LogDebug("Cannot create EC_Hydrax: No main camera set!");
            return; 
        }

        Ogre::Camera *cam = mainCamera->GetComponent<EC_Camera>()->GetCamera();
        impl = new EC_HydraxImpl();
        impl->hydrax = new Hydrax::Hydrax(w->OgreSceneManager(), cam, w->Renderer()->MainViewport());

        // Using projected grid module by default
        Hydrax::Module::ProjectedGrid *module = new Hydrax::Module::ProjectedGrid(impl->hydrax, new Hydrax::Noise::Perlin(),
            Ogre::Plane(Ogre::Vector3::UNIT_Y, Ogre::Vector3::ZERO), Hydrax::MaterialManager::NM_VERTEX);
        impl->hydrax->setModule(module);
        impl->module = module;

        // Load all parameters from config file, but position attribute is always authoritative for the position.
        RequestConfigAsset();

        connect(framework->Frame(), SIGNAL(PostFrameUpdate(float)), SLOT(Update(float)), Qt::UniqueConnection);
    }
    catch(Ogre::Exception &e)
    {
        // Currently if we try to create more than one Hydrax component we end up here due to Ogre internal name collision.
        LogError("Could not create EC_Hydrax: " + std::string(e.what()));
    }
}

void EC_Hydrax::OnActiveCameraChanged(Entity *newActiveCamera)
{
    PROFILE(EC_Hydrax_OnActiveCameraChanged);

    // If no active camera or the new active camera is observing another Ogre scene than the one this EC_Hydrax component is in, 
    // don't initialize Hydrax to that scene.
    if (!newActiveCamera || newActiveCamera->ParentScene() != ParentScene())
    {
        SAFE_DELETE(impl);
        return;
    }

    // If we haven't yet initialized, do a full init.
    if (!impl)
        Create();
    else // Otherwise, update the camera to an existing initialized Hydrax instance.
        if (impl && impl->hydrax)
            impl->hydrax->setCamera(newActiveCamera->GetComponent<EC_Camera>()->GetCamera());
}

void EC_Hydrax::RequestConfigAsset()
{
    PROFILE(EC_Hydrax_RequestConfigAsset);
    QString ref = configRef.Get().ref.trimmed();
    if (ref.isEmpty())
        ref = cDefaultConfig;
    configRefListener.HandleAssetRefChange(framework->Asset(), ref, "Binary");
}

void EC_Hydrax::AttributesChanged()
{
    PROFILE(EC_Hydrax_AttributesChanged);
    if (configRef.ValueChanged())
        RequestConfigAsset();

    if (!impl || !impl->hydrax)
        return;

    if (visible.ValueChanged())
    {
        const float3 &pos = position.Get();
        impl->hydrax->setVisible(visible.Get());
        if (visible.Get() && impl->hydrax->getPosition() != pos)
            impl->hydrax->setPosition(pos);
    }
    if (position.ValueChanged() && visible.Get())
        impl->hydrax->setPosition(position.Get());
/*
    if (noiseModule.ValueChanged() || normalMode.ValueChanged() || noiseType.ValueChanged())
        UpdateNoiseModule();
*/
}
/*
void EC_Hydrax::UpdateNoiseModule()
{
    Hydrax::Noise::Noise *noise = 0;
    switch(noiseType.Get())
    {
    case Perlin:
        noise = new Hydrax::Noise::Perlin();
        break;
    case FFT:
        noise = new Hydrax::Noise::FFT();
        break;
    default:
        LogError("Invalid Hydrax noise module type.");
        return;
    }

    Hydrax::Module::Module *module = 0;
    switch(noiseModule.Get())
    {
    case ProjectedGrid:
        module = new Hydrax::Module::ProjectedGrid(impl->hydrax, noise, Ogre::Plane(Ogre::Vector3::UNIT_Y, Ogre::Vector3::ZERO),
            (Hydrax::MaterialManager::NormalMode)normalMode.Get());
        break;
    case RadialGrid:
        module = new Hydrax::Module::RadialGrid(impl->hydrax, noise, (Hydrax::MaterialManager::NormalMode)normalMode.Get());
        break;
    case SimpleGrid:
        module = new Hydrax::Module::SimpleGrid(impl->hydrax, noise, (Hydrax::MaterialManager::NormalMode)normalMode.Get());
        break;
    default:
        SAFE_DELETE(noise);
        LogError("Invalid Hydrax noise module type.");
        return;
    }
    //if (impl->module && noise)
    //    impl->module->setNoise(noise);

    impl->hydrax->setModule(module);
    impl->module = module;
    //impl->hydrax->loadCfg("HydraxDemo.hdx");
    impl->hydrax->create();
}
*/
void EC_Hydrax::Update(float frameTime)
{
    if (impl && impl->hydrax)
    {
        PROFILE(EC_Hydrax_Update);
#ifdef SKYX_ENABLED
        // Find out if we have SkyX in the scene. If yes, use its sun position.
        if (impl->skyX.expired())
        {
            EntityList entities = ParentEntity()->ParentScene()->GetEntitiesWithComponent(EC_SkyX::TypeNameStatic());
            if (!entities.empty())
                impl->skyX = (*entities.begin())->GetComponent<EC_SkyX>();
        }

        // Set Hydrax's sun position to use either sun or moon, depending which is visible.
        shared_ptr<EC_SkyX> skyX = impl->skyX.lock();
        if (skyX && impl->hydrax->isCreated())
        {
            // Decrease sun strength for moonlight. Otherwise the light projection on the water surface looks unnaturally.
            /// @todo Decrease underwater sun strength too.
            const float defaultSunlightStrenth = 1.75f;
            const float defaultMoonlightStrenth = 0.75f;
            if (skyX->IsSunVisible())
            {
                if (impl->hydrax->getSunStrength() != defaultSunlightStrenth)
                    impl->hydrax->setSunStrength(defaultSunlightStrenth);
                impl->hydrax->setSunPosition(skyX->SunPosition());
            }
            else if (skyX->IsMoonVisible())
            {
                if (impl->hydrax->getSunStrength() != defaultMoonlightStrenth)
                    impl->hydrax->setSunStrength(defaultMoonlightStrenth);
                impl->hydrax->setSunPosition(skyX->MoonPosition());
            }
        }
#endif
        if (impl->hydrax->isCreated())
            impl->hydrax->update(frameTime);
    }
}

void EC_Hydrax::ConfigLoadSucceeded(AssetPtr asset)
{
    PROFILE(EC_Hydrax_ConfigLoadSucceeded);
    // If we haven't yet initialized, do a full init.
    if (!impl || !impl->hydrax || !impl->module)
        Create();

    if (!impl || !impl->hydrax || !impl->module)
    {
        LogError("EC_Hydrax: Could not apply Hydrax config \"" + asset->Name() + "\", hydrax could not be initialized!");
        return;
    }

    std::vector<u8> rawData;
    asset->SerializeTo(rawData);
    QString configData = QString::fromAscii((const char*)&rawData[0], rawData.size());

    if (configData.isEmpty())
    {
        LogInfo("EC_Hydrax: Downloaded config is empty!");
        return;
    }

    try
    {
        // Update the noise module
        if (configData.contains("noise=fft", Qt::CaseInsensitive))
        {
            /// \note Using the FFT noise plugin seems to crash somewhere after we leave this function.
            /// FFT looks better so would be nice to investigate further!
            if (impl->module->getNoise()->getName() != "FFT")
                impl->module->setNoise(new Hydrax::Noise::FFT());
        }
        else if (configData.contains("noise=perlin", Qt::CaseInsensitive))
        {
            if (impl->module->getNoise()->getName() != "Perlin")
                impl->module->setNoise(new Hydrax::Noise::Perlin());
        }
        else
        {
            LogError("EC_Hydrax: Unknown noise param in loaded config, acceptable = FFT/Perlin.");
            SAFE_DELETE(impl);
            return;
        }

        // Load config from the asset data string.
        impl->hydrax->remove();
        impl->hydrax->loadCfgString(configData.toStdString());
        
        // Override the shader mode specified in the config - OpenGL should always use GLSL, D3D HLSL.
        // (Cg is never used, for compatibility, since it requires an extra install and some Linux systems don't always have it enabled)
        if (QString(Ogre::Root::getSingleton().getRenderSystem()->getName().c_str()).contains("OpenGL"))
            impl->hydrax->setShaderMode(Hydrax::MaterialManager::SM_GLSL);
        else
            impl->hydrax->setShaderMode(Hydrax::MaterialManager::SM_HLSL);
        impl->hydrax->create();

        // The position attribute is always authoritative from the component attribute.
        if (visible.Get())
            impl->hydrax->setPosition(position.Get());
    }
    catch (Ogre::Exception &e)
    {
        LogError(std::string("EC_Hydrax: Ogre threw exception while loading new config: ") + e.what());
        if (impl && impl->hydrax)
            impl->hydrax->remove();
        SAFE_DELETE(impl);
    }
}
