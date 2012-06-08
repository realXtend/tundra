#include "OpenAssetImport.h"
#include "assimp/DefaultLogger.h"
#include "OgreDataStream.h"
#include "OgreImage.h"
#include "OgreTexture.h"
#include "OgreTextureManager.h"
#include "OgreMaterial.h"
#include "OgreMaterialManager.h"
#include "OgreLog.h"
#include "OgreLogManager.h"
#include "OgreHardwareBuffer.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreMatrix4.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreMeshManager.h"
#include "OgreSceneManager.h"
#include <OgreStringConverter.h>
#include <OgreSkeletonManager.h>
#include "OgreMeshSerializer.h"
#include "OgreSkeletonSerializer.h"
#include "OgreAnimation.h"
#include "OgreAnimationTrack.h"
#include "OgreKeyFrame.h"
#include "OgreVector3.h"
#include <QString>
#include <QStringList>
#include <boost/tuple/tuple.hpp>
#include <boost/filesystem.hpp>

//#define SKELETON_ENABLED

static int meshNum = 0;

inline Ogre::String toString(const aiColor4D& colour)
{
    return	Ogre::StringConverter::toString(colour.r) + " " +
            Ogre::StringConverter::toString(colour.g) + " " +
            Ogre::StringConverter::toString(colour.b) + " " +
            Ogre::StringConverter::toString(colour.a);
}

int OpenAssetImport::msBoneCount = 0;

void OpenAssetImport::FixLocalReference(QString &matRef, QString addRef)
{
    Ogre::LogManager::getSingleton().logMessage(addRef.toStdString());
    addRef = addRef.remove(addRef.lastIndexOf('/'), addRef.length());
    addRef = addRef.remove(addRef.lastIndexOf('/'), addRef.length());
    addRef = addRef.remove(0, addRef.lastIndexOf('/')+1);
    addRef.insert(0, "local://");
    addRef.insert(addRef.length(), "/images/");

    size_t indx = matRef.indexOf("texture ", 0);
    matRef.replace(indx+8, 0, addRef + '/');
}

void OpenAssetImport::FixHttpReference(QString &matRef, QString addRef)
{
    addRef.replace(0, 7, "http://");
    for (uint i = 0; i < addRef.length(); ++i)
        if (addRef[i].toAscii() == '_') addRef[i] = '/';

    size_t tmp = addRef.lastIndexOf('/')+1;
    addRef.remove(tmp, addRef.length() - tmp);

    addRef = addRef.remove(addRef.lastIndexOf('/'), addRef.length());
    addRef = addRef.remove(addRef.lastIndexOf('/'), addRef.length());
    addRef.insert(addRef.length(), "/images/");
    size_t indx = matRef.indexOf("texture ", 0);
    matRef.replace(indx+8, 0, addRef);
}

std::vector<QString> GetMeshNodes(const aiScene* mScene, const aiNode* pNode)
{
    std::vector<QString> nodeVector;
    for ( int a=0; a<pNode->mChildren[0]->mNumChildren; ++a )
    {
        nodeVector.push_back(pNode->mChildren[0]->mChildren[a]->mName.data);
        Ogre::LogManager::getSingleton().logMessage("Noud " + Ogre::String(pNode->mChildren[0]->mChildren[a]->mName.data) + " found.");
    }
    return nodeVector;
}


double degreeToRadian(double degree)
{
    double radian = 0;
    radian = degree * (Ogre::Math::PI/180);
    return radian;
}

void OpenAssetImport::linearScaleMesh(Ogre::MeshPtr mesh, int targetSize)
{
    Ogre::AxisAlignedBox mAAB = mMesh->getBounds();
    Ogre::Vector3 meshSize = mAAB.getSize();
    Ogre::Vector3 origSize = meshSize;
    while (meshSize.x >= targetSize || meshSize.y >= targetSize || meshSize.z >= targetSize)
        meshSize /= 2;

    Ogre::Real minCoefficient = meshSize.x / origSize.x;

    // Iterate thru submeshes
    Ogre::Mesh::SubMeshIterator smit = mesh->getSubMeshIterator();
    while( smit.hasMoreElements() )
    {
        Ogre::SubMesh* submesh = smit.getNext();
        if(submesh)
        {
            Ogre::VertexData* vertex_data = submesh->vertexData;
            if(vertex_data)
            {
                const Ogre::VertexElement* posElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
                Ogre::HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());
                unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
                Ogre::Real* points;

                for(size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
                {
                    posElem->baseVertexPointerToElement(vertex, (void **)&points);
                    points[0] *= minCoefficient;
                    points[1] *= minCoefficient;
                    points[2] *= minCoefficient;
                }

                vbuf->unlock();
            }
        }
    }

    Ogre::Vector3 scale(minCoefficient, minCoefficient, minCoefficient);
    mAAB.scale(scale);

    mMesh->_setBounds(mAAB);
}

