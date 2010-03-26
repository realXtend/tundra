// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_OgreMeshResource_h
#define incl_OgreRenderer_OgreMeshResource_h

#include "AssetInterface.h"
#include "ResourceInterface.h"
#include "OgreModuleApi.h"

#include <OgreMesh.h>

namespace OgreRenderer
{
    class OgreMeshResource;
    typedef boost::shared_ptr<OgreMeshResource> OgreMeshResourcePtr;

    //! An Ogre-specific mesh resource
    /*! \ingroup OgreRenderingModuleClient
     */
    class OGRE_MODULE_API OgreMeshResource : public Foundation::ResourceInterface
    {
    public:
        //! constructor
        /*! \param id mesh id
         */
        OgreMeshResource(const std::string& id);
        
        //! constructor
        /*! \param id mesh id
            \param source asset data to construct mesh from
        */
        OgreMeshResource(const std::string& id, Foundation::AssetPtr source);

        //! destructor
        virtual ~OgreMeshResource();

        //! returns resource type in text form
        virtual const std::string& GetType() const;

        //! returns whether resource valid
        virtual bool IsValid() const;
        
        //! returns Ogre mesh
        /*! may be null if no data successfully set yet
         */
        Ogre::MeshPtr GetMesh() const { return ogre_mesh_; }

        //! sets contents from asset data
        /*! \param source asset data to construct mesh from
            \return true if successful
        */
        bool SetData(Foundation::AssetPtr source);

        //! returns resource type in text form (static)
        static const std::string& GetTypeStatic();
        
        //! returns original material names
        const StringVector& GetOriginalMaterialNames() const { return original_materials_; }
        
    private:
        //! Ogre mesh
        Ogre::MeshPtr ogre_mesh_;
        
        //! Original materials
        StringVector original_materials_;
        
        //! destroys mesh if exists
        void RemoveMesh();
    };
}

#endif
