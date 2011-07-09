// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include <assimp.hpp>
#include <LogStream.h>
#include <aiScene.h>
#include "Transform.h"

struct SceneDesc;

namespace AssImp
{
    /// Contains mesh data information about a file that can be imported with OpenAssetImport.
    /** GetMeshData() uses this to fill out information which can be used to generate entities
        and components.
    */
    struct MeshData
    {
        QString file_;          ///< file path, same for each individual mesh in the file.
        QString name_;          ///< name of an individual mesh inside the file. May be empty if file contains only one mesh.
        Transform transform_;   ///< transforms in global space for an individual mesh.
    };

    /// Open Asset Import, a wrapper for Open Asset Import library that is used for loading model formats other than Ogre .mesh.
    /** Imports an Ogre mesh from various different model formats. The Ogre mesh is created to Ogre::MeshManager. This class
        doesn't create any entities or components, the caller is responsible for this.
    */
    class OpenAssetImport
    {
    public:
        OpenAssetImport();
        ~OpenAssetImport();

        /// Helper function for stripping mesh name from asset id
        /** If asset is contained in a file that contains other assets as well,
            this function can be used to separate the mesh name so you can
            get a file name and a mesh name
            \param id asset id
            \param outfile asset filename
            \param outMeshname mesh name
        */
        static void StripMeshnameFromAssetId(const QString& id, QString &outfile, QString &outMeshname);
        
        /// Returns true if filename has an extension that implies supported file format
        /**
            \param filename full path or only filename to test
        */
        bool IsSupportedExtension(const QString& filename);

        /// Imports mesh data from a file.
        /** Import mesh names and transformations contained in the model file. This
            information can be used to create entities, components ands asset refs.
            Use Import() to create the actual Ogre mesh data.

            \note Does not handle scene hierarchy, all transformations are converted
                  to world space.

            \param file path to file where to import meshes from
            \param outMeshData Out string vector of mesh names
        */
        void GetMeshData(const QString& file, std::vector<MeshData> &outMeshData) const;

        /// Generates Ogre meshes from memory buffer.
        /** The meshes are generated directly to Ogre::MeshManager and names of the meshes are
            returned. Use GetMeshData() to get hierarchy and transformation data from a model file.

            \param data memory buffer where to import meshes from
            \param length memory buffer length
            \param name file format hint for the importer, looks for extension within the name
            \param hint file format hint for the importer, in practise file extension with the dot included
            \param node name of the node to import
            \param outMeshNames Out string vector of generated Ogre mesh names
        */
        void Import(const void *data, size_t length, const QString &name, const char* hint, const QString &nodeName, std::vector<std::string> &outMeshNames);

        /// Inspects file and returns a scene description structure of the contents of the file.
        /** \param filename File name.
        */
        SceneDesc GetSceneDescription(const QString &filename) const;

    private:
        class AssImpLogStream : public Assimp::LogStream
        {
        public:
            AssImpLogStream() {}
            virtual ~AssImpLogStream() {}

            void write(const char* message);
        };

        void GetNodeData(const aiScene *scene, const aiNode *node, const QString& file,
            const aiMatrix4x4 &parentTransform, std::vector<MeshData> &outMeshNames) const;
        
        void ImportNode(const struct aiScene *scene, const struct aiNode *node, const QString& file, const QString &nodeName, 
            std::vector<std::string> &outMeshNames);

        boost::shared_ptr<Assimp::Importer> importer_;
        AssImpLogStream *logstream_;

        const unsigned int loglevels_;     /// Log levels to capture during import
        const unsigned int default_flags_; /// Default import postprocess flags
    };
}
