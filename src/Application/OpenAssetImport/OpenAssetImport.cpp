/*
This code is based on OgreAssimp.
For the latest info, see http://code.google.com/p/ogreassimp/
Copyright (c) 2011 Jacob 'jacmoe' Moen
Licensed under the MIT license:
*/

#include "StableHeaders.h"
#include "OpenAssetImport.h"

#include "CoreDefines.h"
#include "Framework.h"
#include "OgreMeshAsset.h"
#include "OgreMaterialAsset.h"
#include "LoggingFunctions.h"
#include "Math/MathFunc.h"
#include "AssetAPI.h"
#include "IAssetTransfer.h"
#include "IAsset.h"

#include <Ogre.h>

#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//#define SKELETON_ENABLED

int OpenAssetConverter::msBoneCount = 0;

OpenAssetImport::OpenAssetImport() :
    IModule("OpenAssetImport")
{
}

OpenAssetImport::~OpenAssetImport()
{
}

void OpenAssetImport::Initialize()
{
    assetAPI = GetFramework()->Asset();
    connect(assetAPI, SIGNAL(AssetCreated(AssetPtr)), SLOT(OnAssetCreated(AssetPtr)), Qt::UniqueConnection);
}

void OpenAssetImport::OnAssetCreated(AssetPtr asset)
{
    OgreMeshAsset *meshAsset = dynamic_cast<OgreMeshAsset*>(asset.get());
    if (meshAsset && meshAsset->IsAssimpFileType())
    {
        connect(meshAsset, SIGNAL(ExternalConversionRequested(OgreMeshAsset*, const u8*, size_t)), SLOT(OnConversionRequest(OgreMeshAsset*, const u8*, size_t)), Qt::UniqueConnection);
    }
}


void OpenAssetImport::OnConversionRequest(OgreMeshAsset *asset, const u8 *data, size_t len)
{
    OpenAssetConverter *converter = new OpenAssetConverter(GetFramework());
    connect(converter, SIGNAL(ConversionDone(bool)), asset, SLOT(OnAssimpConversionDone(bool)), Qt::UniqueConnection);
    converter->Convert(data, len, asset->Name(), asset->DiskSource(), asset->ogreMesh);
}

OpenAssetConverter::OpenAssetConverter(Framework *fw) :
    assetAPI(fw->Asset()),
    meshCreated(false),
    texCount(0)
{
}

OpenAssetConverter::~OpenAssetConverter()
{
    disconnect();
}

/**************************************************************************
meshFileDiskSource is the absolute path to the mesh file in disk.
texturePath is the path from the mesh file to the textures.
meshFileName is the http address where the mesh is loaded from.

example: meshFileDiskSource = /home/username/.../object/models/collada.dae
and texturePath = ../images/texture.jpg
return value is /home/username/.../object/images/texture.jpg

example: meshFileName = http://dl.dropbox.com/.../models/collada.dae
and texturePath = ../images/texture.jpg
return value is http://dl.dropbox.com/.../images/texture.jpg

example: meshFileName = http://dl.dropbox.com/.../models/collada.dae
and texturePath = http://.../texture.jpg
return value is http://.../texture.jpg
***************************************************************************/

