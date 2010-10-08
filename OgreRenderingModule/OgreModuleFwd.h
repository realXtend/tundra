/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   OgreModuleFwd.h
 *  @brief  Forward declarations for commonly used OgreRenderingModule and Ogre classes.
 */

#ifndef incl_OgreRendereringModule_OgreModuleFwd_h
#define incl_OgreRendereringModule_OgreModuleFwd_h

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
}

namespace OgreRenderer
{
    class Renderer;
    typedef boost::shared_ptr<Renderer> RendererPtr;
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;

    class OgreRenderingModule;
    class LogListener;
    class ResourceHandler;
    class RenderableListener;
    class QOgreUIView;
    class QOgreWorldView;
    class CAVEManager;
    class StereoController;
    class CompositionHandler;
    class GaussianListener;

    typedef boost::shared_ptr<Ogre::Root> OgreRootPtr;
    typedef boost::shared_ptr<LogListener> OgreLogListenerPtr;
    typedef boost::shared_ptr<ResourceHandler> ResourceHandlerPtr;
    typedef boost::shared_ptr<RenderableListener> RenderableListenerPtr;
}

class EC_Placeable;
class EC_Mesh;

#endif
