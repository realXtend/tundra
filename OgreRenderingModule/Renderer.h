// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_Renderer_h
#define incl_OgreRenderingModule_Renderer_h

#include "RenderServiceInterface.h"
#include <boost/shared_ptr.hpp>
#include "OgreWindowEventUtilities.h"

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

    typedef boost::shared_ptr<Ogre::Root> OgreRootPtr;
    
    //! Ogre renderer
    class Renderer : public Foundation::RenderServiceInterface, public Ogre::WindowEventListener
    {
    public:
        Renderer(Foundation::Framework* framework);
        virtual ~Renderer();

        //! initializes renderer
        void Initialize();
        
        //! returns Ogre root
        OgreRootPtr GetRoot() const { return root_; }

        //! returns Ogre scenemanager
        Ogre::SceneManager* GetSceneManager() const { return scenemanager_; }
        
        //! returns active camera
        Ogre::Camera* GetCurrentCamera() const { return camera_; }

        //! returns current render window
        Ogre::RenderWindow* GetCurrentRenderWindow() const { return renderwindow_; }

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

        //! renders one frame
        void Update();

        //! handles Ogre window close event
        void windowClosed(Ogre::RenderWindow* rw);
        
        //! handles Ogre window resize event
        void windowResized(Ogre::RenderWindow* rw);
        
    private:
        //! sets up Ogre resources based on resources.cfg
        void SetupResources();
        
        //! creates scenemanager & camera
        void SetupScene();
    
        boost::mutex renderer_;
        
        //! Ogre root object
        OgreRootPtr root_;
        
        //! scene manager
        Ogre::SceneManager* scenemanager_;
        
        //! default camera
        Ogre::Camera* camera_;
        
        //! rendering window
        Ogre::RenderWindow* renderwindow_;
        
        //! framework we belong to
        Foundation::Framework* framework_;
        
        //! successfully initialized flag
        bool initialized_;
    };

    typedef boost::shared_ptr<Renderer> RendererPtr;
}

#endif
