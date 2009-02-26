
#ifndef __incl_OgreRenderingSystem_EC_OgreEntity_h__
#define __incl_OgreRenderingSystem_EC_OgreEntity_h__

#include "ComponentInterface.h"
#include "Foundation.h"
#include "EC_Geometry.h"
#include <Poco/Delegate.h>

namespace OgreRenderer
{
    //! Ogre entity component. Stores Ogre scenenode.
    class EC_OgreEntity : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_OgreEntity);
    public:
        virtual ~EC_OgreEntity() {}

    private:
        EC_OgreEntity() : OgreSceneNode(0) { }

        int OgreSceneNode;
    };
}

#endif
