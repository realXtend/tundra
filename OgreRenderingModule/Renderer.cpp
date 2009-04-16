// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Renderer.h"
#include "RendererEvents.h"
#include "OgreRenderingModule.h"

#include <Ogre.h>

using namespace Foundation;

namespace OgreRenderer
{
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

                Event::WindowResized data(rw->getWidth(), rw->getHeight());
                renderer_->framework_->GetEventManager()->SendEvent(renderer_->event_category_, Event::WINDOW_RESIZED, &data);
            }
        }
    
        void windowClosed(Ogre::RenderWindow* rw)
        {
            if (rw == renderer_->renderwindow_)
            {
                renderer_->framework_->Exit();
                renderer_->framework_->GetEventManager()->SendEvent(renderer_->event_category_, Event::WINDOW_CLOSED, NULL);
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

    Renderer::Renderer(Framework* framework) :
        initialized_(false),
        framework_(framework),
        scenemanager_(NULL),
        camera_(NULL),
        renderwindow_(NULL),
        object_id_(0),
        listener_(EventListenerPtr(new EventListener(this))),
        log_listener_(OgreLogListenerPtr(new LogListener))
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        
        event_category_ = event_manager->RegisterEventCategory("Renderer");
        event_manager->RegisterEvent(event_category_, Event::POST_RENDER, "PostRender");
        event_manager->RegisterEvent(event_category_, Event::WINDOW_CLOSED, "WindowClosed");
    }
    
    Renderer::~Renderer()
    {
        if (initialized_)
        {
            Ogre::WindowEventUtilities::removeWindowEventListener(renderwindow_, listener_.get());
            //Ogre::LogManager::getSingleton().getDefaultLog()->removeListener(log_listener_.get());
        }

        root_.reset();
    }
    
    void Renderer::Initialize()
    {
        if (initialized_)
            return;
            
#ifdef _DEBUG
        std::string plugins_filename = "pluginsd.cfg";
#else
        std::string plugins_filename = "plugins.cfg";
#endif
    
        std::string logfilepath = framework_->GetPlatform()->GetUserDocumentsDirectory();
        logfilepath += "/Ogre.log";

        root_ = OgreRootPtr(new Ogre::Root("", "ogre.cfg", logfilepath));
        Ogre::LogManager::getSingleton().getDefaultLog()->addListener(log_listener_.get());
        LoadPlugins(plugins_filename);
        
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
        if (renderwindow_)
            renderwindow_->getCustomAttribute("WINDOW", &window_handle);
        
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
    
    void Renderer::Render()
    {
        if (!initialized_) return;
        
        root_->_fireFrameStarted();
        
        // Render without swapping buffers first
        Ogre::RenderSystem* renderer = root_->getRenderSystem();
        renderer->_updateAllRenderTargets(false);
        // Send postrender event, so that custom rendering may be added
        framework_->GetEventManager()->SendEvent(event_category_, Event::POST_RENDER, NULL);
        // Swap buffers now
        renderer->_swapAllRenderTargetBuffers(renderer->getWaitForVerticalBlank());

        root_->_fireFrameEnded();
    }
    
    std::string Renderer::GetUniqueObjectName()
    {
        return "obj" + Core::ToString<Core::uint>(object_id_++);
    }
}

