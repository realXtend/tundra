// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingSystem_Renderer_h
#define incl_OgreRenderingSystem_Renderer_h

#include "RenderServiceInterface.h"
#include <boost/shared_ptr.hpp>

namespace Foundation
{
    class Framework;
}

namespace Ogre
{
    class Root;
    class SceneManager;
    class Camera;
    class RenderWindow;
}

namespace OgreRenderer
{
    class OgreRenderingModule;
    
    //! Ogre renderer
    class Renderer : public Foundation::RenderServiceInterface
    {
    public:
        Renderer(OgreRenderingModule* module, Foundation::Framework* framework);
        virtual ~Renderer();

        //! initializes renderer
        bool Initialize();
        
        //! returns Ogre scenemanager
        Ogre::SceneManager* GetSceneManager() const { return scenemanager_; }

        //! returns Ogre root
        Ogre::Root* GetRoot() const { return root_; }

        //! returns active camera
        void *GetCurrentCamera() { return NULL; }

        //! returns current render window
        void *GetCurrentRenderWindow() { return NULL; }

        //! Threadsafe service to framework
        virtual void Raycast()
        {
            // mutex_lock lock(mRenderer);
            // perform raycast (synchronized)

            // OR
            //
            // Queue raycast request, perform raycast in update() after rendering the scene
            // Return handle to the raycast request. The request is updated with the result
            // after raycast has been performed. (see Smoke demo, collision handling)
        }

        void Update()
        {
            // mutex_lock lock(mRenderer);
            // Ogre::RenderOneFrame();

            // if raycast asynchronous, process all pending raycast requests
        }
    private:
        //! sets up Ogre resources based on resources.cfg
        void Renderer::SetupResources();
    
        boost::mutex renderer_;
        
        //! Ogre root object
        Ogre::Root* root_;
        
        //! Ogre scene manager
        Ogre::SceneManager* scenemanager_;
        
        //! rendering module we belong to
        OgreRenderingModule* module_;
        
        //! framework we belong to
        Foundation::Framework* framework_;
    };

    typedef boost::shared_ptr<Renderer> RendererPtr;
}

#endif
