// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingSystem_EC_OgreMesh_h
#define incl_OgreRenderingSystem_EC_OgreMesh_h

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
    /*! Needs knowledge of a placeable (aka scene node) to work properly.
     */
    class MODULE_API EC_OgreMesh : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_OgreMesh);
    public:
        virtual ~EC_OgreMesh();

        //! gets placeable component
        Foundation::ComponentPtr GetPlaceable() const { return placeable_; }
        
        //! sets placeable component
        void SetPlaceable(Foundation::ComponentPtr placeable) { placeable_ = placeable; }
        
        //! sets mesh
        /*! if mesh already sets, removes the old one
            \param mesh_name mesh to use
            \return true if successful
            \todo use mesh asset reference when assetcache exists
         */
        bool SetMesh(const std::string& mesh_name);
        
        //! gets number of materials (submeshes) in mesh
        Core::uint GetNumMaterials();
        
        //! sets material in mesh
        /*! \param index submesh index
            \param material_name material name
            \return true if successful
            \todo use material asset reference when assetcache exists
         */
        bool SetMaterial(unsigned index, const std::string& material_name);
        
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
        
        //! 
        Foundation::ComponentPtr placeable_;
        RendererPtr renderer_;
        
        Ogre::Entity* entity_;
    };
}

#endif
