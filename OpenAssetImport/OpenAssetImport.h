// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AssImp_OpenAssetImport_h
#define incl_AssImp_OpenAssetImport_h

#include <assimp.hpp>
#include <LogStream.h>
#include <aiScene.h>
namespace OgreRenderer { class Renderer; }

namespace AssImp
{
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

        //! Generates Ogre meshes from file
        /*!
            \param file path to file where to import meshes from
            \param outMeshNames Out string vector of generated Ogre mesh names
        */
		void Import(const QString& file, std::vector<std::string> &outMeshNames);
        //! Generates Ogre meshes from memory buffer
        /*!
            \param data memory buffer where to import meshes from
            \param length memory buffer length
            \param name file format hint for the importer, looks for extension within the name
            \param outMeshNames Out string vector of generated Ogre mesh names
        */
        void Import(const void *data, size_t length, const QString &name, std::vector<std::string> &outMeshNames);

	private:
        class AssImpLogStream : public Assimp::LogStream
        {
        public:
            AssImpLogStream() {}
            virtual ~AssImpLogStream() {}

            void write(const char* message);
        };

        void ImportScene(const struct aiScene *scene, const QString& file, std::vector<std::string> &outMeshNames);
        void ImportNode(const struct aiScene *scene, const struct aiNode *node, const QString& file, int nodeIdx, 
            std::vector<std::string> &outMeshNames);

		boost::shared_ptr<Assimp::Importer> importer_;
        AssImpLogStream *logstream_;

        const unsigned int loglevels_;     //! Log levels to capture during import
        const unsigned int default_flags_; //! Default import postprocess flags
	};
}
#endif