aiMatrix4x4 updateAnimationFunc(const aiScene * scene, aiNodeAnim * pchannel, Ogre::Real val, float & ticks, aiMatrix4x4 & mat)
{

    // walking animation is found from element 0
    const aiAnimation* mAnim = scene->mAnimations[0];
    double currentTime = val;
    currentTime = fmod( val * 1, mAnim->mDuration);
    ticks = fmod( val * 1, mAnim->mDuration);

    // calculate the transformations for each animation channel

    // get the bone/node which is affected by this animation channel
    const aiNodeAnim* channel = pchannel;

    // ******** Position *****
    aiVector3D presentPosition( 0, 0, 0);
    if( channel->mNumPositionKeys > 0)
    {
        // Look for present frame number. Search from last position if time is after the last time, else from beginning
        // Should be much quicker than always looking from start for the average use case.
        unsigned int frame = 0;
        while( frame < channel->mNumPositionKeys - 1)
        {
            if( currentTime < channel->mPositionKeys[frame+1].mTime)
                break;
            frame++;
        }

        // interpolate between this frame's value and next frame's value
        unsigned int nextFrame = (frame + 1) % channel->mNumPositionKeys;
        const aiVectorKey& key = channel->mPositionKeys[frame];
        const aiVectorKey& nextKey = channel->mPositionKeys[nextFrame];
        double diffTime = nextKey.mTime - key.mTime;
        if( diffTime < 0.0)
            diffTime += mAnim->mDuration;
        if( diffTime > 0)
        {
            float factor = float( (currentTime - key.mTime) / diffTime);
            presentPosition = key.mValue + (nextKey.mValue - key.mValue) * factor;
        } else
        {
            presentPosition = key.mValue;
        }
    }

    // ******** Rotation *********
    aiQuaternion presentRotation( 1, 0, 0, 0);
    if( channel->mNumRotationKeys > 0)
    {
        unsigned int frame = 0;
        while( frame < channel->mNumRotationKeys - 1)
        {
            if( currentTime < channel->mRotationKeys[frame+1].mTime)
                break;
            frame++;
        }

        // interpolate between this frame's value and next frame's value
        unsigned int nextFrame = (frame + 1) % channel->mNumRotationKeys;
        const aiQuatKey& key = channel->mRotationKeys[frame];
        const aiQuatKey& nextKey = channel->mRotationKeys[nextFrame];
        double diffTime = nextKey.mTime - key.mTime;
        if( diffTime < 0.0)
            diffTime += mAnim->mDuration;
        if( diffTime > 0)
        {
            float factor = float( (currentTime - key.mTime) / diffTime);
            aiQuaternion::Interpolate( presentRotation, key.mValue, nextKey.mValue, factor);
        } else
        {
            presentRotation = key.mValue;
        }
    }

    // ******** Scaling **********
    aiVector3D presentScaling( 1, 1, 1);
    if( channel->mNumScalingKeys > 0)
    {
        unsigned int frame = 0;
        while( frame < channel->mNumScalingKeys - 1)
        {
            if( currentTime < channel->mScalingKeys[frame+1].mTime)
                break;
            frame++;
        }

        presentScaling = channel->mScalingKeys[frame].mValue;
    }

    // build a transformation matrix from it
    mat = aiMatrix4x4( presentRotation.GetMatrix());

    mat.a1 *= presentScaling.x; mat.b1 *= presentScaling.x; mat.c1 *= presentScaling.x;
    mat.a2 *= presentScaling.y; mat.b2 *= presentScaling.y; mat.c2 *= presentScaling.y;
    mat.a3 *= presentScaling.z; mat.b3 *= presentScaling.z; mat.c3 *= presentScaling.z;
    mat.a4 = presentPosition.x; mat.b4 = presentPosition.y; mat.c4 = presentPosition.z;
}

void getBasePose(const aiScene * sc, const aiNode * nd)
{
    unsigned int i;
    unsigned int n=0, k = 0, t;

    //insert current mesh's bones into boneMatrices
    for (n=0; n < nd->mNumMeshes; ++n)
    {
        const struct aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];

        std::vector<aiMatrix4x4> boneMatrices( mesh->mNumBones);

        // fill boneMatrices with current bone locations
        for( size_t a = 0; a < mesh->mNumBones; ++a)
        {
            aiBone* bone = mesh->mBones[a];

            // find the corresponding node by again looking recursively through the node hierarchy for the same name
            aiNode* node = sc->mRootNode->FindNode(bone->mName);

            // start with the mesh-to-bone matrix
            boneMatrices[a] = bone->mOffsetMatrix;

            // and now append all node transformations down the parent chain until we're back at mesh coordinates again
            const aiNode* tempNode = node;

            while( tempNode)
            {
                // check your matrix multiplication order here!!!
                boneMatrices[a] = tempNode->mTransformation * boneMatrices[a];

                tempNode = tempNode->mParent;
            }
        }

        // all using the results from the previous code snippet
        std::vector<aiVector3D> resultPos( mesh->mNumVertices);
        std::vector<aiVector3D> resultNorm( mesh->mNumVertices);

        // loop through all vertex weights of all bones
        for( size_t a = 0; a < mesh->mNumBones; ++a)
        {
            const aiBone* bone = mesh->mBones[a];
            const aiMatrix4x4& posTrafo = boneMatrices[a];

            // 3x3 matrix, contains the bone matrix without the translation, only with rotation and possibly scaling
            aiMatrix3x3 normTrafo = aiMatrix3x3( posTrafo);
            for( size_t b = 0; b < bone->mNumWeights; ++b)
            {
                const aiVertexWeight& weight = bone->mWeights[b];

                size_t vertexId = weight.mVertexId;
                const aiVector3D& srcPos = mesh->mVertices[vertexId];
                const aiVector3D& srcNorm = mesh->mNormals[vertexId];

                resultPos[vertexId] += (posTrafo * srcPos) * weight.mWeight;
                resultNorm[vertexId] += (normTrafo * srcNorm)* weight.mWeight;
            }
        }

        for (t = 0; t < mesh->mNumFaces; ++t)
        {
            const struct aiFace* face = &mesh->mFaces[t];

            for(i = 0; i < face->mNumIndices; i++)		// go through all vertices in face
            {
                int vertexIndex = face->mIndices[i];	// get group index for current index


                mesh->mNormals[vertexIndex] = resultNorm[vertexIndex];
                mesh->mVertices[vertexIndex] = resultPos[vertexIndex];
            }

        }
    }

    // draw all children
    for (n = 0; n < nd->mNumChildren; n++   )
    {
        getBasePose(sc, nd->mChildren[n]);
    }
}