QString OpenAssetConverter::GetPathToTexture(const QString &meshFileName, const QString &meshFileDiskSource, QString &texturePath)
{
    texturePath.replace("\\", "/"); // Normalize all path separators to use forward slashes.
    QString path;

    //When the texture path in .dae file is a http address there is no need to parse it.
    if(texturePath.startsWith("http:") || texturePath.startsWith("https:"))
    {
        path = texturePath;
        return path;
    }
    else
    {
        QStringList parsedMeshPath;

        if(meshFileName.startsWith("http:") || meshFileName.startsWith("https:"))
        {
            parsedMeshPath = meshFileName.split("/");
        }
        else
        {
            parsedMeshPath = meshFileDiskSource.split("/");
        }
		
        int index = texturePath.indexOf("/");
        int dots=0;

        for(int i=0; i<index; i++)
        {
            if(texturePath[i]=='.')
            {
                dots++;
                parsedMeshPath.takeLast();
            }
        }

        if(dots!=0)
        {
            texturePath = texturePath.remove(0, dots);
            path = parsedMeshPath.join("/");
            path.append(texturePath);
        }

        //The texturePath is not always a path from the .dae file to the textures
        //It might be just texture.jpg or /images/texture.jpg, etc.
        //In that case the correct texture file is searched using a top-down search.   
        else
        {
//            int length = parsedMeshPath.length();
            QString base;

            for(int j=0; j<4/*length*/; j++)
            {
                parsedMeshPath.removeLast();
                base = parsedMeshPath.join("/");
                path = AssetAPI::RecursiveFindFile(base,texturePath);
			
                if(!path.isEmpty())
                    break;
            }
        }

        return path;
    }
}

//Loads texture file from disk or from http asset server.
void OpenAssetConverter::LoadTextureFile(QString &filename)
{

    //Loads the texture from http address
    if(filename.startsWith("http:") || filename.startsWith("https:"))
    {
        assetAPI->ForgetAsset(filename, false);
        AssetTransferPtr transPtr = assetAPI->RequestAsset(filename, "Texture", true);
	
        if (!transPtr)
        {
            texMatMap.erase(filename);
            LogError("AssImp importer: Failed to load texture file " +filename.toStdString());
            return;
        }

        connect(transPtr.get(), SIGNAL(Downloaded(IAssetTransfer *)), this, SLOT(OnTextureLoaded(IAssetTransfer*)), Qt::UniqueConnection);
        connect(transPtr.get(), SIGNAL(Failed(IAssetTransfer*, QString)),this, SLOT(OnTextureLoadFailed(IAssetTransfer*, QString)), Qt::UniqueConnection);
    }

    //loads the texture from file
    else
    {
        AssetPtr assetPtr = assetAPI->CreateNewAsset("Texture", filename);
        assetPtr->SetDiskSource(filename);
        bool success = assetPtr->LoadFromFile(filename);

        if(success)
            SetTexture(filename);
        else
        {
            texMatMap.erase(filename);
            LogError("AssImp importer: Failed to load texture file " +filename.toStdString());
        }
    }
}

void OpenAssetConverter::SetTexture(QString &texFile)
{
    Ogre::MaterialPtr ogreMat = texMatMap.find(texFile)->second;
    ogreMat->getTechnique(0)->getPass(0)->createTextureUnitState(AssetAPI::SanitateAssetRef(texFile.toStdString()));
    ogreMat->load();

    texMatMap.erase(texFile);

    if(meshCreated && PendingTextures())
        emit ConversionDone(true);
}

void OpenAssetConverter::OnTextureLoaded(IAssetTransfer* assetTransfer)
{
    QString texFile = assetTransfer->Asset()->Name();
    SetTexture(texFile);
}

void OpenAssetConverter::OnTextureLoadFailed(IAssetTransfer* assetTransfer, QString reason)
{
    QString texFile = assetTransfer->Asset()->Name();
    LogError("AssImp importer::createMaterial: Failed to load texture file " +texFile.toStdString()+ " reason: " + reason.toStdString());
    texMatMap.erase(texFile);

    if(meshCreated && PendingTextures())
        emit ConversionDone(true); //mesh created succesfully without textures
}

bool OpenAssetConverter::PendingTextures()
{
    return texMatMap.empty();
}

