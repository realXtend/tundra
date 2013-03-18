/*
This code is based on OgreAssimp.
For the latest info, see http://code.google.com/p/ogreassimp/
Copyright (c) 2011 Jacob 'jacmoe' Moen
Licensed under the MIT license:
*/

#pragma once

#include "OpenAssetImportApi.h"
#include "CoreTypes.h"
#include "AssetFwd.h"

#include <OgreMesh.h>
#include <OgreMeshSerializer.h>

#include <assimp/vector3.h>
#include <assimp/matrix4x4.h>

#include <map>
#include <QString>
#include <QObject>

struct aiNode;
struct aiBone;
struct aiMesh;
struct aiMaterial;
struct aiScene;
struct aiAnimation;

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

class OPENASSETIMPORT_API OpenAssetImport : public QObject
{
    Q_OBJECT

public:
    explicit OpenAssetImport(AssetAPI *assetApi);
    ~OpenAssetImport();
    /// Converts collada files to ogre meshes, also parses and genarates ogre materials.
    void Convert(const u8 *data_, size_t numBytes, const QString &fileName, const QString &diskSource, Ogre::MeshPtr mesh);

signals:
    /// This signal is emitted when the ogre mesh is created and generated materials are ready.
    void ConversionDone(bool success);

private:
    /// Sets texture unit to material.
    void SetTexture(QString &texFile);
    /// Returns if all textures are loaded.
    bool PendingTextures();
    /// Parses texture path.
    QString GetPathToTexture(const QString &meshFileName, const QString &meshFileDiskSource, QString &texturePath);
    /// Loads texture files from disk or requests them from http asset server.
    void LoadTextureFile(QString &filename);
    /// Creates vertex data to submeshes.
    bool CreateVertexData(const Ogre::String& name, const aiNode* pNode, const aiMesh *mesh, Ogre::SubMesh* submesh, Ogre::AxisAlignedBox& mAAB);
    /// Generates the ogre materials.
    Ogre::MaterialPtr CreateMaterial(Ogre::String& matName, const aiMaterial* mat, const QString &meshFileDiskSource, const QString &meshFileName);
    Ogre::MaterialPtr CreateVertexColorMaterial();
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
    int texCount;
    Ogre::String mMaterialCode;
    Ogre::String mCustomAnimationName;
    Ogre::SkeletonPtr mSkeleton;
    static int msBoneCount;

    Ogre::Real mTicksPerSecond;
    Ogre::Real mAnimationSpeedModifier;

    AssetAPI *assetAPI;
    bool meshCreated;
    ///Map that holds the corresponding texture name and the ogre material pointer whre the texture belongs.
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
