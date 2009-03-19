// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingSystem_EC_OgreEntity_h
#define incl_OgreRenderingSystem_EC_OgreEntity_h

#include "ComponentInterface.h"
#include "Foundation.h"

namespace OgreRenderer
{
    //! Ogre entity component. Stores Ogre scenenode.
    class EC_OgreEntity : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_OgreEntity);
    public:
        virtual ~EC_OgreEntity() {}

    private:
        EC_OgreEntity(Foundation::ModuleInterface* module) : ogre_scene_node_(0) { }
        //! todo add copy constructor and assignment operator

        int ogre_scene_node_;
    };
}

#endif