bool OpenAssetImport::convert(const Ogre::String& filename, bool generateMaterials, QString addr, int index)
{
    Assimp::DefaultLogger::create("asslogger.log",Assimp::Logger::VERBOSE);

    this->addr = addr;
    mAnimationSpeedModifier = 1.0f;
    Assimp::Importer importer;
    index = -1;
    this->generateMaterials = generateMaterials;
    bool searchFromIndex = false;

    /// NOTICE!!!
    // Some converted mesh might show up pretty messed up, it's happening because some formats might
    // contain unnecessary vertex information, lines and points. Uncomment the line below for fixing this issue.
    // by default remove points and lines from the model, since these are usually
    // degenerate structures from bad modelling or bad import/export.  if they
    // are needed it can be turned on with IncludeLinesPoints

    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
    /// END OF NOTICE

    // Limit triangles because for each mesh there's limited index memory (16bit)
    // ...which should be easy to just change to 32 bit but it didn't seem to be the case
    importer.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, 21845);

    //importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS|aiComponent_LIGHTS|aiComponent_TEXTURES|aiComponent_ANIMATIONS);
    //importer.SetPropertyInteger(AI_CONFIG_FAVOUR_SPEED,1);
    //importer.SetPropertyInteger(AI_CONFIG_GLOB_MEASURE_TIME,0);

    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // propably to request more postprocessing than we do in this example.

    if (index != -1)
        searchFromIndex = true;

    std::string filu;
    filu = boost::filesystem::path(addr.toStdString()).stem();

    unsigned int pFlags = 0
                          | aiProcess_SplitLargeMeshes
                          | aiProcess_FindInvalidData
                          | aiProcess_GenSmoothNormals
                          | aiProcess_Triangulate
                          | aiProcess_FlipUVs
                          | aiProcess_JoinIdenticalVertices
                          | aiProcess_OptimizeMeshes
                          | aiProcess_RemoveRedundantMaterials
                          | aiProcess_ImproveCacheLocality
                          | aiProcess_LimitBoneWeights
                          | aiProcess_SortByPType;

#ifndef SKELETON_ENABLED
    pFlags = pFlags | aiProcess_PreTransformVertices;
#endif

    /*if (!searchFromIndex)
        pFlags = pFlags | aiProcess_PreTransformVertices;*/

    scene = importer.ReadFile(filename, pFlags);

    // If the import failed, report it
    if( !scene)
    {
        Ogre::LogManager::getSingleton().logMessage("AssImp importer failed with the following message:");
        Ogre::LogManager::getSingleton().logMessage(importer.GetErrorString() );
        return false;
    }

    if (scene->HasAnimations())
        getBasePose(scene, scene->mRootNode);

    grabNodeNamesFromNode(scene, scene->mRootNode);

#ifdef SKELETON_ENABLED
    grabBoneNamesFromNode(scene, scene->mRootNode);
#endif

    /* Code sketch for loading meshes by index from Collada file
    if (searchFromIndex)
    {
        std::vector<QString> mainNodes = GetMeshNodes(scene, scene->mRootNode);
        if (mainNodes.size() > 1 && mainNodes.size() <= index)
        {
            rootNode = scene->mRootNode->FindNode(QString(mainNodes[index]).toStdString().c_str());
            transform.FromEulerAnglesXYZ(degreeToRadian(0), 0, degreeToRadian(180));
        }
        else
            Ogre::LogManager::getSingleton().logMessage("Index " + Ogre::StringConverter::toString(index) + " not found! Loading whole mesh!");
    }
    else
        transform.FromEulerAnglesXYZ(degreeToRadian(90), 0, degreeToRadian(180));
    */

    aiMatrix4x4 transform;
    transform.FromEulerAnglesXYZ(degreeToRadian(90), 0, degreeToRadian(180));
    scene->mRootNode->mTransformation = transform;

    computeNodesDerivedTransform(scene, scene->mRootNode, transform);

