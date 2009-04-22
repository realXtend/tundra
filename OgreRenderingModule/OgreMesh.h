// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_OgreMesh_h
#define incl_OgreRenderer_OgreMesh_h

#include "AssetInterface.h"
#include "ResourceInterface.h"

#include <OgreMesh.h>

namespace OgreRenderer
{
    class OgreMesh;
    typedef boost::shared_ptr<OgreMesh> OgreMeshPtr;

    //! an Ogre mesh resource
    class OgreMesh : public Foundation::ResourceInterface
    {
    public:
        //! constructor
        /*! \param id mesh id
         */
        OgreMesh(const std::string& id);
        
        //! constructor
        /*! \param id mesh id
            \param source asset data to construct mesh from
        */
        OgreMesh(const std::string& id, Foundation::AssetPtr source);

        //! destructor
        virtual ~OgreMesh();

        //! returns resource type in text form
        virtual const std::string& GetTypeName() const;

        //! returns Ogre mesh
        /*! may be null if no data successfully set yet
         */
        Ogre::MeshPtr GetMesh() const { return ogre_mesh_; }

        //! sets contents from asset data
        /*! \param source asset data to construct mesh from
            \return true if successful
        */
        bool SetData(Foundation::AssetPtr source);

    private:
        //! Ogre mesh
        Ogre::MeshPtr ogre_mesh_;
    };
}

#endif
