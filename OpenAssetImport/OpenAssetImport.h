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
        The Ogre mesh is created to Ogre::MeshManager, this class
        doesn't create any entities or components, the caller is
        responsible for this.
    */
    class OpenAssetImport
    {
    public:
		OpenAssetImport();
		~OpenAssetImport();

        bool IsSupportedExtension(const QString& extension);

		void Import(Foundation::Framework *framework, const QString& file, std::vector<std::string> &outMeshNames);

	private:
        class AssImpLogStream : public Assimp::LogStream
        {
        public:
            AssImpLogStream() {}
            virtual ~AssImpLogStream() {}

            void write(const char* message);
        };

        void ImportScene(Foundation::Framework *framework, const struct aiScene *scene, const QString& file, std::vector<std::string> &outMeshNames);
        void ImportNode(const boost::shared_ptr<OgreRenderer::Renderer> &renderer, const struct aiScene *scene, 
            const struct aiNode *node, const QString& file, std::vector<std::string> &outMeshNames);

		boost::shared_ptr<Assimp::Importer> importer_;
        AssImpLogStream *logstream_;

        const unsigned int loglevels_;
	};
}
#endif