#ifdef SKELETON_ENABLED
    if(mBonesByName.size())
    {
        mSkeleton = Ogre::SkeletonManager::getSingleton().create("conversion", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        msBoneCount = 0;
        createBonesFromNode(scene, scene->mRootNode);
        msBoneCount = 0;
        createBoneHiearchy(scene, scene->mRootNode);

        if(scene->HasAnimations())
        {
            for(int i = 0; i < scene->mNumAnimations; ++i)
            {
                parseAnimation(scene, i, scene->mAnimations[i]);
            }
        }
    }
#endif

    loadDataFromNode(scene, scene->mRootNode, mPath);

    Ogre::LogManager::getSingleton().logMessage("*** Finished loading ass file ***");
    Assimp::DefaultLogger::kill();

#ifdef SKELETON_ENABLED
    if(!mSkeleton.isNull())
    {
        unsigned short numBones = mSkeleton->getNumBones();
        unsigned short i;
        for (i = 0; i < numBones; ++i)
        {
            Ogre::Bone* pBone = mSkeleton->getBone(i);
            assert(pBone);
        }

        Ogre::SkeletonSerializer binSer;

        //binSer.exportSkeleton(mSkeleton.getPointer(), "model.skeleton");
    }

    Ogre::MeshSerializer meshSer;
    for(MeshVector::iterator it = mMeshes.begin(); it != mMeshes.end(); ++it)
    {
        Ogre::MeshPtr mMesh = *it;
        if(mBonesByName.size())
        {
            mMesh->setSkeletonName(filu + ".skeleton");
        }

        Ogre::Mesh::SubMeshIterator smIt = mMesh->getSubMeshIterator();
        while (smIt.hasMoreElements())
        {
            Ogre::SubMesh* sm = smIt.getNext();
            if (!sm->useSharedVertices)
            {
                // AutogreMaterialic
#if OGRE_VERSION_MINOR >= 8 && OGRE_VERSION_MAJOR >= 1
                Ogre::VertexDeclaration* newDcl =
                        sm->vertexData->vertexDeclaration->getAutoOrganisedDeclaration(mMesh->hasSkeleton(), mMesh->hasVertexAnimation(), false);
#else
                Ogre::VertexDeclaration* newDcl =
                        sm->vertexData->vertexDeclaration->getAutoOrganisedDeclaration(mMesh->hasSkeleton(), mMesh->hasVertexAnimation());
#endif
                if (*newDcl != *(sm->vertexData->vertexDeclaration))
                {
                    // Usages don't matter here since we're only exporting
                    Ogre::BufferUsageList bufferUsages;
                    for (size_t u = 0; u <= newDcl->getMaxSource(); ++u)
                        bufferUsages.push_back(Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
                    sm->vertexData->reorganiseBuffers(newDcl, bufferUsages);
                }
            }
        }
        //meshSer.exportMesh(mMesh.getPointer(), "model.mesh");
    }
#endif

    QString fileLocation = addr;

    if(generateMaterials)
    {
        Ogre::MaterialSerializer ms;
        for(MeshVector::iterator it = mMeshes.begin(); it != mMeshes.end(); ++it)
        {
            mMesh = *it;

            // queue up the materials for serialise
            Ogre::MaterialManager *mmptr = Ogre::MaterialManager::getSingletonPtr();
            Ogre::Mesh::SubMeshIterator it = mMesh->getSubMeshIterator();

            while(it.hasMoreElements())
            {
                Ogre::SubMesh* sm = it.getNext();

                Ogre::String matName(sm->getMaterialName());
                Ogre::MaterialPtr materialPtr = mmptr->getByName(matName);


                ms.queueForExport(materialPtr, true);

                QString materialInfo = ms.getQueuedAsString().c_str();

                if (materialInfo.contains("texture "))
                {
                    if (addr.startsWith("http"))
                        FixHttpReference(materialInfo, addr);
                    else
                        if (!scene->HasTextures())
                            FixLocalReference(materialInfo, addr);
                }

                if (fileLocation.startsWith("http"))
                    matList[fileLocation + "#" + sm->getMaterialName().c_str() + ".material"] = materialInfo;
                else
                {
                    QStringList parsedRef = fileLocation.split("/");
                    int length=parsedRef.length();
                    QString output=parsedRef[length-3] + "_" + parsedRef[length-2] + "_" + parsedRef[length-1];

                    matList[output + "#" + sm->getMaterialName().c_str() + ".material"] = materialInfo;
                }

                QString tmp = sm->getMaterialName().c_str();
                matNameList.push_back(tmp + ".material");
            }
        }
    }

    //Scale mesh scale (xyz) below 10 units.
    if (!scene->HasAnimations())
        linearScaleMesh(mMesh, 10);

    mMeshes.clear();
    mMaterialCode = "";
    mBonesByName.clear();
    mBoneNodesByName.clear();
    boneMap.clear();
    mSkeleton = Ogre::SkeletonPtr(NULL);
    mCustomAnimationName = "";
    // etc...

    // Ogre::MaterialManager::getSingleton().
    Ogre::MeshManager::getSingleton().removeUnreferencedResources();
    Ogre::SkeletonManager::getSingleton().removeUnreferencedResources();

    return true;
}

void OpenAssetImport::parseAnimation (const aiScene* mScene, int index, aiAnimation* anim)
{
    // DefBonePose a matrix that represents the local bone transform (can build from Ogre bone components)
    // PoseToKey a matrix representing the keyframe translation
    // What assimp stores aiNodeAnim IS the decomposed form of the transform (DefBonePose * PoseToKey)
    // To get PoseToKey which is what Ogre needs we'ed have to build the transform from components in
    // aiNodeAnim and then DefBonePose.Inverse() * aiNodeAnim(generated transform) will be the right transform

    Ogre::String animName;
    if(mCustomAnimationName != "")
    {
        animName = mCustomAnimationName;
        if(index >= 1)
        {
            animName += Ogre::StringConverter::toString(index);
        }
    }
    else
    {
        animName = Ogre::String(anim->mName.data);
    }
    if(animName.length() < 1)
    {
        animName = "Animation" + Ogre::StringConverter::toString(index);
    }

    Ogre::LogManager::getSingleton().logMessage("Animation name = '" + animName + "'");
    Ogre::LogManager::getSingleton().logMessage("duration = " + Ogre::StringConverter::toString(Ogre::Real(anim->mDuration)));
    Ogre::LogManager::getSingleton().logMessage("tick/sec = " + Ogre::StringConverter::toString(Ogre::Real(anim->mTicksPerSecond)));
    Ogre::LogManager::getSingleton().logMessage("channels = " + Ogre::StringConverter::toString(anim->mNumChannels));

    Ogre::Animation* animation;
    mTicksPerSecond = 1;
    animation = mSkeleton->createAnimation(Ogre::String(animName), Ogre::Real(anim->mDuration/mTicksPerSecond));

    for (int i = 0; i < (int)anim->mNumChannels; i++)
    {
        Ogre::TransformKeyFrame* keyframe;
        aiNodeAnim* node_anim = anim->mChannels[i];
        Ogre::String boneName = Ogre::String(node_anim->mNodeName.data);

        if(mSkeleton->hasBone(boneName))
        {
            Ogre::Bone* bone = mSkeleton->getBone(boneName);

            Ogre::Matrix4 defBonePoseInv;
            defBonePoseInv.makeInverseTransform(bone->getPosition(), bone->getScale(), bone->getOrientation());
            Ogre::NodeAnimationTrack* track = animation->createNodeTrack(i, bone);

            for (int g = 0; g < 300; g++)
            {
                aiVector3D pos;
                aiQuaternion rot;
                aiMatrix4x4 mat;
                float time;
                updateAnimationFunc(scene, node_anim, g, time, mat);
                mat.DecomposeNoScaling(rot, pos);
                keyframe = track->createNodeKeyFrame(Ogre::Real(time));

                Ogre::Vector3 trans(pos.x, pos.y, pos.z);
                Ogre::Quaternion rotat(rot.w, rot.x, rot.y, rot.z);

                Ogre::Vector3 scale(1,1,1); // ignore scale for now
                Ogre::Matrix4 fullTransform;
                fullTransform.makeTransform(trans, scale, rotat);

                Ogre::Matrix4 poseTokey = defBonePoseInv * fullTransform;
                poseTokey.decomposition(trans, scale, rotat);

                keyframe->setTranslate(trans);
                keyframe->setRotation(rotat);
                keyframe->setScale(scale);

            }
        }
    } // if bone exists

    // loop through channels

    mSkeleton->optimiseAllAnimations();
}

void OpenAssetImport::markAllChildNodesAsNeeded(const aiNode *pNode)
{
    flagNodeAsNeeded(pNode->mName.data);
    // Traverse all child nodes of the current node instance
    for ( int childIdx=0; childIdx<pNode->mNumChildren; ++childIdx )
    {
        const aiNode *pChildNode = pNode->mChildren[ childIdx ];
        markAllChildNodesAsNeeded(pChildNode);
    }
}

void OpenAssetImport::grabNodeNamesFromNode(const aiScene* mScene, const aiNode* pNode)
{
    boneNode bNode;
    bNode.node = const_cast<aiNode*>(pNode);
    if(NULL != pNode->mParent)
    {
        bNode.parent = const_cast<aiNode*>(pNode->mParent);
    }
    bNode.isNeeded = false;
    boneMap.insert(std::pair<Ogre::String, boneNode>(Ogre::String(pNode->mName.data), bNode));
    mBoneNodesByName[pNode->mName.data] = pNode;

    Ogre::LogManager::getSingleton().logMessage("Node " + Ogre::String(pNode->mName.data) + " found.");


    // Traverse all child nodes of the current node instance
    for ( int childIdx=0; childIdx<pNode->mNumChildren; ++childIdx )
    {
        const aiNode *pChildNode = pNode->mChildren[ childIdx ];
        grabNodeNamesFromNode(mScene, pChildNode);
    }
}


void OpenAssetImport::computeNodesDerivedTransform(const aiScene* mScene,  const aiNode *pNode, const aiMatrix4x4 accTransform)
{
    if(mNodeDerivedTransformByName.find(pNode->mName.data) == mNodeDerivedTransformByName.end())
    {
        mNodeDerivedTransformByName[pNode->mName.data] = accTransform;
    }
    for ( int childIdx=0; childIdx<pNode->mNumChildren; ++childIdx )
    {
        const aiNode *pChildNode = pNode->mChildren[ childIdx ];
        computeNodesDerivedTransform(mScene, pChildNode, accTransform * pChildNode->mTransformation);
    }
}

void OpenAssetImport::createBonesFromNode(const aiScene* mScene,  const aiNode *pNode)
{
    if(isNodeNeeded(pNode->mName.data))
    {
        Ogre::Bone* bone = mSkeleton->createBone(Ogre::String(pNode->mName.data), msBoneCount);

        aiQuaternion rot;
        aiVector3D pos;

        aiMatrix4x4 aiM = pNode->mTransformation;

        aiM.DecomposeNoScaling(rot, pos);

        if (!aiM.IsIdentity())
        {
            bone->setPosition(pos.x, pos.y, pos.z);
            bone->setOrientation(rot.w, rot.x, rot.y, rot.z);
        }

        Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(msBoneCount) + ") Creating bone '" + Ogre::String(pNode->mName.data) + "'");

        msBoneCount++;
    }
    // Traverse all child nodes of the current node instance
    for ( int childIdx=0; childIdx<pNode->mNumChildren; ++childIdx )
    {
        const aiNode *pChildNode = pNode->mChildren[ childIdx ];
        createBonesFromNode(mScene, pChildNode);
    }
}