aiMatrix4x4 UpdateAnimationFunc(const aiScene * scene, aiNodeAnim * pchannel, Ogre::Real val, float & ticks, aiMatrix4x4 & mat)
{

    // walking animation is found from element 0
    const aiAnimation* mAnim = scene->mAnimations[0];
    double currentTime = val;
    currentTime = fmod( (float)val * 1.f, (float)mAnim->mDuration);
    ticks = fmod( (float)val * 1.f, (float)mAnim->mDuration);

    // calculate the transformations for each animation channel

    // get the bone/node which is affected by this animation channel
    const aiNodeAnim* channel = pchannel;

    // ******** Position *****
    aiVector3D presentPosition( 0, 0, 0);
    if(channel->mNumPositionKeys > 0)
    {
        // Look for present frame number. Search from last position if time is after the last time, else from beginning
        // Should be much quicker than always looking from start for the average use case.
        unsigned int frame = 0;
        while(frame < channel->mNumPositionKeys - 1)
        {
            if(currentTime < channel->mPositionKeys[frame+1].mTime)
                break;
            frame++;
        }

        // interpolate between this frame's value and next frame's value
        unsigned int nextFrame = (frame + 1) % channel->mNumPositionKeys;
        const aiVectorKey& key = channel->mPositionKeys[frame];
        const aiVectorKey& nextKey = channel->mPositionKeys[nextFrame];
        double diffTime = nextKey.mTime - key.mTime;
        if(diffTime < 0.0)
            diffTime += mAnim->mDuration;
        if(diffTime > 0)
        {
            float factor = float( (currentTime - key.mTime) / diffTime);
            presentPosition = key.mValue + (nextKey.mValue - key.mValue) * factor;
        }
        else
        {
            presentPosition = key.mValue;
        }
    }

    // ******** Rotation *********
    aiQuaternion presentRotation( 1, 0, 0, 0);
    if(channel->mNumRotationKeys > 0)
    {
        unsigned int frame = 0;
        while(frame < channel->mNumRotationKeys - 1)
        {
            if(currentTime < channel->mRotationKeys[frame+1].mTime)
                break;
            frame++;
        }

        // interpolate between this frame's value and next frame's value
        unsigned int nextFrame = (frame + 1) % channel->mNumRotationKeys;
        const aiQuatKey& key = channel->mRotationKeys[frame];
        const aiQuatKey& nextKey = channel->mRotationKeys[nextFrame];
        double diffTime = nextKey.mTime - key.mTime;
        if(diffTime < 0.0)
            diffTime += mAnim->mDuration;
        if(diffTime > 0)
        {
            float factor = float( (currentTime - key.mTime) / diffTime);
            aiQuaternion::Interpolate( presentRotation, key.mValue, nextKey.mValue, factor);
        } 
        else
        {
            presentRotation = key.mValue;
        }
    }

    // ******** Scaling **********
    aiVector3D presentScaling( 1, 1, 1);
    if(channel->mNumScalingKeys > 0)
    {
        unsigned int frame = 0;
        while(frame < channel->mNumScalingKeys - 1)
        {
            if(currentTime < channel->mScalingKeys[frame+1].mTime)
                break;
            frame++;
        }

        presentScaling = channel->mScalingKeys[frame].mValue;
    }

    // build a transformation matrix from it
    mat = aiMatrix4x4(presentRotation.GetMatrix());

    mat.a1 *= presentScaling.x; mat.b1 *= presentScaling.x; mat.c1 *= presentScaling.x;
    mat.a2 *= presentScaling.y; mat.b2 *= presentScaling.y; mat.c2 *= presentScaling.y;
    mat.a3 *= presentScaling.z; mat.b3 *= presentScaling.z; mat.c3 *= presentScaling.z;
    mat.a4 = presentPosition.x; mat.b4 = presentPosition.y; mat.c4 = presentPosition.z;

    return mat;
}

