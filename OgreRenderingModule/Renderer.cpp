// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Renderer.h"
#include "RendererEvents.h"
#include "OgreRenderingModule.h"
#include "OgreTexture.h"
#include "OgreMesh.h"
#include "ResourceInterface.h"
#include "AssetEvents.h"

#include <Ogre.h>

using namespace Foundation;

namespace OgreRenderer
{
    //! Responds to Ogre's window events and manages the Realxtend world render window.
    class EventListener : public Ogre::WindowEventListener
    {
    public:
        EventListener(Renderer* renderer) :
            renderer_(renderer)
        {
        }

        ~EventListener() 
        {
        }
        
        void windowResized(Ogre::RenderWindow* rw)
        {
            if (rw == renderer_->renderwindow_)
            {
                if (renderer_->camera_)
                    renderer_->camera_->setAspectRatio(Ogre::Real(rw->getWidth() / Ogre::Real(rw->getHeight())));

                Events::WindowResized data(rw->getWidth(), rw->getHeight());
                renderer_->framework_->GetEventManager()->SendEvent(renderer_->renderercategory_id_, Events::WINDOW_RESIZED, &data);
            }
        }
    
        void windowClosed(Ogre::RenderWindow* rw)
        {
            if (rw == renderer_->renderwindow_)
            {
                renderer_->OnWindowClosed();
            }
        }
        
    private:
        Renderer* renderer_;
    };

/////////////////////////////////////////////////////////////

    //! Ogre log listener, for passing ogre log messages
    class LogListener : public Ogre::LogListener
    {
    public:
        LogListener() : Ogre::LogListener() {}
        virtual ~LogListener() {}

        //! Subscribe new listener for ogre log messages
        void SubscribeListener(const Foundation::LogListenerPtr &listener)
        {
            listeners_.push_back(listener);
        }

        //! Unsubscribe listener for ogre log messages
        void UnsubscribeListener(const Foundation::LogListenerPtr &listener)
        {
            listeners_.remove(listener);
        }

        void messageLogged( const std::string& message, Ogre::LogMessageLevel lml, bool maskDebug, const std::string &logName )
        {
            for (ListenerList::iterator listener = listeners_.begin() ;
                  listener != listeners_.end() ; 
                  ++listener)
            {
                (*listener)->LogMessage(message);
            }
        }
    private:
        typedef std::list<Foundation::LogListenerPtr> ListenerList;

        //! list of subscribed listeners
        ListenerList listeners_;
    };

/////////////////////////////////////////////////////////

    Renderer::Renderer(Framework* framework, const std::string& config, const std::string& plugins) :
        initialized_(false),
        framework_(framework),
        scenemanager_(NULL),
        camera_(NULL),
        renderwindow_(NULL),
        object_id_(0),
        main_window_handle_(0),
        listener_(EventListenerPtr(new EventListener(this))),
        log_listener_(OgreLogListenerPtr(new LogListener)),
        config_filename_ (config),
        plugins_filename_ (plugins)
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        
        renderercategory_id_ = event_manager->RegisterEventCategory("Renderer");
        event_manager->RegisterEvent(renderercategory_id_, Events::POST_RENDER, "PostRender");
        event_manager->RegisterEvent(renderercategory_id_, Events::WINDOW_CLOSED, "WindowClosed");
        event_manager->RegisterEvent(renderercategory_id_, Events::WINDOW_RESIZED, "WindowResized");
    }
    
    Renderer::~Renderer()
    {
        if (initialized_)
        {
            Ogre::WindowEventUtilities::removeWindowEventListener(renderwindow_, listener_.get());
            //Ogre::LogManager::getSingleton().getDefaultLog()->removeListener(log_listener_.get());
        }

        textures_.clear();
        meshes_.clear();

        root_.reset();
    }
    
    void Renderer::Initialize()
    {
        if (initialized_)
            return;
            
        std::string logfilepath = framework_->GetPlatform()->GetUserDocumentsDirectory();
        logfilepath += "/Ogre.log";

        root_ = OgreRootPtr(new Ogre::Root("", config_filename_, logfilepath));
        Ogre::LogManager::getSingleton().getDefaultLog()->addListener(log_listener_.get());
        LoadPlugins(plugins_filename_);
        
#ifdef _WINDOWS
        std::string rendersystem_name = framework_->GetDefaultConfig().DeclareSetting<std::string>("OgreRenderer", "RenderSystem", "Direct3D9 Rendering Subsystem");
#else
        std::string rendersystem_name = "OpenGL Rendering Subsystem";
        framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "RenderSystem", rendersystem_name);