void OpenAssetImport::createBoneHiearchy(const aiScene* mScene,  const aiNode *pNode)
{
    if(isNodeNeeded(pNode->mName.data))
    {
        Ogre::Bone* parent = 0;
        Ogre::Bone* child = 0;
        if(pNode->mParent)
        {
            if(mSkeleton->hasBone(pNode->mParent->mName.data))
            {
                parent = mSkeleton->getBone(pNode->mParent->mName.data);
            }
        }
        if(mSkeleton->hasBone(pNode->mName.data))
        {
            child = mSkeleton->getBone(pNode->mName.data);
        }
        if(parent && child)
        {
            parent->addChild(child);
        }
    }
    // Traverse all child nodes of the current node instance
    for ( int childIdx=0; childIdx<pNode->mNumChildren; childIdx++ )
    {
        const aiNode *pChildNode = pNode->mChildren[ childIdx ];
        createBoneHiearchy(mScene, pChildNode);
    }
}

void OpenAssetImport::flagNodeAsNeeded(const char* name)
{
    boneMapType::iterator iter = boneMap.find(Ogre::String(name));
    if( iter != boneMap.end())
    {
        iter->second.isNeeded = true;
    }
}

bool OpenAssetImport::isNodeNeeded(const char* name)
{
    boneMapType::iterator iter = boneMap.find(Ogre::String(name));
    if( iter != boneMap.end())
    {
        return iter->second.isNeeded;
    }
    return false;
}

