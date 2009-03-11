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
        root_(0),
        scenemanager_(0)
    {
    }
    
    Renderer::~Renderer()
    {
        delete root_;
        root_ = 0;
    }
    
    bool Renderer::Initialize()
    {
#ifdef _DEBUG
        std::string plugins_filename = "pluginsd.cfg";
#else
        std::string plugins_filename = "plugins.cfg";
#endif
    
        root_ = new Ogre::Root(plugins_filename);
        
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
        root_->createRenderWindow(Application::Name(), width, height, fullscreen, &params);

        SetupResources();
        
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
}
