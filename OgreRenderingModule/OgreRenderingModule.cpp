// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

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
#include "ConsoleServiceInterface.h"
#include "ConsoleCommandServiceInterface.h"
#include "RendererSettings.h"
#include "EventManager.h"
#include "AssetAPI.h"
#include "GenericAssetFactory.h"
#include "OgreMeshAsset.h"
#include "OgreParticleAsset.h"
#include "OgreSkeletonAsset.h"
#include "OgreMaterialAsset.h"
#include "TextureAsset.h"
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
        // Provide supported file extensions for each factory.
        /// \todo Ask ogre what file formats Ogre::Image supports during run time, if at all possible.
        QStringList supportedFileFormats;
        supportedFileFormats << "jpg" << "jpeg" << "png" << "tga" << "bmp" << "dds" << "gif" << "tiff";
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<TextureAsset>("Texture", supportedFileFormats)));
        
        supportedFileFormats.clear();
        supportedFileFormats << "mesh";

        /*! \todo Ask assimp somehow the supported mesh list if it can be different that the below list.
                  It should also be noted that all of the below types have not been tested to work, but are expexted
                  to work in theory via assimp.
        */
#ifdef ASSIMP_ENABLED
        supportedFileFormats << "3d" << ".b3d" << "dae" << "bvh" << "3ds" << "ase" << "obj" << "ply" << "dxf" 
                             << "nff" << "smd" << "vta" << "mdl" << "md2" << "md3" << "mdc" << "md5mesh" << "x"
                             << "q3o" << "q3s" << "raw" << "ac" << "stl" << "irrmesh" << "irr" << "off" << "ter"
                             << "mdl" << "hmp" << "ms3d" << "lwo" << "lws" << "lxo" << "csm" << "ply" << "cob" << "scn";
#endif
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<OgreMeshAsset>("OgreMesh", supportedFileFormats)));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<OgreParticleAsset>("OgreParticle", "particle")));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<OgreSkeletonAsset>("OgreSkeleton", "skeleton")));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<OgreMaterialAsset>("OgreMaterial", "material")));
    }

    void OgreRenderingModule::PreInitialize()
    {
        std::string ogre_config_filename = "ogre.cfg";
#if defined (_WINDOWS) && (_DEBUG)
        std::string plugins_filename = "pluginsd.cfg";
#elif defined (_WINDOWS)
        std::string plugins_filename = "plugins.cfg";
#elif defined(__APPLE__)
        std::string plugins_filename = "plugins-mac.cfg";
#else
        std::string plugins_filename = "plugins-unix.cfg";
#endif

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

        RegisterConsoleCommand(Console::CreateCommand(
                "RenderStats", "Prints out render statistics.", 
                Console::Bind(this, &OgreRenderingModule::ConsoleStats)));
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

    Console::CommandResult OgreRenderingModule::ConsoleStats(const StringVector &params)
    {
        if (renderer_)
        {
            Console::ConsoleServiceInterface *console = GetFramework()->GetService<Console::ConsoleServiceInterface>();
            if (console)
            {
                const Ogre::RenderTarget::FrameStats& stats = renderer_->GetCurrentRenderWindow()->getStatistics();
                console->Print("Average FPS: " + ToString(stats.avgFPS));
                console->Print("Worst FPS: " + ToString(stats.worstFPS));
                console->Print("Best FPS: " + ToString(stats.bestFPS));
                console->Print("Triangles: " + ToString(stats.triangleCount));
                console->Print("Batches: " + ToString(stats.batchCount));

                return Console::ResultSuccess();
            }
        }

        return Console::ResultFailure("No renderer found.");
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
