// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Renderer.h"

#include "Ogre.h"

namespace OgreRenderer
{
    Renderer::Renderer() :
        root_(0),
        scenemanager_(0)
    {
#ifdef _DEBUG
        std::string plugins_filename = "pluginsd.cfg";
#else
        std::string plugins_filename = "plugins.cfg";
#endif
    
        root_ = new Ogre::Root(plugins_filename);
    }
    
    Renderer::~Renderer()
    {
        delete root_;
    }
}