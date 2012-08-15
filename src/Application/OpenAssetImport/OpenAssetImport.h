/*
This code is based on OgreAssimp.
For the latest info, see http://code.google.com/p/ogreassimp/
Copyright (c) 2011 Jacob 'jacmoe' Moen
Licensed under the MIT license:
*/

#ifndef incl_OpenAssetImport_h
#define incl_OpenAssetImport_h

#include "CoreTypes.h"
#include "AssetAPI.h"
#include "IAssetTransfer.h"
#include "IAsset.h"
#include "AssetFwd.h"
#include <string>
#include <OgreMesh.h>
#include <OgreMeshSerializer.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <map>
#include <QString>
#include <QObject>

struct boneNode
{
    aiNode* node;
    aiNode* parent;
    bool isNeeded;
};

typedef std::map<Ogre::String, const aiBone*> BoneMap;
typedef std::map<Ogre::String, boneNode> BoneMapType;
typedef std::map<Ogre::String, const aiNode*> BoneNodeMap;
typedef std::map<Ogre::String, aiMatrix4x4> NodeTransformMap;
typedef std::vector<Ogre::MeshPtr> MeshVector;
typedef std::map<QString, Ogre::MaterialPtr> TextureMaterialPointerMap;
typedef std::pair<QString, Ogre::MaterialPtr> TexMatPair;

class OpenAssetImport : public QObject
{
    Q_OBJECT

public:
    OpenAssetImport(AssetAPI *assetApi);
    ~OpenAssetImport();
    void Convert(const u8 *data_, size_t numBytes, const QString &fileName, const QString &diskSource, Ogre::MeshPtr mesh);

signals:
    void ConversionDone(bool success);

private:
    void SetTexture(QString &texFile);
    bool PendingTextures();
    QString GetPathToTexture(const QString &meshFileName, const QString &meshFileDiskSource, QString &texturePath);
    void LoadTextureFile(QString &filename);
    bool CreateVertexData(const Ogre::String& name, const aiNode* pNode, const aiMesh *mesh, Ogre::SubMesh* submesh, Ogre::AxisAlignedBox& mAAB);
    Ogre::MaterialPtr CreateMaterial(int index, const aiMaterial* mat, const QString &meshFileDiskSource, const QString &meshFileName);
    Ogre::MaterialPtr CreateMaterialByScript(int index, const aiMaterial* mat);
    void GrabNodeNamesFromNode(const aiScene* mScene,  const aiNode* pNode);
    void GrabBoneNamesFromNode(const aiScene* mScene,  const aiNode* pNode);
    void ComputeNodesDerivedTransform(const aiScene* mScene,  const aiNode *pNode, const aiMatrix4x4 accTransform);
    void CreateBonesFromNode(const aiScene* mScene,  const aiNode* pNode);
    void CreateBoneHiearchy(const aiScene* mScene,  const aiNode *pNode);
    void LoadDataFromNode(const aiScene* mScene,  const aiNode *pNode, const QString &meshFileDiskSource, const QString &meshFileName,Ogre::MeshPtr mesh);
    void MarkAllChildNodesAsNeeded(const aiNode *pNode);
    void FlagNodeAsNeeded(const char* name);
    bool IsNodeNeeded(const char* name);
    void ParseAnimation (const aiScene* mScene, int index, aiAnimation* anim);

    const aiScene *scene;
    int mLoaderParams;
    Ogre::String mMaterialCode;
    Ogre::String mCustomAnimationName;
    Ogre::SkeletonPtr mSkeleton;
    static int msBoneCount;

    Ogre::Real mTicksPerSecond;
    Ogre::Real mAnimationSpeedModifier;

    AssetAPI *assetAPI;
    bool meshCreated;
    TextureMaterialPointerMap texMatMap;

    BoneMapType boneMap;
    BoneNodeMap mBoneNodesByName;
    BoneMap mBonesByName;
    NodeTransformMap mNodeDerivedTransformByName;
    MeshVector mMeshes;



private slots:
    void OnTextureLoaded(IAssetTransfer* assetTransfer);
    void OnTextureLoadFailed(IAssetTransfer* assetTransfer, QString reason);

};

#endif // __OpenAssetImport_h__