void GetBasePose(const aiScene * sc, const aiNode * nd)
{
    unsigned int i;
    unsigned int n=0, /*k = 0,*/ t;

    //insert current mesh's bones into boneMatrices
    for(n=0; n < nd->mNumMeshes; ++n)
    {
        const struct aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];

        std::vector<aiMatrix4x4> boneMatrices(mesh->mNumBones);

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

            while(tempNode)
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
        for(size_t a = 0; a < mesh->mNumBones; ++a)
        {
            const aiBone* bone = mesh->mBones[a];
            const aiMatrix4x4& posTrafo = boneMatrices[a];

            // 3x3 matrix, contains the bone matrix without the translation, only with rotation and possibly scaling
            aiMatrix3x3 normTrafo = aiMatrix3x3( posTrafo);
            for(size_t b = 0; b < bone->mNumWeights; ++b)
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
    for (n = 0; n < nd->mNumChildren; n++)
    {
        GetBasePose(sc, nd->mChildren[n]);
    }
}

void OpenAssetConverter::Convert(const u8 *data_, size_t numBytes, const QString &fileName, const QString &diskSource, Ogre::MeshPtr mesh)
{
    meshCreated = false;
    LogInfo("AssImp importer: Converting file:" +fileName.toStdString());
    Assimp::DefaultLogger::create("asslogger.log",Assimp::Logger::VERBOSE);
    mAnimationSpeedModifier = 1.0f;
    Assimp::Importer importer;
//    bool searchFromIndex = false;

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

    //assimp importer looks for a loader to support the file extension specified by hint 
    QString hint = fileName.right(fileName.length() - fileName.lastIndexOf('.')-1);
    scene = importer.ReadFileFromMemory(reinterpret_cast<const void*>(data_), numBytes, pFlags, hint.toStdString().c_str());

    if(!scene)
    {
        LogInfo("AssImp importer::convert: Failed to read file:" +fileName.toStdString()+ " from memory: ");
        LogInfo(importer.GetErrorString());
        LogInfo("AssImp importer::convert: Trying to load data from file:" +fileName.toStdString());
		
        // If the importer failed to read the file from memory,
        // try to read the file again.
        scene = importer.ReadFile(diskSource.toStdString(), pFlags);

        if(!scene)
        {
            LogError("AssImp importer::convert: conversion failed, importer unable to load data from file:" +fileName.toStdString());
            emit ConversionDone(false);
            return;
        }
    }

    if (scene->HasAnimations())
        GetBasePose(scene, scene->mRootNode);

    GrabNodeNamesFromNode(scene, scene->mRootNode);

#ifdef SKELETON_ENABLED
    GrabBoneNamesFromNode(scene, scene->mRootNode);
#endif

    aiMatrix4x4 transform;
    transform.FromEulerAnglesXYZ(DegToRad(90), 0, DegToRad(180));
    scene->mRootNode->mTransformation = transform;

    ComputeNodesDerivedTransform(scene, scene->mRootNode, transform);

#ifdef SKELETON_ENABLED
    if(mBonesByName.size())
    {
        mSkeleton = Ogre::SkeletonManager::getSingleton().create("conversion", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        msBoneCount = 0;
        CreateBonesFromNode(scene, scene->mRootNode);
        msBoneCount = 0;
        CreateBoneHiearchy(scene, scene->mRootNode);

        if(scene->HasAnimations())
        {
            for(int i = 0; i < scene->mNumAnimations; ++i)
            {
                ParseAnimation(scene, i, scene->mAnimations[i]);
            }
        }
    }
#endif

    LoadDataFromNode(scene, scene->mRootNode, diskSource, fileName, mesh);

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
            mMesh->setSkeletonName(fileName.toStdString() + ".skeleton");
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
    }

#endif

    mMeshes.clear();
    mMaterialCode = "";
    mBonesByName.clear();
    mBoneNodesByName.clear();
    boneMap.clear();
    mSkeleton = Ogre::SkeletonPtr(NULL);
    mCustomAnimationName = "";
    // etc...
    meshCreated = true;
    Ogre::MeshManager::getSingleton().removeUnreferencedResources();
    Ogre::SkeletonManager::getSingleton().removeUnreferencedResources();
	
    if(meshCreated && PendingTextures())
        emit ConversionDone(true);
}

void OpenAssetConverter::ParseAnimation (const aiScene* mScene, int index, aiAnimation* anim)
{
    UNREFERENCED_PARAM(mScene);

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

    for(int i = 0; i < (int)anim->mNumChannels; i++)
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

            for(int g = 0; g < 300; g++)
            {
                aiVector3D pos;
                aiQuaternion rot;
                aiMatrix4x4 mat;
                float time;
                UpdateAnimationFunc(scene, node_anim, g, time, mat);
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

void OpenAssetConverter::MarkAllChildNodesAsNeeded(const aiNode *pNode)
{
    FlagNodeAsNeeded(pNode->mName.data);
    // Traverse all child nodes of the current node instance
    for (unsigned int childIdx=0; childIdx<pNode->mNumChildren; ++childIdx )
    {
        const aiNode *pChildNode = pNode->mChildren[ childIdx ];
        MarkAllChildNodesAsNeeded(pChildNode);
    }
}

void OpenAssetConverter::GrabNodeNamesFromNode(const aiScene* mScene, const aiNode* pNode)
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
    for (unsigned int childIdx=0; childIdx<pNode->mNumChildren; ++childIdx)
    {
        const aiNode *pChildNode = pNode->mChildren[childIdx];
        GrabNodeNamesFromNode(mScene, pChildNode);
    }
}


void OpenAssetConverter::ComputeNodesDerivedTransform(const aiScene* mScene,  const aiNode *pNode, const aiMatrix4x4 accTransform)
{
    if(mNodeDerivedTransformByName.find(pNode->mName.data) == mNodeDerivedTransformByName.end())
    {
        mNodeDerivedTransformByName[pNode->mName.data] = accTransform;
    }
    for (unsigned int childIdx=0; childIdx<pNode->mNumChildren; ++childIdx)
    {
        const aiNode *pChildNode = pNode->mChildren[ childIdx ];
        ComputeNodesDerivedTransform(mScene, pChildNode, accTransform * pChildNode->mTransformation);
    }
}

void OpenAssetConverter::CreateBonesFromNode(const aiScene* mScene,  const aiNode *pNode)
{
    if(IsNodeNeeded(pNode->mName.data))
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
    for (unsigned int childIdx=0; childIdx<pNode->mNumChildren; ++childIdx)
    {
        const aiNode *pChildNode = pNode->mChildren[childIdx];
        CreateBonesFromNode(mScene, pChildNode);
    }
}

void OpenAssetConverter::CreateBoneHiearchy(const aiScene* mScene,  const aiNode *pNode)
{
    if(IsNodeNeeded(pNode->mName.data))
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
    for (unsigned int childIdx=0; childIdx<pNode->mNumChildren; childIdx++)
    {
        const aiNode *pChildNode = pNode->mChildren[ childIdx ];
        CreateBoneHiearchy(mScene, pChildNode);
    }
}

void OpenAssetConverter::FlagNodeAsNeeded(const char* name)
{
    BoneMapType::iterator iter = boneMap.find(Ogre::String(name));
    if(iter != boneMap.end())
    {
        iter->second.isNeeded = true;
    }
}

bool OpenAssetConverter::IsNodeNeeded(const char* name)
{
    BoneMapType::iterator iter = boneMap.find(Ogre::String(name));
    if(iter != boneMap.end())
    {
        return iter->second.isNeeded;
    }
    return false;
}

void OpenAssetConverter::GrabBoneNamesFromNode(const aiScene* mScene,  const aiNode *pNode)
{

    if(pNode->mNumMeshes > 0)
    {
        for(unsigned int idx=0; idx<pNode->mNumMeshes; ++idx)
        {
            aiMesh *pAIMesh = mScene->mMeshes[pNode->mMeshes[idx]];

            if(pAIMesh->HasBones())
            {
                for(Ogre::uint32 i=0; i < pAIMesh->mNumBones; ++i)
                {
                    aiBone *pAIBone = pAIMesh->mBones[i];
                    if (NULL != pAIBone)
                    {
                        mBonesByName[pAIBone->mName.data] = pAIBone;

                        Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(i) + ") REAL BONE with name : " + Ogre::String(pAIBone->mName.data));

                        // flag this node and all parents of this node as needed, until we reach the node holding the mesh, or the parent.
                        aiNode* node = mScene->mRootNode->FindNode(pAIBone->mName.data);
                        while(node)
                        {
                            if(node->mName.data == pNode->mName.data)
                            {
                                FlagNodeAsNeeded(node->mName.data);
                                break;
                            }
                            if(node->mName.data == pNode->mParent->mName.data)
                            {
                                FlagNodeAsNeeded(node->mName.data);
                                break;
                            }

                            // Not a root node, flag this as needed and continue to the parent
                            FlagNodeAsNeeded(node->mName.data);
                            node = node->mParent;
                        }

                        // Flag all children of this node as needed
                        node = mScene->mRootNode->FindNode(pAIBone->mName.data);
                        MarkAllChildNodesAsNeeded(node);

                    } // if we have a valid bone
                } // loop over bones
            } // if this mesh has bones
        } // loop over meshes
    } // if this node has meshes

    // Traverse all child nodes of the current node instance
    for (unsigned int childIdx=0; childIdx<pNode->mNumChildren; childIdx++)
    {
        const aiNode *pChildNode = pNode->mChildren[childIdx];
        GrabBoneNamesFromNode(mScene, pChildNode);
    }
}

Ogre::MaterialPtr OpenAssetConverter::CreateVertexColorMaterial()
{
    Ogre::MaterialManager* ogreMaterialMgr = Ogre::MaterialManager::getSingletonPtr();
    Ogre::String materialName = "vertexcolor.material";

    if(ogreMaterialMgr->resourceExists(materialName))
    {
        Ogre::MaterialPtr ogreMaterial = ogreMaterialMgr->getByName(materialName);
        if(ogreMaterial->isLoaded())
        {
            return ogreMaterial;
        }
    }

    Ogre::MaterialPtr ogreMaterial = ogreMaterialMgr->create(materialName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);
    ogreMaterial->setCullingMode(Ogre::CULL_NONE);
    ogreMaterial->getTechnique(0)->getPass(0)->setVertexColourTracking(Ogre::TVC_AMBIENT|Ogre::TVC_DIFFUSE|Ogre::TVC_SPECULAR);
    return ogreMaterial;
}

Ogre::MaterialPtr OpenAssetConverter::CreateMaterial(Ogre::String& matName, const aiMaterial* mat, const QString &meshFileDiskSource, const QString &meshFileName)
{
    std::ostringstream matname;
    Ogre::MaterialManager* ogreMaterialMgr =  Ogre::MaterialManager::getSingletonPtr();
    enum aiTextureType Type = aiTextureType_DIFFUSE;
    aiString path;
//    aiTextureMapping mapping = aiTextureMapping_UV;       // the mapping (should be uv for now)
    unsigned int uvindex = 0;                             // the texture uv index channel
//    float blend = 1.0f;                                   // blend
//    aiTextureOp op = aiTextureOp_Multiply;                // op
//    aiTextureMapMode mapmode[2] =  { aiTextureMapMode_Wrap, aiTextureMapMode_Wrap };    // mapmode
    std::ostringstream texname;

    aiString szPath;

    if(AI_SUCCESS == aiGetMaterialString(mat, AI_MATKEY_TEXTURE_DIFFUSE(0), &szPath))
    {
	    Ogre::LogManager::getSingleton().logMessage("File: " + meshFileName.toStdString() + ". Texture " + Ogre::String(szPath.data) + " for channel " + Ogre::StringConverter::toString(uvindex));
        //LogInfo("File: " + meshFileName.toStdString() + ". Texture " + Ogre::String(szPath.data) + " for channel " + Ogre::StringConverter::toString(uvindex));
    }

    Ogre::MaterialPtr ogreMaterial = ogreMaterialMgr->create(matName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);

    // ambient
    aiColor4D clr(1.0f, 1.0f, 1.0f, 1.0f);
    //Ambient is usually way too low! FIX ME!
    if (mat->GetTexture(Type, 0, &path) != AI_SUCCESS)
        aiGetMaterialColor(mat, AI_MATKEY_COLOR_AMBIENT, &clr);

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

    if (mat->GetTexture(Type, 0, &path) == AI_SUCCESS)
    {
        //If the assimp scene contains textures they are loaded into the Ogre resource system
        if (!scene->HasTextures())
        {
            QString tex = QString::fromStdString(szPath.data);
            QString texPath = GetPathToTexture(meshFileName, meshFileDiskSource, tex);
            texMatMap.insert(TexMatPair(texPath, ogreMaterial));
            LoadTextureFile(texPath);
        }
    }
    else
        ogreMaterial->load();

    return ogreMaterial;
}

bool OpenAssetConverter::CreateVertexData(const Ogre::String& name, const aiNode* pNode, const aiMesh *mesh, Ogre::SubMesh* submesh, Ogre::AxisAlignedBox& mAAB)
{
    // if animated all submeshes must have bone weights
    if(mBonesByName.size() && !mesh->HasBones())
    {
        Ogre::LogManager::getSingleton().logMessage("Skipping Mesh " + Ogre::String(mesh->mName.data) + "with no bone weights");
        return false;
    }
    
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
    for(int tn=0 ; tn<AI_MAX_NUMBER_OF_TEXTURECOORDS ; ++tn)
    {
        if(mesh->mTextureCoords[tn])
        {
            if(mesh->mNumUVComponents[tn] == 3)
            {
                decl->addElement(1, offset, Ogre::VET_FLOAT3, Ogre::VES_TEXTURE_COORDINATES, tn);
                offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
            } 
            else
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

    offset = 0;
    for(int vn=0 ; vn<AI_MAX_NUMBER_OF_COLOR_SETS; ++vn)
    {
        if (mesh->mColors[vn])
        {
            decl->addElement(2, offset, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);
            offset += Ogre::VertexElement::getTypeSize(Ogre::VET_COLOUR);

        }
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
                    } 
                    else
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

    if(mesh->HasVertexColors(0))
    {
        Ogre::HardwareVertexBufferSharedPtr vbufColor= Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
                decl->getVertexSize(2), // This value is the size of a vertex in memory
                data->vertexCount, // The number of vertices you'll put into this buffer
                Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, // Properties
                false
                );
        Ogre::RGBA* pVertexColor = static_cast<Ogre::RGBA*>(vbufColor->lock(Ogre::HardwareBuffer::HBL_NORMAL));

        for (unsigned int n=0; n<data->vertexCount; ++n)
        {
            for (int vn=0; vn<AI_MAX_NUMBER_OF_COLOR_SETS; ++vn)
            {
                if (mesh->mColors[vn])
                {
				    Ogre::RenderSystem* rs = Ogre::Root::getSingleton().getRenderSystem();
                    if (rs)
                        rs->convertColourValue(Ogre::ColourValue(mesh->mColors[vn][n].r, mesh->mColors[vn][n].g, mesh->mColors[vn][n].b, mesh->mColors[vn][n].a), &pVertexColor[n]);
                }
            }
        }
        vbufColor->unlock();
        data->vertexBufferBinding->setBinding(2, vbufColor);
        data->closeGapsInBindings();
    }

    size_t numIndices = mesh->mNumFaces * 3;            // support only triangles, so 3 indices per face

    Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
            Ogre::HardwareIndexBuffer::IT_16BIT, // You can use several different value types here
            numIndices, // The number of indices you'll put in that buffer
            Ogre::HardwareBuffer::HBU_DYNAMIC // Properties
            );

    Ogre::uint16 *idxData = static_cast<Ogre::uint16*>(ibuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
    offset = 0;
    for (unsigned int n=0 ; n<mesh->mNumFaces ; ++n)
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
        for(Ogre::uint32 i=0; i < mesh->mNumBones; i++ )
        {
            aiBone *pAIBone = mesh->mBones[ i ];
            if (NULL != pAIBone)
            {
                Ogre::String bname = pAIBone->mName.data;
                for (Ogre::uint32 weightIdx = 0; weightIdx < pAIBone->mNumWeights; weightIdx++)
                {
                    aiVertexWeight aiWeight = pAIBone->mWeights[weightIdx];

                    Ogre::VertexBoneAssignment vba;
                    vba.vertexIndex = aiWeight.mVertexId;
                    vba.boneIndex = mSkeleton->getBone(bname)->getHandle();
                    vba.weight= aiWeight.mWeight;

                    submesh->addBoneAssignment(vba);
                }
            }
        }
    } // if mesh has bones

    return true;
}

void OpenAssetConverter::LoadDataFromNode(const aiScene* mScene,  const aiNode *pNode, const QString &meshFileDiskSource, const QString &meshFileName,Ogre::MeshPtr mesh)
{
    if(pNode->mNumMeshes > 0)
    {
        Ogre::AxisAlignedBox mAAB;

        if(mMeshes.size() == 0)
        {
            mMeshes.push_back(mesh);
        }
        else
        {
            mesh = mMeshes[0];
            mAAB = mesh->getBounds();
        }

        for (unsigned int idx=0; idx<pNode->mNumMeshes; ++idx )
        {
            aiMesh *pAIMesh = mScene->mMeshes[ pNode->mMeshes[ idx ] ];
            //if pAIMesh->
            Ogre::LogManager::getSingleton().logMessage("SubMesh " + Ogre::StringConverter::toString(idx) + " for mesh '" + Ogre::String(pNode->mName.data) + "'");

            const aiMaterial *pAIMaterial = mScene->mMaterials[ pAIMesh->mMaterialIndex ];
            Ogre::MaterialPtr matptr;

            //generates material name
            Ogre::String matName = Ogre::String(meshFileName.toStdString()+"_generatedMat" + Ogre::StringConverter::toString(pAIMesh->mMaterialIndex)+ ".material");
            //checks if the material already exist, it might have been generated before to another submesh.
            matptr = Ogre::MaterialManager::getSingleton().getByName(matName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

            if(matptr.isNull())
            {
                if(pAIMesh->HasVertexColors(0))
                    matptr = CreateVertexColorMaterial();
                else
                   matptr = CreateMaterial(matName, pAIMaterial, meshFileDiskSource, meshFileName);

                //we must create an OgreMaterialAsset through assetAPI and put the just created
                //ogre material pointer to it
                //GenerateTemporaryNonexistingAssetFilename() is used to prevent the "Asset Storage contains ambiguous assets in two different subdirectories!" warning 
                QString matname = assetAPI->GenerateTemporaryNonexistingAssetFilename(QString::fromStdString(matptr->getName()));
                AssetPtr assetPtr = assetAPI->CreateNewAsset("OgreMaterial", matname);
                OgreMaterialAsset *mat = static_cast<OgreMaterialAsset *>(assetPtr.get());
                mat->ogreMaterial = matptr;
            }

            Ogre::SubMesh* submesh = mesh->createSubMesh(pNode->mName.data + Ogre::StringConverter::toString(idx));
            CreateVertexData(Ogre::StringConverter::toString(pNode->mName.data), pNode, pAIMesh, submesh, mAAB);
            submesh->setMaterialName(matptr->getName());
        }

        // We must indicate the bounding box
        mesh->_setBounds(mAAB);
        mesh->_setBoundingSphereRadius((mAAB.getMaximum()- mAAB.getMinimum()).length()/2.0);
    }

    // Traverse all child nodes of the current node instance

    for (unsigned int childIdx=0; childIdx<pNode->mNumChildren; childIdx++)
    {
        const aiNode *pChildNode = pNode->mChildren[ childIdx ];
        LoadDataFromNode(mScene, pChildNode, meshFileDiskSource, meshFileName, mesh);
    }
}

extern "C"
{
DLLEXPORT void TundraPluginMain(Framework *fw)
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
    IModule *module = new OpenAssetImport();
    fw->RegisterModule(module);
}
}