#endif
        int width = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "WindowWidth", 800);
        int height = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "WindowHeight", 600);
        bool fullscreen = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "Fullscreen", false);
        
        Ogre::RenderSystem* rendersystem = root_->getRenderSystemByName(rendersystem_name);
        
        if (!rendersystem)
        {
            throw Core::Exception("Could not find Ogre rendersystem.");
        }

        // GTK's pango/cairo/whatever's font rendering doesn't work if the floating point mode is not set to strict.
        // This however creates undefined behavior for D3D (refrast + D3DX), but we aren't using those anyway.
        Ogre::ConfigOptionMap& map = rendersystem->getConfigOptions();
        if (map.find("Floating-point mode") != map.end())
            rendersystem->setConfigOption("Floating-point mode", "Consistent");
        
        root_->setRenderSystem(rendersystem);
        root_->initialise(false);

        Ogre::NameValuePairList params;
        std::string application_name = framework_->GetDefaultConfig().GetSetting<std::string>(Foundation::Framework::ConfigurationGroup(), "application_name");

        if (!external_window_parameter_.empty()) 
            params["externalWindowHandle"] = external_window_parameter_;

        try
        {
            renderwindow_ = root_->createRenderWindow(application_name, width, height, fullscreen, &params);
        }
        catch (Ogre::Exception e) {}
        
        if (!renderwindow_)
        {
            throw Core::Exception("Could not create Ogre rendering window");
        }

        SetupResources();
        SetupScene();
        
        Ogre::WindowEventUtilities::addWindowEventListener(renderwindow_, listener_.get());
        
        initialized_ = true;
    }

    void Renderer::PostInitialize()
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        
        resourcecategory_id_ = event_manager->QueryEventCategory("Resource");
    }

    void Renderer::LoadPlugins(const std::string& plugin_filename)
    {
        Ogre::ConfigFile file;
        try
        {
            file.load(plugin_filename);
        }
        catch (Ogre::Exception e)
        {
            OgreRenderingModule::LogError("Could not load Ogre plugins configuration file");
            return;
        }

        Ogre::String plugin_dir = file.getSetting("PluginFolder");
        Ogre::StringVector plugins = file.getMultiSetting("Plugin");
        
        if (plugin_dir.length())
        {
            if ((plugin_dir[plugin_dir.length() - 1] != '\\') && (plugin_dir[plugin_dir.length() - 1] != '/'))
            {
#ifdef _WINDOWS
                plugin_dir += "\\";
#else
                plugin_dir += "/";
#endif
            }
        }
        
        for (Core::uint i = 0; i < plugins.size(); ++i)
        {
            try
            {
                root_->loadPlugin(plugin_dir + plugins[i]);
            }
            catch (Ogre::Exception e)
            {
                OgreRenderingModule::LogError("Plugin " + plugins[i] + " failed to load");
            }
        }
    }
    
    void Renderer::SetupResources()
    {
        Ogre::ConfigFile cf;
        cf.load("resources.cfg");

        Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
        Ogre::String sec_name, type_name, arch_name;
        
        while(seci.hasMoreElements())
        {
            sec_name = seci.peekNextKey();
            Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();
            Ogre::ConfigFile::SettingsMultiMap::iterator i;
            for(i = settings->begin(); i != settings->end(); ++i)
            {
                type_name = i->first;
                arch_name = i->second;
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch_name, type_name, sec_name);
            }
        }
        
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    }
    
    void Renderer::SetupScene()
    {
        scenemanager_ = root_->createSceneManager(Ogre::ST_GENERIC, "SceneManager");
        camera_ = scenemanager_->createCamera("Camera");
        camera_->setNearClipDistance(0.1f);
        camera_->setFarClipDistance(2000.f);
        Ogre::Viewport* viewport = renderwindow_->addViewport(camera_);
        camera_->setAspectRatio(Ogre::Real(viewport->getActualWidth()) / Ogre::Real(viewport->getActualHeight()));
    }

    size_t Renderer::GetWindowHandle() const
    {
        size_t window_handle = 0;

        if (external_window_parameter_.empty())
        {
            if (renderwindow_)
                renderwindow_->getCustomAttribute("WINDOW", &window_handle);
        } else
        {
            window_handle = main_window_handle_;
        }

        return window_handle;
    }
    int Renderer::GetWindowWidth() const
    {
        int size = 0;
        if (renderwindow_)
            size = renderwindow_->getWidth();
        return size;
    }
    int Renderer::GetWindowHeight() const
    {
        int size = 0;
        if (renderwindow_)
            size = renderwindow_->getHeight();
        return size;
    }

    void Renderer::SubscribeLogListener(const Foundation::LogListenerPtr &listener)
    {
        log_listener_->SubscribeListener(listener);
    }

    void Renderer::UnsubscribeLogListener(const Foundation::LogListenerPtr &listener)
    {
        log_listener_->UnsubscribeListener(listener);
    }
    
    void Renderer::Update(Core::f64 frametime)
    {
        Ogre::WindowEventUtilities::messagePump();
    }
    
    void Renderer::Resize(Core::uint width, Core::uint height)
    {
        if (renderwindow_)
        {
            renderwindow_->resize(width, height);
        }
    }

    void Renderer::OnWindowClosed()
    {
        framework_->Exit();
        framework_->GetEventManager()->SendEvent(renderercategory_id_, Events::WINDOW_CLOSED, NULL);
    }

    void Renderer::Render()
    {
        if (!initialized_) return;
        
        root_->_fireFrameStarted();
        
        // Render without swapping buffers first
        Ogre::RenderSystem* renderer = root_->getRenderSystem();
        renderer->_updateAllRenderTargets(false);
        // Send postrender event, so that custom rendering may be added
        framework_->GetEventManager()->SendEvent(renderercategory_id_, Events::POST_RENDER, NULL);
        // Swap buffers now
        renderer->_swapAllRenderTargetBuffers(renderer->getWaitForVerticalBlank());

        root_->_fireFrameEnded();
    }
    
    std::string Renderer::GetUniqueObjectName()
    {
        return "obj" + Core::ToString<Core::uint>(object_id_++);
    }

    bool Renderer::HandleAssetEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id == Asset::Events::ASSET_READY)
        {
            Asset::Events::AssetReady *event_data = checked_static_cast<Asset::Events::AssetReady*>(data); 
            if (event_data->asset_type_ == "Mesh")
            {
                // Check that the request tag matches our request, so we do not (possibly) update unnecessarily many times
                // because of others' requests     
                if (expected_request_tags_.find(event_data->tag_) != expected_request_tags_.end())                      
                    UpdateMesh(event_data->asset_, event_data->tag_);
            }
        }

        return false;
    }

    bool Renderer::HandleResourceEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id == Resource::Events::RESOURCE_READY)
        {     
            Resource::Events::ResourceReady *event_data = checked_static_cast<Resource::Events::ResourceReady*>(data);  
            if (event_data->resource_)
            {
                if (event_data->resource_->GetTypeName() == "Texture")
                {
                    // Check that the request tag matches our request, so we do not (possibly) update unnecessarily many times
                    // because of others' requests
                    if (expected_request_tags_.find(event_data->tag_) != expected_request_tags_.end())
                        UpdateTexture(event_data->resource_, event_data->tag_);
                }
            }
        }

        return false;
    }


    Core::request_tag_t Renderer::RequestTexture(const std::string& id)
    {
        Core::request_tag_t tag = framework_->GetEventManager()->GetNextRequestTag();
            
        // See if already have the texture and at maximum quality level
        Foundation::ResourcePtr tex = GetTexture(id);
        if (tex)
        {
            if (checked_static_cast<OgreTexture*>(tex.get())->GetLevel() == 0)
            {
                Resource::Events::ResourceReady* event_data = new Resource::Events::ResourceReady(tex->GetId(), tex, tag);
                framework_->GetEventManager()->SendDelayedEvent(resourcecategory_id_, Resource::Events::RESOURCE_READY, Foundation::EventDataPtr(event_data));
                return tag;
            }
        }           

        // Request from texture decoder
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager(); 
        if (service_manager->IsRegistered(Foundation::Service::ST_Texture))
        {
            request_tags_[id].push_back(tag);

            boost::shared_ptr<Foundation::TextureServiceInterface> texture_service = service_manager->GetService<Foundation::TextureServiceInterface>(Foundation::Service::ST_Texture).lock();
            Core::request_tag_t source_tag = texture_service->RequestTexture(id);
            if (source_tag)
            {
                expected_request_tags_.insert(source_tag);
                return tag;
            }
        }
        
        return 0;
    }


    Foundation::ResourcePtr Renderer::GetTexture(const std::string& id)
    {
        Foundation::ResourceMap::iterator i = textures_.find(id);
        if (i == textures_.end())
            return Foundation::ResourcePtr();
        else
            return i->second;
    }

    void Renderer::RemoveTexture(const std::string& id)
    {
        Foundation::ResourceMap::iterator i = textures_.find(id);
        if (i == textures_.end())
            return;
        else
            textures_.erase(i);
    }

    bool Renderer::UpdateTexture(Foundation::ResourcePtr source, Core::request_tag_t tag)
    {
        Foundation::TexturePtr source_tex = boost::shared_dynamic_cast<Foundation::TextureInterface>(source);
        if (!source_tex) 
            return false;
    
        // If not found, prepare new
        Foundation::ResourcePtr tex = GetTexture(source_tex->GetId());
        if (!tex)
        {
            tex = textures_[source_tex->GetId()] = Foundation::ResourcePtr(new OgreTexture(source_tex->GetId()));
        }

        // If highest level, erase texture decode request tag (should not get more raw resource events for this texture)
        if (source_tex->GetLevel() == 0)
            expected_request_tags_.erase(tag);

        // If success, send Ogre resource ready event
        bool success = false;
        if (checked_static_cast<OgreTexture*>(tex.get())->SetData(source_tex))
        {
            const Core::RequestTagVector& tags = request_tags_[source_tex->GetId()];            
            for (Core::uint i = 0; i < tags.size(); ++i)
            {
                Resource::Events::ResourceReady event_data(tex->GetId(), tex, tags[i]);
                framework_->GetEventManager()->SendEvent(resourcecategory_id_, Resource::Events::RESOURCE_READY, &event_data);
            }
           
            success = true;            
        }

        // If highest level, erase also request tags 
        if (source_tex->GetLevel() == 0)
            request_tags_.erase(source_tex->GetId());

        return success;
    }    

    Core::request_tag_t Renderer::RequestMesh(const std::string& id)
    {
        Core::request_tag_t tag = framework_->GetEventManager()->GetNextRequestTag();
        
        // See if already have the mesh with data
        Foundation::ResourcePtr mesh = GetMesh(id);
        if (mesh)
        {
            if (!checked_static_cast<OgreMesh*>(mesh.get())->GetMesh().isNull())
            {
                Resource::Events::ResourceReady* event_data = new Resource::Events::ResourceReady(mesh->GetId(), mesh, tag);
                framework_->GetEventManager()->SendDelayedEvent(resourcecategory_id_, Resource::Events::RESOURCE_READY, Foundation::EventDataPtr(event_data));
                return tag;
            }
        }
        
        // Request from asset system
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager(); 
        if (service_manager->IsRegistered(Foundation::Service::ST_Asset))
        {
            request_tags_[id].push_back(tag);

            boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = service_manager->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
            Core::request_tag_t source_tag = asset_service->RequestAsset(id, "Mesh");
            if (source_tag) 
            {
                expected_request_tags_.insert(source_tag);
                return tag;
            }           
        }

        return 0;
    }

    Foundation::ResourcePtr Renderer::GetMesh(const std::string& id)
    {
        Foundation::ResourceMap::iterator i = meshes_.find(id);
        if (i == meshes_.end())
            return Foundation::ResourcePtr();
        else
            return i->second;
    }

    void Renderer::RemoveMesh(const std::string& id)
    {
        Foundation::ResourceMap::iterator i = meshes_.find(id);
        if (i == meshes_.end())
            return;
        else
            meshes_.erase(i);
    }

    bool Renderer::UpdateMesh(Foundation::AssetPtr source, Core::request_tag_t tag)
    {    
        expected_request_tags_.erase(tag);
            
        // If not found, prepare new
        Foundation::ResourcePtr mesh = GetMesh(source->GetId());
        if (!mesh)
        {
            mesh = meshes_[source->GetId()] = Foundation::ResourcePtr(new OgreMesh(source->GetId()));
        }

        // If success, send Ogre resource ready event
        bool success = false;
        if (checked_static_cast<OgreMesh*>(mesh.get())->SetData(source))
        {
            const Core::RequestTagVector& tags = request_tags_[source->GetId()];            
            for (Core::uint i = 0; i < tags.size(); ++i)
            {        
                Resource::Events::ResourceReady event_data(source->GetId(), mesh, tags[i]);
                framework_->GetEventManager()->SendEvent(resourcecategory_id_, Resource::Events::RESOURCE_READY, &event_data);
            }
            
            success = true;
        }
        
        request_tags_.erase(source->GetId());
        return success;
    }    
}

