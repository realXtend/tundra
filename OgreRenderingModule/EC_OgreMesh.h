// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreMesh_h
#define incl_OgreRenderer_EC_OgreMesh_h

#include "ComponentInterface.h"
#include "Foundation.h"

namespace Ogre
{
    class Entity;
}

namespace OgreRenderer
{
    class Renderer;
    class EC_OgrePlaceable;
    
    typedef boost::shared_ptr<Renderer> RendererPtr;
    
    //! Ogre mesh component
    /*! Needs to be attached to a placeable (aka scene node) to be visible.
     */
    class MODULE_API EC_OgreMesh : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_OgreMesh);
    public:
        virtual ~EC_OgreMesh();

        //! gets placeable component
        Foundation::ComponentPtr GetPlaceable() const { return placeable_; }
        
        //! sets placeable component
        /*! set a null placeable to detach the mesh, otherwise will attach
            \param placeable placeable component
         */
        void SetPlaceable(Foundation::ComponentPtr placeable);
        
        //! sets mesh
        /*! if mesh already sets, removes the old one
            \param mesh_name mesh to use
            \return true if successful
            \todo use mesh asset reference when asset system exists
         */
        bool SetMesh(const std::string& mesh_name);
        
        //! gets number of materials (submeshes) in mesh
        Core::uint GetNumMaterials();
        
        //! sets material in mesh
        /*! \param index submesh index
            \param material_name material name
            \return true if successful
            \todo use material asset reference when asset system exists
         */
        bool SetMaterial(Core::uint index, const std::string& material_name);
        
        //! removes mesh
        void RemoveMesh();
        
        //! returns if mesh exists
        bool HasMesh() const { return entity_ != NULL; }
        
        //! returns mesh
        Ogre::Entity* GetMesh() const { return entity_; }
        
    private:
        //! constructor
        /*! \param module renderer module
         */
        EC_OgreMesh(Foundation::ModuleInterface* module);
        
        //! attaches mesh to placeable
        void AttachMesh();
        
        //! detaches mesh from placeable
        void DetachMesh();
        
        //! placeable component 
        Foundation::ComponentPtr placeable_;
        
        //! renderer
        RendererPtr renderer_;
        
        //! Ogre mesh entity
        Ogre::Entity* entity_;
        
        //! mesh entity attached to placeable -flag
        bool attached_;
    };
}

#endif
