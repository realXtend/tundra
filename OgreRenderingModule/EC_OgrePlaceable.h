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
    
    typedef boost::shared_ptr<Renderer> RendererPtr;
    
    //! Ogre placeable (scene node) component
    class MODULE_API EC_OgrePlaceable : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_OgrePlaceable);
    public:
        virtual ~EC_OgrePlaceable();

        //! sets parent placeable
        /*! set null placeable to attach to scene root (the default)
            \param placeable new parent
         */
        void SetParent(Foundation::ComponentPtr placeable);
        
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

        //! gets parent placeable
        Foundation::ComponentPtr GetParent() { return parent_; }
        
        //! returns position
        Core::Vector3df GetPosition() const;
        //! returns orientation
        Core::Quaternion GetOrientation() const;
        //! returns scale
        Core::Vector3df GetScale() const;
        
        //! returns Ogre scenenode
        Ogre::SceneNode* GetSceneNode() const { return scene_node_; }

    private:
        //! constructor
        /*! \param module renderer module
         */
        EC_OgrePlaceable(Foundation::ModuleInterface* module);
        
        //! attaches scenenode to parent
        void AttachNode();
        
        //! detaches scenenode from parent
        void DetachNode();
        
        //! renderer
        RendererPtr renderer_;
        
        //! parent placeable
        Foundation::ComponentPtr parent_;
        
        //! Ogre scene node
        Ogre::SceneNode* scene_node_;
    };
}

#endif
