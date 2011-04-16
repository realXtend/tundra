// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Application.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "EC_OgreSky.h"
#include "EC_OgreCustomObject.h"
#include "EC_OgreMovableTextOverlay.h"
#include "EC_AnimationController.h"
#include "EC_OgreEnvironment.h"
#include "EC_OgreCamera.h"
#include "EC_OgreCompositor.h"
#include "EC_RttTarget.h"
#include "EC_BillboardWidget.h"
#include "EC_SelectionBox.h"
#include "Entity.h"
#include "RendererSettings.h"
#include "EventManager.h"
#include "AssetAPI.h"
#include "GenericAssetFactory.h"
#include "OgreMeshAsset.h"
#include "OgreParticleAsset.h"
#include "OgreSkeletonAsset.h"
#include "OgreMaterialAsset.h"
#include "TextureAsset.h"
#include "ConsoleAPI.h"
#include "ConsoleCommandUtils.h"
#include "ConfigAPI.h"

#include "MemoryLeakCheck.h"

namespace OgreRenderer
{
    std::string OgreRenderingModule::type_name_static_ = "OgreRendering";

    OgreRenderingModule::OgreRenderingModule() :
        IModule(type_name_static_)
    {
    }

    OgreRenderingModule::~OgreRenderingModule()
    {
    }

    // virtual
    void OgreRenderingModule::Load()
    {
        DECLARE_MODULE_EC(EC_Placeable);
        DECLARE_MODULE_EC(EC_Mesh);
        DECLARE_MODULE_EC(EC_OgreSky);
        DECLARE_MODULE_EC(EC_OgreCustomObject);
        DECLARE_MODULE_EC(EC_OgreMovableTextOverlay);
        DECLARE_MODULE_EC(EC_AnimationController);
        DECLARE_MODULE_EC(EC_OgreEnvironment);
        DECLARE_MODULE_EC(EC_OgreCamera);
        DECLARE_MODULE_EC(EC_BillboardWidget);
        DECLARE_MODULE_EC(EC_OgreCompositor);
        DECLARE_MODULE_EC(EC_RttTarget);
        DECLARE_MODULE_EC(EC_SelectionBox);

        // Create asset type factories for each asset OgreRenderingModule provides to the system.
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<TextureAsset>("Texture")));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<TextureAsset>("OgreTexture"))); // deprecated/old style.
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<OgreMeshAsset>("OgreMesh")));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<OgreParticleAsset>("OgreParticle")));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<OgreSkeletonAsset>("OgreSkeleton")));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<OgreMaterialAsset>("OgreMaterial")));
    }

    void OgreRenderingModule::PreInitialize()
    {
        std::string ogre_config_filename = Application::InstallationDirectory().toStdString() + "ogre.cfg"; ///\todo Unicode support!
#if defined (_WINDOWS) && (_DEBUG)
        std::string plugins_filename = "pluginsd.cfg";
#elif defined (_WINDOWS)
        std::string plugins_filename = "plugins.cfg";
#elif defined(__APPLE__)
        std::string plugins_filename = "plugins-mac.cfg";
#else
        std::string plugins_filename = "plugins-unix.cfg";
#endif

        plugins_filename = Application::InstallationDirectory().toStdString() + plugins_filename; ///\todo Unicode support!

        // Create renderer here, so it can be accessed in uninitialized state by other module's PreInitialize()
        std::string window_title = framework_->Config()->GetApplicationIdentifier().toStdString();
        renderer_ = OgreRenderer::RendererPtr(new OgreRenderer::Renderer(framework_, ogre_config_filename, plugins_filename, window_title));
    }

    void OgreRenderingModule::Initialize()
    {
        assert (renderer_);
        assert (!renderer_->IsInitialized());
        renderer_->Initialize();

        framework_->GetServiceManager()->RegisterService(Service::ST_Renderer, renderer_);

        framework_->RegisterDynamicObject("renderer", renderer_.get());
    }

    void OgreRenderingModule::PostInitialize()
    {
        EventManagerPtr event_manager = framework_->GetEventManager();

        renderer_->PostInitialize();

        framework_->Console()->RegisterCommand(CreateConsoleCommand(
                "RenderStats", "Prints out render statistics.", 
                ConsoleBind(this, &OgreRenderingModule::ConsoleStats)));
        renderer_settings_ = RendererSettingsPtr(new RendererSettings(framework_));
    }

    bool OgreRenderingModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
    {
        PROFILE(OgreRenderingModule_HandleEvent);
        if (!renderer_)
            return false;

        return false;
    }

    void OgreRenderingModule::Uninitialize()
    {
        // We're shutting down. Force a release of all loaded asset objects from the Asset API so that 
        // no refs to Ogre assets remain - below 'renderer_.reset()' is going to delete Ogre::Root.
        framework_->Asset()->ForgetAllAssets();

        framework_->GetServiceManager()->UnregisterService(renderer_);

        // Explicitly remove log listener now, because the service has been
        // unregistered now, and no-one can get at the renderer to remove themselves
        if (renderer_)
            renderer_->RemoveLogListener();

        renderer_settings_.reset();
        renderer_.reset();
    }

    void OgreRenderingModule::Update(f64 frametime)
    {
        {
            PROFILE(OgreRenderingModule_Update);
            renderer_->Update(frametime);
        }
        RESETPROFILER;
    }

    ConsoleCommandResult OgreRenderingModule::ConsoleStats(const StringVector &params)
    {
        if (renderer_)
        {
            ConsoleAPI *c = framework_->Console();
            const Ogre::RenderTarget::FrameStats& stats = renderer_->GetCurrentRenderWindow()->getStatistics();
            c->Print("Average FPS: " + QString::number(stats.avgFPS));
            c->Print("Worst FPS: " + QString::number(stats.worstFPS));
            c->Print("Best FPS: " + QString::number(stats.bestFPS));
            c->Print("Triangles: " + QString::number(stats.triangleCount));
            c->Print("Batches: " + QString::number(stats.batchCount));
            return ConsoleResultSuccess();
        }

        return ConsoleResultFailure("No renderer found.");
    }
}

void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace OgreRenderer;

extern "C"
{
__declspec(dllexport) void TundraPluginMain(Foundation::Framework *fw)
{
    IModule *module = new OgreRenderer::OgreRenderingModule();
    fw->GetModuleManager()->DeclareStaticModule(module);
}
}
