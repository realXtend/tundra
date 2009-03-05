// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingSystem_Renderer_h
#define incl_OgreRenderingSystem_Renderer_h

#include "RenderServiceInterface.h"
#include <boost/shared_ptr.hpp>

namespace OgreRenderer
{
    //! mockup Ogre renderer
    class Renderer : public Foundation::RenderServiceInterface
    {
    public:
        Renderer() {};
        virtual ~Renderer() {};

        //! returns Ogre scenemanager
        void *getSceneManager() { return NULL; }

        //! returna Ogre root
        void *getRoot() { return NULL; }

        //! returns active camera
        void *getCurrentCamera() { return NULL; }

        //! returns current render window
        void *getCurrentRenderWindow() { return NULL; }

        //! Threadsafe service to framework
        virtual void raycast()
        {
            // mutex_lock lock(mRenderer);
            // perform raycast (synchronized)

            // OR
            //
            // Queue raycast request, perform raycast in update() after rendering the scene
            // Return handle to the raycast request. The request is updated with the result
            // after raycast has been performed. (see Smoke demo, collision handling)
        }

        void update()
        {
            // mutex_lock lock(mRenderer);
            // Ogre::RenderOneFrame();

            // if raycast asynchronous, process all pending raycast requests
        }
    private:
        boost::mutex mRenderer;
    };

    typedef boost::shared_ptr<Renderer> RendererPtr;
}

#endif