void OpenAssetImport::grabBoneNamesFromNode(const aiScene* mScene,  const aiNode *pNode)
{
    meshNum++;
    if(pNode->mNumMeshes > 0)
    {
        for ( int idx=0; idx<pNode->mNumMeshes; ++idx )
        {
            aiMesh *pAIMesh = mScene->mMeshes[ pNode->mMeshes[ idx ] ];

            if(pAIMesh->HasBones())
            {
                for ( Ogre::uint32 i=0; i < pAIMesh->mNumBones; ++i )
                {
                    aiBone *pAIBone = pAIMesh->mBones[ i ];
                    if ( NULL != pAIBone )
                    {
                        mBonesByName[pAIBone->mName.data] = pAIBone;

                        Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(i) + ") REAL BONE with name : " + Ogre::String(pAIBone->mName.data));

                        // flag this node and all parents of this node as needed, until we reach the node holding the mesh, or the parent.
                        aiNode* node = mScene->mRootNode->FindNode(pAIBone->mName.data);
                        while(node)
                        {
                            if(node->mName.data == pNode->mName.data)
                            {
                                flagNodeAsNeeded(node->mName.data);
                                break;
                            }
                            if(node->mName.data == pNode->mParent->mName.data)
                            {
                                flagNodeAsNeeded(node->mName.data);
                                break;
                            }

                            // Not a root node, flag this as needed and continue to the parent
                            flagNodeAsNeeded(node->mName.data);
                            node = node->mParent;
                        }

                        // Flag all children of this node as needed
                        node = mScene->mRootNode->FindNode(pAIBone->mName.data);
                        markAllChildNodesAsNeeded(node);

                    } // if we have a valid bone
                } // loop over bones
            } // if this mesh has bones
        } // loop over meshes
    } // if this node has meshes

    // Traverse all child nodes of the current node instance
    for ( int childIdx=0; childIdx<pNode->mNumChildren; childIdx++ )
    {
        const aiNode *pChildNode = pNode->mChildren[ childIdx ];
        grabBoneNamesFromNode(mScene, pChildNode);
    }
}


Ogre::String ReplaceSpaces(const Ogre::String& s)
{
    Ogre::String res(s);
    replace(res.begin(), res.end(), ' ', '_');

    return res;
}

Ogre::MaterialPtr OpenAssetImport::createMaterial(int index, const aiMaterial* mat, const Ogre::String& mDir)
{
    static int dummyMatCount = 0;
    static int texCount = 0;
    // extreme fallback texture -- 2x2 hot pink
    Ogre::uint8 s_RGB[] = {0, 0, 0, 128, 0, 255, 128, 0, 255, 128, 0, 255};

    std::ostringstream matname;
    Ogre::MaterialManager* ogreMaterialMgr =  Ogre::MaterialManager::getSingletonPtr();
    enum aiTextureType type = aiTextureType_DIFFUSE;
    aiString path;
    aiTextureMapping mapping = aiTextureMapping_UV;       // the mapping (should be uv for now)
    unsigned int uvindex = 0;                             // the texture uv index channel
    float blend = 1.0f;                                   // blend
    aiTextureOp op = aiTextureOp_Multiply;                // op
    aiTextureMapMode mapmode[2] =  { aiTextureMapMode_Wrap, aiTextureMapMode_Wrap };    // mapmode
    std::ostringstream texname;

    aiString szPath;

    if(AI_SUCCESS == aiGetMaterialString(mat, AI_MATKEY_TEXTURE_DIFFUSE(0), &szPath))
    {
        Ogre::LogManager::getSingleton().logMessage("Using aiGetMaterialString : Found texture " + Ogre::String(szPath.data) + " for channel " + Ogre::StringConverter::toString(uvindex));
    }
    if(szPath.length < 1 && generateMaterials)
    {
        Ogre::LogManager::getSingleton().logMessage("Didn't find any texture units...");
        szPath = Ogre::String("dummyMat" + Ogre::StringConverter::toString(dummyMatCount)).c_str();
        dummyMatCount++;
    }

    Ogre::String basename;
    Ogre::String outPath;
    Ogre::StringUtil::splitFilename(Ogre::String(szPath.data), basename, outPath);
    Ogre::LogManager::getSingleton().logMessage("Creating " + addr.toStdString());

    if (scene->HasTextures() && szPath.length > 0)
        basename.insert(0, addr.right(addr.length() - (addr.lastIndexOf('/')+1)).toStdString());

    Ogre::ResourceManager::ResourceCreateOrRetrieveResult status = ogreMaterialMgr->createOrRetrieve(ReplaceSpaces(basename), "General", true);
    Ogre::MaterialPtr ogreMaterial = status.first;
    if (!status.second)
        return ogreMaterial;

    // ambient
    aiColor4D clr(1.0f, 1.0f, 1.0f, 1.0);
    //Ambient is usually way too low! FIX ME!
    if (mat->GetTexture(type, 0, &path) != AI_SUCCESS)
        aiGetMaterialColor(mat, AI_MATKEY_COLOR_AMBIENT,  &clr);
    ogreMaterial->setAmbient(clr.r, clr.g, clr.b);

    // diffuse
    clr = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
    if(AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &clr))
    {
        ogreMaterial->setDiffuse(clr.r, clr.g, clr.b, clr.a);
    }

    // specular
    clr = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
    if(AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &clr))
    {
        ogreMaterial->setSpecular(clr.r, clr.g, clr.b, clr.a);
    }

    // emissive
    clr = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
    if(AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_EMISSIVE, &clr))
    {
        ogreMaterial->setSelfIllumination(clr.r, clr.g, clr.b);
    }

    float fShininess;
    if(AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_SHININESS, &fShininess))
    {
        ogreMaterial->setShininess(Ogre::Real(fShininess));
    }

    int two_sided;
    aiGetMaterialInteger(mat, AI_MATKEY_TWOSIDED, &two_sided);

    if (two_sided != 0)
        ogreMaterial->setCullingMode(Ogre::CULL_NONE);

    if (mat->GetTexture(type, 0, &path) == AI_SUCCESS)
    {

        //hack for showing back and front faces when loading material containing texture
        //it's working for 3d warehouse models though the problem is probably in the models and how the faces has been set by the modeler
        //ogreMaterial->setCullingMode(Ogre::CULL_NONE);

        //set texture info into the ogreMaterial
        if (!scene->HasTextures())
            Ogre::TextureUnitState* texUnitState = ogreMaterial->getTechnique(0)->getPass(0)->createTextureUnitState(basename);
        else
        {
            // If data[0] is *, assume texture index is given
            if (path.data[0] == '*')
            {
                Ogre::Image img;

                int texIndex;
                std::string parsedReference = path.data;
                parsedReference.erase(0, 1);
                std::stringstream str(parsedReference);

                // Parse number after * char in path.data
                str >> texIndex;

                // Hint for compressed texture format (e.g. "jpg", "png" etc)
                std::string format = scene->mTextures[texIndex]->achFormatHint;
                parsedReference.append("." + format);

                QString modelFile = addr.mid(addr.lastIndexOf('/') + 1, addr.length() - addr.lastIndexOf('/'));

                modelFile = modelFile.left(modelFile.lastIndexOf('.'));
                parsedReference.insert(0, modelFile.toStdString());

                // Create datastream from scene->mTextures[texIndex] where texIndex points for each compressed texture data
                // mWidth stores texture size in bytes
                Ogre::DataStreamPtr altStrm(OGRE_NEW Ogre::MemoryDataStream((unsigned char*)scene->mTextures[texIndex]->pcData, scene->mTextures[texIndex]->mWidth, false));
                // Load image to Ogre::Image
                img.load(altStrm);
                // Load image to Ogre Resourcemanager
                Ogre::TextureManager::getSingleton().loadImage(parsedReference.c_str(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, img, Ogre::TEX_TYPE_2D, 0);

                // Png format might contain alpha data so allow alpha blending
                if (format == "png")
                {
                    // Alpha blending needs fixing
                    ogreMaterial->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
                    ogreMaterial->setDepthWriteEnabled(false);
                    ogreMaterial->setDepthCheckEnabled(true);
                    //ogreMaterial->setDepthBias(0.01f, 1.0f);
                    ogreMaterial->setDepthFunction(Ogre::CMPF_LESS );
                }

                //Set loaded image as a texture reference. So Tundra knows name should be loaded from ResourceManager
                ogreMaterial->getTechnique(0)->getPass(0)->createTextureUnitState(parsedReference.c_str());
            }

        }

    }
    ogreMaterial->load();
    return ogreMaterial;
}

