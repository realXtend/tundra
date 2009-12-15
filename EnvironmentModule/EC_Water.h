// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Water_h
#define incl_EC_Water_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "RexUUID.h"

namespace Ogre
{
    class SceneNode;
    class Entity;
}

namespace Environment
{
    /// Represents data related to a water plane.
    class EC_Water : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_Water);
    public:
        virtual ~EC_Water();

        /// Sets the new water plane height.
        void SetWaterHeight(float height);

        /// @return The currently active water plane height if water does not exist returns 0.0
        float GetWaterHeight() const;


    private:
        EC_Water(Foundation::ModuleInterface* module);
  
        void CreateOgreWaterObject();

        /// Just need to have this here to be able to destroy the Ogre data. ///\todo This pointer could theoretically dangle... 
        Foundation::ModuleInterface* owner_; 

        //! Ogre scene node
        Ogre::SceneNode *scene_node_;

        Ogre::Entity *entity_;

    };
}

#endif
