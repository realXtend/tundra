// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_Renderer_h
#define incl_OgreRenderer_Renderer_h

#include "Foundation.h"
#include "RenderServiceInterface.h"
#include "LogListenerInterface.h"
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
    class EventListener;
    class LogListener;

    typedef boost::shared_ptr<Ogre::Root> OgreRootPtr;
    typedef boost::shared_ptr<EventListener> EventListenerPtr;
    typedef boost::shared_ptr<LogListener> OgreLogListenerPtr;
    
    //! Ogre renderer
    /*! Created by OgreRenderingModule.
     */
    class Renderer : public Foundation::RenderServiceInterface
    {
        friend class EventListener;
        
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

        //! returns an unique name to create Ogre objects that require a mandatory name
        std::string GetUniqueObjectName();

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

        size_t GetWindowHandle() const;

        int GetWindowWidth() const;
        int GetWindowHeight() const;

        //! subscribe a listener to renderer log. Can be used before renderer is initialized.
        void SubscribeLogListener(const Foundation::LogListenerPtr &listener);
        //! unsubsribe a listener to renderer log. Can be used before renderer is initialized.
        void UnsubscribeLogListener(const Foundation::LogListenerPtr &listener);

        //! performs update.
        /*! pumps Ogre window events.
         */
        void Update(Core::f64 frametime);
        
        //! renders the screen
        void Render();

        //! handles Ogre window close event
        void windowClosed(Ogre::RenderWindow* rw);
        
        //! handles Ogre window resize event
        void windowResized(Ogre::RenderWindow* rw);
        
    private:
        //! loads Ogre plugins in a manner which allows individual plugin loading to fail
        /*! \param plugin_filename path & filename of the Ogre plugins file
         */
        void LoadPlugins(const std::string& plugin_filename);
        
        //! sets up Ogre resources based on resources.cfg
        void SetupResources();
        
        //! creates scenemanager & camera
        void SetupScene();
    
        boost::mutex renderer_;

        //! successfully initialized flag
        bool initialized_;
        
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
        
        //! Ogre event listener
        EventListenerPtr listener_;

        //! Ogre log listener
        OgreLogListenerPtr log_listener_;
        
        //! Renderer event category
        Core::event_category_id_t event_category_;
        
        //! counter for unique name creation
        Core::uint object_id_;
    };
}

#endif