bool OpenAssetImport::createSubMesh(const Ogre::String& name, int index, const aiNode* pNode, const aiMesh *mesh, const aiMaterial* mat, Ogre::MeshPtr mMesh, Ogre::AxisAlignedBox& mAAB, const Ogre::String& mDir)
{
    // if animated all submeshes must have bone weights
    if(mBonesByName.size() && !mesh->HasBones())
    {
        Ogre::LogManager::getSingleton().logMessage("Skipping Mesh " + Ogre::String(mesh->mName.data) + "with no bone weights");
        return false;
    }

    Ogre::MaterialPtr matptr;

    if(generateMaterials)
        matptr = createMaterial(mesh->mMaterialIndex, mat, mDir);

    // now begin the object definition
    // We create a submesh per material
    Ogre::SubMesh* submesh = mMesh->createSubMesh(name + Ogre::StringConverter::toString(index));

    // We must create the vertex data, indicating how many vertices there will be
    submesh->useSharedVertices = false;
#include "DisableMemoryLeakCheck.h"
    submesh->vertexData = new Ogre::VertexData();
#include "EnableMemoryLeakCheck.h"
    submesh->vertexData->vertexStart = 0;
    submesh->vertexData->vertexCount = mesh->mNumVertices;
    Ogre::VertexData *data = submesh->vertexData;

    // Vertex declarations
    size_t offset = 0;
    Ogre::VertexDeclaration* decl = submesh->vertexData->vertexDeclaration;
    offset += decl->addElement(0,offset,Ogre::VET_FLOAT3,Ogre::VES_POSITION).getSize();
    offset += decl->addElement(0,offset,Ogre::VET_FLOAT3,Ogre::VES_NORMAL).getSize();

    offset = 0;
    for (int tn=0 ; tn<AI_MAX_NUMBER_OF_TEXTURECOORDS ; ++tn)
    {
        if (mesh->mTextureCoords[tn])
        {
            if (mesh->mNumUVComponents[tn] == 3)
            {
                decl->addElement(1, offset, Ogre::VET_FLOAT3, Ogre::VES_TEXTURE_COORDINATES, tn);
                offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
            } else
            {
                decl->addElement(1, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, tn);
                offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);
            }
        }
    }
    if (mesh->HasTangentsAndBitangents())
    {
        decl->addElement(1, offset, Ogre::VET_FLOAT3, Ogre::VES_TANGENT);
        offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        decl->addElement(1, offset, Ogre::VET_FLOAT3, Ogre::VES_BINORMAL);
    }

    // Write vertex data to buffer
    Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
            decl->getVertexSize(0), // This value is the size of a vertex in memory
            data->vertexCount, // The number of vertices you'll put into this buffer
            Ogre::HardwareBuffer::HBU_DYNAMIC // Properties
            );
    Ogre::Real *vbData = static_cast<Ogre::Real*>(vbuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));

    aiMatrix4x4 aiM = mNodeDerivedTransformByName.find(pNode->mName.data)->second;

    offset = 0;
    aiVector3D vect;
    for (unsigned int n=0 ; n<data->vertexCount ; ++n)
    {
        if (mesh->mVertices != NULL)
        {
            vect.x = mesh->mVertices[n].x;
            vect.y = mesh->mVertices[n].y;
            vect.z = mesh->mVertices[n].z;
#ifndef SKELETONS_ENABLED
            vect *= aiM;
#endif

            Ogre::Vector3 position( vect.x, vect.y, vect.z );
            vbData[offset++] = vect.x;
            vbData[offset++] = vect.y;
            vbData[offset++] = vect.z;

            mAAB.merge(position);
        }

        if (mesh->mNormals != NULL)
        {
            vect.x = mesh->mNormals[n].x;
            vect.y = mesh->mNormals[n].y;
            vect.z = mesh->mNormals[n].z;
#ifndef SKELETONS_ENABLED
            vect *= aiM;
#endif
            vbData[offset++] = vect.x;
            vbData[offset++] = vect.y;
            vbData[offset++] = vect.z;
        }
    }

    vbuf->unlock();
    data->vertexBufferBinding->setBinding(0, vbuf);

    if (mesh->HasTextureCoords(0))
    {
        vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
                decl->getVertexSize(1), // This value is the size of a vertex in memory
                data->vertexCount, // The number of vertices you'll put into this buffer
                Ogre::HardwareBuffer::HBU_DYNAMIC // Properties
                );
        vbData = static_cast<Ogre::Real*>(vbuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));

        offset = 0;
        for (unsigned int n=0 ; n<data->vertexCount ; ++n)
        {
            for (int tn=0 ; tn<AI_MAX_NUMBER_OF_TEXTURECOORDS ; ++tn)
            {
                if (mesh->mTextureCoords[tn])
                {
                    if (mesh->mNumUVComponents[tn] == 3)
                    {
                        vbData[offset++] = mesh->mTextureCoords[tn][n].x;
                        vbData[offset++] = mesh->mTextureCoords[tn][n].y;
                        vbData[offset++] = mesh->mTextureCoords[tn][n].z;
                    } else
                    {
                        vbData[offset++] = mesh->mTextureCoords[tn][n].x;
                        vbData[offset++] = mesh->mTextureCoords[tn][n].y;
                    }
                }
            }

            if (mesh->HasTangentsAndBitangents())
            {
                vbData[offset++] = mesh->mTangents[n].x;
                vbData[offset++] = mesh->mTangents[n].y;
                vbData[offset++] = mesh->mTangents[n].z;

                vbData[offset++] = mesh->mBitangents[n].x;
                vbData[offset++] = mesh->mBitangents[n].y;
                vbData[offset++] = mesh->mBitangents[n].z;
            }
        }
        vbuf->unlock();
        data->vertexBufferBinding->setBinding(1, vbuf);
    }

    size_t numIndices = mesh->mNumFaces * 3;            // support only triangles, so 3 indices per face

    Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
            Ogre::HardwareIndexBuffer::IT_16BIT, // You can use several different value types here
            numIndices, // The number of indices you'll put in that buffer
            Ogre::HardwareBuffer::HBU_DYNAMIC // Properties
            );

    Ogre::uint16 *idxData = static_cast<Ogre::uint16*>(ibuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
    offset = 0;
    for (int n=0 ; n<mesh->mNumFaces ; ++n)
    {
        idxData[offset++] = mesh->mFaces[n].mIndices[0];
        idxData[offset++] = mesh->mFaces[n].mIndices[1];
        idxData[offset++] = mesh->mFaces[n].mIndices[2];
    }

    ibuf->unlock();

    submesh->indexData->indexBuffer = ibuf; // The pointer to the index buffer
    submesh->indexData->indexCount = numIndices; // The number of indices we'll use
    submesh->indexData->indexStart = 0;

    // set bone weigths
    if(mesh->HasBones())
    {
        for ( Ogre::uint32 i=0; i < mesh->mNumBones; i++ )
        {
            aiBone *pAIBone = mesh->mBones[ i ];
            if ( NULL != pAIBone )
            {
                Ogre::String bname = pAIBone->mName.data;
                for ( Ogre::uint32 weightIdx = 0; weightIdx < pAIBone->mNumWeights; weightIdx++ )
                {
                    aiVertexWeight aiWeight = pAIBone->mWeights[ weightIdx ];

                    Ogre::VertexBoneAssignment vba;
                    vba.vertexIndex = aiWeight.mVertexId;
                    vba.boneIndex = mSkeleton->getBone(bname)->getHandle();
                    vba.weight= aiWeight.mWeight;

                    submesh->addBoneAssignment(vba);
                }
            }
        }
    } // if mesh has bones

    // Finally we set a material to the submesh
    if (generateMaterials)
        submesh->setMaterialName(matptr->getName());

    return true;
}

