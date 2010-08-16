// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "ResourceHandler.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreMesh.h"
#include "EC_OgreLight.h"
#include "EC_OgreSky.h"
#include "EC_OgreCustomObject.h"
#include "EC_OgreConsoleOverlay.h"
#include "EC_OgreMovableTextOverlay.h"
#include "EC_OgreParticleSystem.h"
#include "EC_OgreAnimationController.h"
#include "EC_OgreEnvironment.h"
#include "EC_OgreCamera.h"

#include "InputEvents.h"
#include "SceneEvents.h"
#include "Entity.h"
#include "ConsoleServiceInterface.h"
#include "ConsoleCommandServiceInterface.h"
#include "RendererSettings.h"
#include "ConfigurationManager.h"
#include "EventManager.h"

namespace OgreRenderer
{
    std::string OgreRenderingModule::type_name_static_ = "OgreRendering";

    OgreRenderingModule::OgreRenderingModule() :
        ModuleInterface(type_name_static_),
        asset_event_category_(0),
        resource_event_category_(0),
        input_event_category_(0),
        scene_event_category_(0)
    {
    }

    OgreRenderingModule::~OgreRenderingModule()
    {
    }

    // virtual
    void OgreRenderingModule::Load()
    {
        DECLARE_MODULE_EC(EC_OgrePlaceable);
        DECLARE_MODULE_EC(EC_OgreMesh);
        DECLARE_MODULE_EC(EC_OgreLight);
        DECLARE_MODULE_EC(EC_OgreSky);
        DECLARE_MODULE_EC(EC_OgreCustomObject);
        DECLARE_MODULE_EC(EC_OgreConsoleOverlay);
        DECLARE_MODULE_EC(EC_OgreMovableTextOverlay);
        DECLARE_MODULE_EC(EC_OgreParticleSystem);
        DECLARE_MODULE_EC(EC_OgreAnimationController);
        DECLARE_MODULE_EC(EC_OgreEnvironment);
        DECLARE_MODULE_EC(EC_OgreCamera);
    }

    // virtual
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

        // Create window title
        std::string group = Foundation::Framework::ConfigurationGroup();
        std::string version_major = framework_->GetDefaultConfig().GetSetting<std::string>(group, "version_major");
        std::string version_minor = framework_->GetDefaultConfig().GetSetting<std::string>(group, "version_minor");
        std::string window_title = framework_->GetDefaultConfig().GetSetting<std::string>(group, "window_title") + " " + version_major + "." + version_minor;

        // Create renderer here, so it can be accessed in uninitialized state by other module's PreInitialize()
        renderer_ = OgreRenderer::RendererPtr(new OgreRenderer::Renderer(framework_, ogre_config_filename, plugins_filename, window_title));
    }

    // virtual
    void OgreRenderingModule::Initialize()
    {
        assert (renderer_);
        assert (!renderer_->IsInitialized());
        renderer_->Initialize();

        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Renderer, renderer_);
    }

    // virtual
    void OgreRenderingModule::PostInitialize()
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();

        asset_event_category_ = event_manager->QueryEventCategory("Asset");
        resource_event_category_ = event_manager->QueryEventCategory("Resource");
        input_event_category_ = event_manager->QueryEventCategory("Input");
        scene_event_category_ = event_manager->QueryEventCategory("Scene");
        
        renderer_->PostInitialize();

        RegisterConsoleCommand(Console::CreateCommand(
                "RenderStats", "Prints out render statistics.", 
                Console::Bind(this, &OgreRenderingModule::ConsoleStats)));
        renderer_settings_ = RendererSettingsPtr(new RendererSettings(framework_));
    }

    // virtual
    bool OgreRenderingModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
    {
        PROFILE(OgreRenderingModule_HandleEvent);
        if (!renderer_)
            return false;

        if (category_id == asset_event_category_)
        {
            return renderer_->GetResourceHandler()->HandleAssetEvent(event_id, data);
        }

        if (category_id == resource_event_category_)
        {
            return renderer_->GetResourceHandler()->HandleResourceEvent(event_id, data);
        }

        if (category_id == input_event_category_ && event_id == Input::Events::INWORLD_CLICK)
        {
            // do raycast into the world when user clicks mouse button
            Input::Events::Movement *movement = checked_static_cast<Input::Events::Movement*>(data);
            assert(movement);
            Foundation::RaycastResult result = renderer_->Raycast(movement->x_.abs_, movement->y_.abs_);

            Scene::Entity *entity = result.entity_;
            if (entity)
            {
                Scene::Events::RaycastEventData event_data(entity->GetId());
                event_data.pos = result.pos_, event_data.submesh = result.submesh_, event_data.u = result.u_, event_data.v = result.v_; 
                framework_->GetEventManager()->SendEvent(scene_event_category_, Scene::Events::EVENT_ENTITY_GRAB, &event_data);

                //Semantically same as above but sends the entity pointer
                Scene::Events::EntityClickedData clicked_event_data(entity);
                framework_->GetEventManager()->SendEvent(scene_event_category_, Scene::Events::EVENT_ENTITY_CLICKED, &clicked_event_data);
            }
            else
                framework_->GetEventManager()->SendEvent(scene_event_category_, Scene::Events::EVENT_ENTITY_NONE_CLICKED, 0);
        }

        return false;
    }

    // virtual 
    void OgreRenderingModule::Uninitialize()
    {
        framework_->GetServiceManager()->UnregisterService(renderer_);

        // Explicitly remove log listener now, because the service has been
        // unregistered now, and no-one can get at the renderer to remove themselves
        if (renderer_)
            renderer_->RemoveLogListener();

        renderer_settings_.reset();
        renderer_.reset();
    }
    
    // virtual
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

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace OgreRenderer;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(OgreRenderingModule)
POCO_END_MANIFEST

