// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Foundation.h"
#include "Renderer.h"
#include "OgreRenderingModule.h"

#include "Ogre.h"

using namespace Foundation;

namespace OgreRenderer
{
    Renderer::Renderer(OgreRenderingModule* module, Framework* framework) :
        framework_(framework),
        module_(module),
        scenemanager_(0),
        camera_(0),
        renderwindow_(0),
        initialized_(false)
    {
    }
    
    Renderer::~Renderer()
    {
        if (initialized_)
            Ogre::WindowEventUtilities::removeWindowEventListener(renderwindow_, this);

        root_.reset();
    }
    
    bool Renderer::Initialize()
    {
#ifdef _DEBUG
        std::string plugins_filename = "pluginsd.cfg";
#else
        std::string plugins_filename = "plugins.cfg";
#endif
    
        std::string logfilepath = framework_->GetPlatform()->GetUserDocumentsDirectory();
        logfilepath += "/Ogre.log";

        root_ = OgreRootPtr(new Ogre::Root(plugins_filename, "ogre.cfg", logfilepath));
        
#ifdef _WINDOWS
        std::string rendersystem_name = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "RenderSystem", "Direct3D9 Rendering Subsystem");
#else
        std::string rendersystem_name = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "RenderSystem", "OpenGL Rendering Subsystem");
#endif
        int width = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "WindowWidth", 800);
        int height = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "WindowHeight", 600);
        bool fullscreen = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "Fullscreen", false);
        
        Ogre::RenderSystem* rendersystem = root_->getRenderSystemByName(rendersystem_name);
        if (!rendersystem)
        {
            module_->LogError("Could not find rendersystem");
            return false;
        }
        root_->setRenderSystem(rendersystem);
        root_->initialise(false);

        Ogre::NameValuePairList params;
        std::string application_name = framework_->GetDefaultConfig().GetString(Foundation::Framework::ConfigurationGroup(), "application_name");
        renderwindow_ = root_->createRenderWindow(application_name, width, height, fullscreen, &params);
        if (!renderwindow_)
        {
            module_->LogError("Could not create rendering window");
            return false;
        }

        SetupResources();
        SetupScene();
        
        Ogre::WindowEventUtilities::addWindowEventListener(renderwindow_, this);
        
        initialized_ = true;
        return true;
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
    
    void Renderer::windowResized(Ogre::RenderWindow* rw)
    {
        if ((camera_) && (rw == renderwindow_))
        {
            camera_->setAspectRatio(Ogre::Real(rw->getWidth() / Ogre::Real(rw->getHeight())));
        }
    }
    
    void Renderer::windowClosed(Ogre::RenderWindow* rw)
    {
        if (rw == renderwindow_)
            framework_->Exit();
    }
}