void OpenAssetImport::loadDataFromNode(const aiScene* mScene,  const aiNode *pNode, const Ogre::String& mDir)
{
    if(pNode->mNumMeshes > 0)
    {
        Ogre::MeshPtr mesh;
        Ogre::AxisAlignedBox mAAB;

        if(mMeshes.size() == 0)
        {
            mesh = Ogre::MeshManager::getSingleton().createManual("ROOTMesh", "General");
            mMeshes.push_back(mesh);
        }
        else
        {
            mesh = mMeshes[0];
            mAAB = mesh->getBounds();
        }

        for ( int idx=0; idx<pNode->mNumMeshes; ++idx )
        {
            aiMesh *pAIMesh = mScene->mMeshes[ pNode->mMeshes[ idx ] ];
            //if pAIMesh->
            Ogre::LogManager::getSingleton().logMessage("SubMesh " + Ogre::StringConverter::toString(idx) + " for mesh '" + Ogre::String(pNode->mName.data) + "'");


            // Create a material instance for the mesh.

            const aiMaterial *pAIMaterial = mScene->mMaterials[ pAIMesh->mMaterialIndex ];
            createSubMesh(pNode->mName.data, idx, pNode, pAIMesh, pAIMaterial, mesh, mAAB, mDir);
        }

        // We must indicate the bounding box
        mesh->_setBounds(mAAB);
        mesh->_setBoundingSphereRadius((mAAB.getMaximum()- mAAB.getMinimum()).length()/2.0);
    }

    // Traverse all child nodes of the current node instance

    for ( int childIdx=0; childIdx<pNode->mNumChildren; childIdx++ )
    {
        const aiNode *pChildNode = pNode->mChildren[ childIdx ];
        loadDataFromNode(mScene, pChildNode, mDir);
    }
}

