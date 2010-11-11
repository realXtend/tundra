// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AssImp_OpenAssetImport_h
#define incl_AssImp_OpenAssetImport_h

#include <assimp.hpp>
#include <LogStream.h>
#include <aiScene.h>
#include "Transform.h"
namespace OgreRenderer { class Renderer; }

//! Open Asset Import, a wrapper for Open Asset Import library that is used for loading model formats other than Ogre .mesh.
namespace AssImp
{
    struct MeshData
    {
        QString file_;
        QString name_;
        Transform transform_;
    };

    /*! Imports an Ogre mesh from various different model formats.
        The Ogre mesh is created to Ogre::MeshManager. This class
        doesn't create any entities or components, the caller is
        responsible for this.
    */
    class OpenAssetImport
    {
    public:
		OpenAssetImport();
		~OpenAssetImport();
        
        //! Returns true if filename has an extension that implies supported file format
        bool IsSupportedExtension(const QString& filename);

        //! Imports mesh data from a file.
        /*! Import mesh names and transformations contained in the model file. This
            information can be used to create entities, components ands asset refs.
            Use Import() to create the actual mesh data.

            \note Does not handle scene hierarchy, all transformations are converted
                  to world space.

            \param file path to file where to import meshes from
            \param outMeshData Out string vector of mesh names
        */
        void GetMeshData(const QString& file, std::vector<MeshData> &outMeshData);


        //! Generates Ogre meshes from memory buffer.
        /*! The meshes are generated directly to Ogre::MeshManager and names of the meshes are
            returned. Use GetMeshData() to get hierarchy and transformation data from a model file.

            \param data memory buffer where to import meshes from
            \param length memory buffer length
            \param name file format hint for the importer, looks for extension within the name
            \param hint file format hint for the importer, in practise file extension with the dot included
            \param node name of the node to import
            \param outMeshNames Out string vector of generated Ogre mesh names
        */
        void Import(const void *data, size_t length, const QString &name, const char* hint, const QString &nodeName, std::vector<std::string> &outMeshNames);

	private:
        class AssImpLogStream : public Assimp::LogStream
        {
        public:
            AssImpLogStream() {}
            virtual ~AssImpLogStream() {}

            void write(const char* message);
        };

        void GetNodeData(const aiScene *scene, const aiNode *node, const QString& file,
            const aiMatrix4x4 &parentTransform, std::vector<MeshData> &outMeshNames);
        
        void ImportNode(const struct aiScene *scene, const struct aiNode *node, const QString& file, const QString &nodeName, 
            std::vector<std::string> &outMeshNames);

		boost::shared_ptr<Assimp::Importer> importer_;
        AssImpLogStream *logstream_;

        const unsigned int loglevels_;     //! Log levels to capture during import
        const unsigned int default_flags_; //! Default import postprocess flags
	};
}
#endif
