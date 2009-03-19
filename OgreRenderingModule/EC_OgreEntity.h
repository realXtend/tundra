// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingSystem_EC_OgreEntity_h
#define incl_OgreRenderingSystem_EC_OgreEntity_h

#include "ComponentInterface.h"
#include "Foundation.h"

namespace Ogre
{
    class SceneNode;
}

namespace OgreRenderer
{
    class Renderer;
    class OgreRenderingModule;
    
    //! Ogre entity component. Stores Ogre scenenode.
    class VIEWER_API EC_OgreEntity : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_OgreEntity);
    public:
        virtual ~EC_OgreEntity();

    private:
        EC_OgreEntity(Foundation::ModuleInterface* module);

        OgreRenderingModule* module_;
        Ogre::SceneNode* scene_node_;
    };
}

#endif
