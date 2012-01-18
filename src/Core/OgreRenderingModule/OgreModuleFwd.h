/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   OgreModuleFwd.h
 *  @brief  Forward declarations for commonly used OgreRenderingModule and Ogre classes.
 */

#pragma once

#include <boost/smart_ptr.hpp>

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
}

typedef boost::shared_ptr<Ogre::Root> OgreRootPtr;

namespace OgreRenderer
{
    class OgreRenderingModule;
    class Renderer;
    typedef boost::shared_ptr<Renderer> RendererPtr;
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;
}

class OgreCompositionHandler;
class GaussianListener;
class OgreWorld;
class OgreMaterialAsset;

class EC_AnimationController;
class EC_Camera;
class EC_Light;
class EC_Material;
class EC_Mesh;
class EC_OgreCompositor;
class EC_Placeable;
class EC_RttTarget;

typedef boost::shared_ptr<OgreWorld> OgreWorldPtr;
typedef boost::weak_ptr<OgreWorld> OgreWorldWeakPtr;
