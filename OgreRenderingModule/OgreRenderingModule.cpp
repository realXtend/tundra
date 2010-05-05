// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "ComponentRegistrarInterface.h"
#include "ServiceManager.h"
#include "ResourceHandler.h"
#include "OgreMeshResource.h"
#include "OgreTextureResource.h"
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
#include "EC_HoveringWidget.h"

#include "InputEvents.h"
#include "SceneEvents.h"
#include "Entity.h"
#include "ConsoleServiceInterface.h"
#include "ConsoleCommand.h"
#include "ConsoleCommandServiceInterface.h"
#include "RendererSettings.h"
#include "ConfigurationManager.h"
#include "EventManager.h"
#include <Ogre.h>


#include "SceneManager.h"

namespace OgreRenderer
{
    OgreRenderingModule::OgreRenderingModule() : ModuleInterfaceImpl(type_static_),
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
        using namespace OgreRenderer;

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
        DECLARE_MODULE_EC(EC_HoveringWidget);
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
        // create renderer here, so it can be accessed in uninitialized state by other module's PreInitialize()
        std::string window_title = framework_->GetDefaultConfig().GetSetting<std::string>(
            Foundation::Framework::ConfigurationGroup(), "window_title") + " " + VersionMajor() + "." + VersionMinor();

        renderer_ = OgreRenderer::RendererPtr(new OgreRenderer::Renderer(framework_, ogre_config_filename, plugins_filename, window_title));
    }

    // virtual
    void OgreRenderingModule::Initialize()
    {
        assert (renderer_);
        assert (!renderer_->IsInitialized());
        renderer_->Initialize();

        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Renderer, renderer_);
        
        renderer_settings_ = RendererSettingsPtr(new RendererSettings(framework_));
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
    }

    bool OgreRenderingModule::CheckInfoIconIntersection(int x, int y)
    {
        bool ret_val = false;
        QList<EC_HoveringWidget*> visible_widgets;
        Real scr_x = x/(Real)renderer_->GetWindowWidth();
        Real scr_y = y/(Real)renderer_->GetWindowHeight();

        Ogre::Ray original_ray = renderer_->GetCurrentCamera()->getCameraToViewportRay(scr_x, scr_y);

        Scene::ScenePtr current_scene = framework_->GetDefaultWorldScene();
        if (!current_scene.get())
            return ret_val;

        Scene::SceneManager::iterator iter = current_scene->begin();
        Scene::SceneManager::iterator end = current_scene->end();

        while (iter != end)
        {
            Scene::EntityPtr entity = (*iter);
            ++iter;
            if (!entity.get())
                continue;

            EC_HoveringWidget* widget = entity->GetComponent<EC_HoveringWidget>().get();
            if(widget && widget->IsVisible())
                visible_widgets.append(widget);
        }
        Ogre::Quaternion quat;
        if(!visible_widgets.empty())
            quat = visible_widgets.at(0)->GetBillboardSet().getParentSceneNode()->getOrientation();
        for(int i=0; i< visible_widgets.size();i++)
        {
            Ogre::Ray ray = original_ray;
            EC_HoveringWidget* widget = visible_widgets.at(i);
            Ogre::Billboard board = widget->GetBillboard();
            Ogre::Vector3 vec = board.getPosition();
            Ogre::Real billboard_h = widget->GetBillboardSet().getDefaultHeight();
            Ogre::Real billboard_w = widget->GetBillboardSet().getDefaultWidth();
            
            widget->GetBillboardSet().getParentSceneNode()->setOrientation(renderer_->GetCurrentCamera()->getDerivedOrientation());
            Ogre::Matrix4 mat;
            widget->GetBillboardSet().getWorldTransforms(&mat);


            Ogre::AxisAlignedBox box(vec.x, vec.y - billboard_w/2, vec.z - billboard_h/2, vec.x , vec.y + billboard_w/2, vec.z + billboard_h/2);
            //Transform cameratoviewportray into the billboards local space
            Ogre::Vector4 temp(ray.getDirection());
            temp.w = 0;
            temp = mat.inverseAffine() *temp;
            ray.setDirection(Ogre::Vector3(temp.x, temp.y, temp.z));
            ray.setOrigin(mat.inverseAffine() * ray.getOrigin());
            std::pair<bool, Real> result = ray.intersects(box);
            if(result.first)
            {
                Ogre::Vector3 inters_point = ray.getPoint(result.second);
                inters_point -= vec;
                Real x = (inters_point.y + billboard_w/2)/billboard_w;
                Real y = (inters_point.z + billboard_h/2)/billboard_h;
                widget->WidgetClicked(1-x, 1-y);
                ret_val = true;
                
            }
        }

        return ret_val;
    }

    // virtual
    bool OgreRenderingModule::HandleEvent(
        event_category_id_t category_id,
        event_id_t event_id, 
        Foundation::EventDataInterface* data)
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
            CheckInfoIconIntersection(movement->x_.abs_, movement->y_.abs_);
            Foundation::RaycastResult result = renderer_->Raycast(movement->x_.abs_, movement->y_.abs_);

            Scene::Entity *entity = result.entity_;

            if (entity)
            {
                //std::cout << "Raycast hit entity " << entity << " pos " << result.pos_.x << " " << result.pos_.y << " " << result.pos_.z
                //          << " submesh " << result.submesh_ << " uv " << result.u_ << " " << result.v_ << std::endl;
                
                Scene::Events::RaycastEventData event_data(entity->GetId());
         
                event_data.pos = result.pos_, event_data.submesh = result.submesh_, event_data.u = result.u_, event_data.v = result.v_; 
                framework_->GetEventManager()->SendEvent(scene_event_category_, Scene::Events::EVENT_ENTITY_GRAB, &event_data);
            
                //Semantically same as above but sends the entity pointer
                Scene::Events::EntityClickedData clicked_event_data(entity);
                framework_->GetEventManager()->SendEvent(scene_event_category_, Scene::Events::EVENT_ENTITY_CLICKED, &clicked_event_data);
            }
        }

        if (category_id == input_event_category_ && event_id == Input::Events::INWORLD_CLICK_BUILD)
        {
            Input::Events::Movement *movement = checked_static_cast<Input::Events::Movement*>(data);
            Foundation::RaycastResult result = renderer_->Raycast(movement->x_.abs_, movement->y_.abs_);

            Scene::Entity *entity = result.entity_;
            if (entity)
            {
                Scene::Events::CreateEntityEventData event_data(result.pos_);
                framework_->GetEventManager()->SendEvent(scene_event_category_, Scene::Events::EVENT_ENTITY_CREATE, &event_data);
            }
        }

        if (category_id == input_event_category_ && event_id == Input::Events::NAALI_BINDINGS_CHANGED)
        {
            Input::Events::BindingsData *bindings_data = dynamic_cast<Input::Events::BindingsData *>(data);
            if (bindings_data)
                renderer_->UpdateKeyBindings(bindings_data->bindings);
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
            boost::shared_ptr<Console::ConsoleServiceInterface> console = GetFramework()->GetService<Console::ConsoleServiceInterface>(Foundation::Service::ST_Console).lock();
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

