// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingSystem_EC_OgrePlaceable_h
#define incl_OgreRenderingSystem_EC_OgrePlaceable_h

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
    
    //! Ogre placeable component. Stores Ogre scenenode.
    /*! An initialized renderer module must exist before these components can be created.
     */
    class MODULE_API EC_OgrePlaceable : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_OgrePlaceable);
    public:
        virtual ~EC_OgrePlaceable();

        //! sets position
        /*! \param position new position 
         */
        void SetPosition(const Core::Vector3df& position);
        //! sets orientation
        /*! \param orientation new orientation
         */
        void SetOrientation(const Core::Quaternion& orientation);
        //! sets scale
        /*! \param scale new scale
         */
        void SetScale(const Core::Vector3df& scale);

        //! returns position
        Core::Vector3df GetPosition() const;
        //! returns orientation
        Core::Quaternion GetOrientation() const;
        //! returns scale
        Core::Vector3df GetScale() const;
        
        //! returns scenenode
        Ogre::SceneNode* GetSceneNode() const { return scene_node_; }

    private:
        //! constructor
        /*! \param module renderer module
         */
        EC_OgrePlaceable(Foundation::ModuleInterface* module);

        OgreRenderingModule* module_;
        Ogre::SceneNode* scene_node_;
    };
}

#endif
