// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_Renderer_h
#define incl_OgreRenderer_Renderer_h

#include "Foundation.h"
#include "RenderServiceInterface.h"
#include "LogListenerInterface.h"
#include "ResourceInterface.h"

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
    /*! Created by OgreRenderingModule. Implements the RenderServiceInterface.
        \ingroup OgreRenderingModuleClient
    */
    class Renderer : public Foundation::RenderServiceInterface
    {
        friend class EventListener;
        
    public:
        //! Constructor
        Renderer(Foundation::Framework* framework);

        //! Destructor
        virtual ~Renderer();

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

        //! Resizes the window
        virtual void Resize(Core::uint width, Core::uint height);

        //! Renders the screen
        virtual void Render();

        //! Returns window handle, or 0 if no render window
        virtual size_t GetWindowHandle() const;

        //! Returns window width, or 0 if no render window
        virtual int GetWindowWidth() const;

        //! Returns window height, or 0 if no render window
        virtual int GetWindowHeight() const;

        //! Subscribe a listener to renderer log. Can be used before renderer is initialized.
        virtual void SubscribeLogListener(const Foundation::LogListenerPtr &listener);
        
        //! Unsubsribe a listener to renderer log. Can be used before renderer is initialized.
        virtual void UnsubscribeLogListener(const Foundation::LogListenerPtr &listener);
        
        
        //! Callback when renderwindow closed
        /*! Sends event and exits the framework main loop
         */
        void OnWindowClosed();

        //! Sets external window parameter, for embedding the Ogre renderwindow. Usually a child window.
        void SetExternalWindowParameter(const std::string& param) { external_window_parameter_ = param; }

        //! set handle for the main window. This is for the top level window.
        void SetMainWindowHandle(Core::uint hndl) { main_window_handle_ = hndl; }

        //! Returns framework
        Foundation::Framework* GetFramework() const { return framework_; }

        //! Returns initialized state
        bool IsInitialized() const { return initialized_; }

        //! Returns Ogre root
        OgreRootPtr GetRoot() const { return root_; }

        //! Returns Ogre scenemanager
        Ogre::SceneManager* GetSceneManager() const { return scenemanager_; }
        
        //! Returns active camera
        Ogre::Camera* GetCurrentCamera() const { return camera_; }

        //! Returns current render window
        Ogre::RenderWindow* GetCurrentRenderWindow() const { return renderwindow_; }

        //! Returns an unique name to create Ogre objects that require a mandatory name
        std::string GetUniqueObjectName();

        //! Returns an Ogre texture resource, null if not found
        /*! Does not automatically make a request to the asset system
         */
        Foundation::ResourcePtr GetTexture(const std::string& id);

        //! Requests a texture to be downloaded & decoded
        /*! A resource event will be sent once each texture quality level is decoded
         */
        void RequestTexture(const std::string& id);

        //! Deletes an Ogre texture resource
        void RemoveTexture(const std::string& id);

        //! Returns an Ogre mesh resource, null if not found
        /*! does not automatically make a request to the asset system
         */
        Foundation::ResourcePtr GetMesh(const std::string& id);

        //! Requests a mesh to be downloaded
        /*! a resource event will be sent once the mesh asset is ready
         */
        void RequestMesh(const std::string& id);

        //! Deletes an Ogre mesh resource
        void RemoveMesh(const std::string& id);

        //! Initializes renderer. Called by OgreRenderingModule
        /*! Creates render window. If render window is to be embedded, call SetExternalWindowParameter() before.
         */
        void Initialize();

        //! Post-initializes renderer. Called by OgreRenderingModule
        /*! Queries event categories it needs
         */
        void PostInitialize();

        //! Performs update. Called by OgreRenderingModule
        /*! Pumps Ogre window events.
         */
        void Update(Core::f64 frametime);

        //! Handles an asset system event. Called by OgreRenderingModule
        bool HandleAssetEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);

        //! Handles a resource event. Called by OgreRenderingModule
        bool HandleResourceEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);

    private:
        //! Loads Ogre plugins in a manner which allows individual plugin loading to fail
        /*! \param plugin_filename path & filename of the Ogre plugins file
         */
        void LoadPlugins(const std::string& plugin_filename);
        
        //! Sets up Ogre resources based on resources.cfg
        void SetupResources();
        
        //! Creates scenemanager & camera
        void SetupScene();
    
        //! Creates or updates a texture, based on a source raw texture resource
        /*! \param source Raw texture 
            \return true if successful
         */
        bool UpdateTexture(Foundation::ResourcePtr source);

        //! Creates or updates a mesh, based on source asset data
        /*! \param source Asset
            \return true if successful
         */
        bool UpdateMesh(Foundation::AssetPtr source);

        boost::mutex renderer_;

        //! Successfully initialized flag
        bool initialized_;
        
        //! Ogre root object
        OgreRootPtr root_;
        
        //! Scene manager
        Ogre::SceneManager* scenemanager_;
        
        //! Default camera
        Ogre::Camera* camera_;
        
        //! Rendering window
        Ogre::RenderWindow* renderwindow_;
        
        //! Framework we belong to
        Foundation::Framework* framework_;
        
        //! Ogre event listener
        EventListenerPtr listener_;

        //! Ogre log listener
        OgreLogListenerPtr log_listener_;
        
        //! Renderer event category
        Core::event_category_id_t renderercategory_id_;
        
        //! Resource event category
        Core::event_category_id_t resourcecategory_id_;

        //! Counter for unique name creation
        Core::uint object_id_;

        //! Ogre texture resources
        Foundation::ResourceMap textures_;

        //! Ogre mesh resources
        Foundation::ResourceMap meshes_;

        //! External window parameter, to be used when embedding the renderwindow
        std::string external_window_parameter_;

        //! handle for the main window
        Core::uint main_window_handle_;
    };
}

#endif
