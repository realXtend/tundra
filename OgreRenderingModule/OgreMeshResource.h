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
        OgreMeshResource(const std::string& id, Foundation::AssetInterfacePtr source);

        //! destructor
        virtual ~OgreMeshResource();

        //! returns resource type in text form
        virtual const std::string& GetType() const;

        //! returns whether resource valid
        virtual bool IsValid() const;

        //! Export Ogre mesh to the specified file
        /*! \param filename full path to file
        */
        virtual bool Export(const std::string &filename) const;
        
        //! returns Ogre mesh
        /*! may be null if no data successfully set yet
         */
        Ogre::MeshPtr GetMesh() const { return ogre_mesh_; }

        //! sets contents from asset data
        /*! \param source asset data to construct mesh from
            \return true if successful
        */
        bool SetData(Foundation::AssetInterfacePtr source);

        //! returns resource type in text form (static)
        static const std::string& GetTypeStatic();
        
        //! returns original material names
        const StringVector& GetOriginalMaterialNames() const { return original_materials_; }

        //! Resets asset references to what the mesh currently has
        void ResetReferences();
        
    private:
        //! Set default material for the mesh that won't complain
        void SetDefaultMaterial();

        //! Ogre mesh
        Ogre::MeshPtr ogre_mesh_;
        
        //! Original materials
        StringVector original_materials_;
        
        //! destroys mesh if exists
        void RemoveMesh();
    };
}

#endif
