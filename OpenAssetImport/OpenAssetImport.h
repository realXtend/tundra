// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AssImp_OpenAssetImport_h
#define incl_AssImp_OpenAssetImport_h

#include <assimp.hpp>
#include <LogStream.h>
#include <aiScene.h>
namespace OgreRenderer { class Renderer; }

namespace AssImp
{

    class OpenAssetImport
    {
    public:
		OpenAssetImport();
		~OpenAssetImport();

        bool IsSupportedExtension(const QString& extension);

		void Import(Foundation::Framework *framework, const QString& file);

	private:
        class AssImpLogStream : public Assimp::LogStream
        {
        public:
            AssImpLogStream() {}
            virtual ~AssImpLogStream() {}

            void write(const char* message);
        };

        void ImportScene(Foundation::Framework *framework, const struct aiScene *scene);
        void ImportNode(const boost::shared_ptr<OgreRenderer::Renderer> &renderer, const struct aiScene *scene, const struct aiNode *node);

		boost::shared_ptr<Assimp::Importer> importer_;
        AssImpLogStream *logstream_;

        const unsigned int loglevels_;
	};
}
#endif
