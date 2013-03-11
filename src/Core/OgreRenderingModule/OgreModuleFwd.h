/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   OgreModuleFwd.h
    @brief  Forward declarations for commonly used OgreRenderingModule and Ogre classes. */

#pragma once

#include "CoreTypes.h"

namespace Ogre
{
    class Entity;
    class Mesh;
    class Node;
    class SceneNode;
    class Camera;
    class ManualObject;
    class Light;
    class Root;
    class SceneManager;
    class RenderWindow;
    class RaySceneQuery;
    class Viewport;
    class RenderTexture;
    class DefaultHardwareBufferManager;
    class ParticleSystem;
    class Skeleton;
    class TagPoint;
    class MaterialPtr;
    class BillboardSet;
    class Billboard;
    class Bone;
}

typedef shared_ptr<Ogre::Root> OgreRootPtr;

namespace OgreRenderer
{
    class OgreRenderingModule;
    class Renderer;
    typedef shared_ptr<Renderer> RendererPtr;
    typedef weak_ptr<Renderer> RendererWeakPtr;
}

class OgreCompositionHandler;
class GaussianListener;
class OgreWorld;
class UiPlane;
class RenderWindow;

class TextureAsset;
class OgreMeshAsset;
class OgreMaterialAsset;
class OgreSkeletonAsset;
class OgreParticleAsset;

typedef shared_ptr<TextureAsset> TextureAssetPtr;
typedef shared_ptr<OgreMeshAsset> OgreMeshAssetPtr;
typedef shared_ptr<OgreMaterialAsset> OgreMaterialAssetPtr;
typedef shared_ptr<OgreSkeletonAsset> OgreSkeletonAssetPtr;
typedef shared_ptr<OgreParticleAsset> OgreParticleAssetPtr;

class EC_AnimationController;
class EC_Camera;
class EC_Light;
class EC_Material;
class EC_Mesh;
class EC_OgreCompositor;
class EC_Placeable;
class EC_RttTarget;
class EC_Billboard;
class EC_ParticleSystem;
class EC_Sky;
class EC_EnvironmentLight;
class EC_Fog;

typedef shared_ptr<OgreWorld> OgreWorldPtr;
typedef weak_ptr<OgreWorld> OgreWorldWeakPtr;
