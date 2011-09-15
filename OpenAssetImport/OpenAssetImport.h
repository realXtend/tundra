#ifndef incl_OpenAssetImport_h
#define incl_OpenAssetImport_h

#include <string>
#include <OgreMesh.h>
#include <OgreMeshSerializer.h>

#include <assimp/assimp.hpp>
#include <assimp/aiScene.h>
#include <assimp/aiPostProcess.h>
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
    bool convert(const Ogre::String& filename, bool generateMaterials, QString addr = "", int index = -1);

    Ogre::Mesh * GetMesh() { return mMesh.get(); }
    std::map<QString, QString> matList;
    std::vector<QString> matNameList;
    const Ogre::String& getBasename(){ return mBasename; }
    void FixLocalReference(QString &matRef, QString addRef);
    void FixHttpReference(QString &matRef, QString addRef);
    bool IsSupportedExtension(QString extension);

private:
    const aiScene *scene;
    QString addr;
    Ogre::MeshPtr mMesh;
    bool generateMaterials;
    void linearScaleMesh(Ogre::MeshPtr mesh, int targetSize);
    bool createSubMesh(const Ogre::String& name, int index, const aiNode* pNode, const aiMesh *mesh, const aiMaterial* mat, Ogre::MeshPtr pMesh, Ogre::AxisAlignedBox& mAAB, const Ogre::String& mDir);
    Ogre::MaterialPtr createMaterial(int index, const aiMaterial* mat, const Ogre::String& mDir);
    Ogre::MaterialPtr createMaterialByScript(int index, const aiMaterial* mat);
    void grabNodeNamesFromNode(const aiScene* mScene,  const aiNode* pNode);
    void grabBoneNamesFromNode(const aiScene* mScene,  const aiNode* pNode);
    void computeNodesDerivedTransform(const aiScene* mScene,  const aiNode *pNode, const aiMatrix4x4 accTransform);
    void createBonesFromNode(const aiScene* mScene,  const aiNode* pNode);
    void createBoneHiearchy(const aiScene* mScene,  const aiNode *pNode);
    void loadDataFromNode(const aiScene* mScene,  const aiNode *pNode, const Ogre::String& mDir);
    void markAllChildNodesAsNeeded(const aiNode *pNode);
    void flagNodeAsNeeded(const char* name);
    bool isNodeNeeded(const char* name);
    void parseAnimation (const aiScene* mScene, int index, aiAnimation* anim);
    typedef std::map<Ogre::String, boneNode> boneMapType;
    boneMapType boneMap;
    int mLoaderParams;
    Ogre::String mBasename;
    Ogre::String mPath;
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
