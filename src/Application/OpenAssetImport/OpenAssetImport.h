#ifndef incl_OpenAssetImport_h
#define incl_OpenAssetImport_h

#include "CoreTypes.h"
#include <string>
#include <OgreMesh.h>
#include <OgreMeshSerializer.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <map>
#include <QString>

struct boneNode
{
    aiNode* node;
    aiNode* parent;
    bool isNeeded;
};

class OpenAssetImport
{
public:
	OpenAssetImport();
	~OpenAssetImport();
    bool convert(const u8 *data_, size_t numBytes, const QString &fileName, const QString &diskSource, Ogre::MeshPtr mesh);

private:
    const aiScene *scene;
	QString GetPathToTexture(const QString &meshFileDiskSource, QString &texturePath);
	bool loadTextureFile(QString &filename);
    void linearScaleMesh(Ogre::MeshPtr mesh, int targetSize);
    bool createSubMesh(const Ogre::String& name, int index, const aiNode* pNode, const aiMesh *mesh, const aiMaterial* mat, Ogre::MeshPtr pMesh, Ogre::AxisAlignedBox& mAAB, const QString &meshFileDiskSource);
    Ogre::MaterialPtr createMaterial(int index, const aiMaterial* mat, const QString &meshFileDiskSource);
    Ogre::MaterialPtr createMaterialByScript(int index, const aiMaterial* mat);
    void grabNodeNamesFromNode(const aiScene* mScene,  const aiNode* pNode);
    void grabBoneNamesFromNode(const aiScene* mScene,  const aiNode* pNode);
    void computeNodesDerivedTransform(const aiScene* mScene,  const aiNode *pNode, const aiMatrix4x4 accTransform);
    void createBonesFromNode(const aiScene* mScene,  const aiNode* pNode);
    void createBoneHiearchy(const aiScene* mScene,  const aiNode *pNode);
    void loadDataFromNode(const aiScene* mScene,  const aiNode *pNode, const QString &meshFileDiskSource, Ogre::MeshPtr mesh);
    void markAllChildNodesAsNeeded(const aiNode *pNode);
    void flagNodeAsNeeded(const char* name);
    bool isNodeNeeded(const char* name);
    void parseAnimation (const aiScene* mScene, int index, aiAnimation* anim);
    typedef std::map<Ogre::String, boneNode> boneMapType;
    boneMapType boneMap;
    int mLoaderParams;
    Ogre::String mMaterialCode;
    Ogre::String mCustomAnimationName;

    typedef std::map<Ogre::String, const aiNode*> BoneNodeMap;
    BoneNodeMap mBoneNodesByName;

    typedef std::map<Ogre::String, const aiBone*> BoneMap;
    BoneMap mBonesByName;

    typedef std::map<Ogre::String, aiMatrix4x4> NodeTransformMap;
    NodeTransformMap mNodeDerivedTransformByName;

    typedef std::vector<Ogre::MeshPtr> MeshVector;
    MeshVector mMeshes;

    Ogre::SkeletonPtr mSkeleton;

    static int msBoneCount;

    Ogre::Real mTicksPerSecond;
    Ogre::Real mAnimationSpeedModifier;

};

#endif // __OpenAssetImport_h__

