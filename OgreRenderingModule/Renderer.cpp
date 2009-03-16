// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Foundation.h"
#include "Renderer.h"
#include "OgreRenderingModule.h"

#include "Ogre.h"

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
            if ((renderer_->camera_) && (rw == renderer_->renderwindow_))
                renderer_->camera_->setAspectRatio(Ogre::Real(rw->getWidth() / Ogre::Real(rw->getHeight())));
        }
    
        void windowClosed(Ogre::RenderWindow* rw)
        {
            if (rw == renderer_->renderwindow_)
                renderer_->framework_->Exit();
        }
        
    private:
        Renderer* renderer_;
    };

    Renderer::Renderer(Framework* framework) :
        initialized_(false),
        framework_(framework),
        scenemanager_(0),
        camera_(0),
        renderwindow_(0),
        listener_(EventListenerPtr(new EventListener(this)))
    {
    }
    
    Renderer::~Renderer()
    {
        if (initialized_)
            Ogre::WindowEventUtilities::removeWindowEventListener(renderwindow_, listener_.get());

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
        LoadPlugins(plugins_filename);
        
#ifdef _WINDOWS
        std::string rendersystem_name = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "RenderSystem", "Direct3D9 Rendering Subsystem");
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
        rendersystem->setConfigOption("Floating-point mode", "Consistent");
        
        root_->setRenderSystem(rendersystem);
        root_->initialise(false);

        Ogre::NameValuePairList params;
        std::string application_name = framework_->GetDefaultConfig().GetString(Foundation::Framework::ConfigurationGroup(), "application_name");

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
        
        for (unsigned i = 0; i < plugins.size(); ++i)
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
        
        // Initialize resource groups
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    }
    
    void Renderer::SetupScene()
    {
        // Create scene manager
        scenemanager_ = root_->createSceneManager(Ogre::ST_GENERIC, "SceneManager");

        // Create the camera
        camera_ = scenemanager_->createCamera("Camera");

        // Create one viewport, entire window
        Ogre::Viewport* viewport = renderwindow_->addViewport(camera_);

        // Alter the camera aspect ratio to match the viewport
        camera_->setAspectRatio(Ogre::Real(viewport->getActualWidth()) / Ogre::Real(viewport->getActualHeight()));
    }
    
    void Renderer::Update()
    {
        if (!initialized_) return;
        
        Ogre::WindowEventUtilities::messagePump();
        root_->renderOneFrame();
    }
    

}

