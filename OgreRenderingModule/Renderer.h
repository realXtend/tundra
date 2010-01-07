// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_Renderer_h
#define incl_OgreRenderer_Renderer_h

#include "Foundation.h"
#include "RenderServiceInterface.h"
#include "LogListenerInterface.h"
#include "ResourceInterface.h"
#include "OgreModuleApi.h"

#include "QOgreUIView.h"
#include "QOgreWorldView.h"
#include "CompositionHandler.h"

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
    class RaySceneQuery;
    class Viewport;
    class Mesh;
}

namespace OgreRenderer
{
    class OgreRenderingModule;
    class EventListener;
    class LogListener;
    class ResourceHandler;

    typedef boost::shared_ptr<Ogre::Root> OgreRootPtr;
    typedef boost::shared_ptr<EventListener> EventListenerPtr;
    typedef boost::shared_ptr<LogListener> OgreLogListenerPtr;
    typedef boost::shared_ptr<ResourceHandler> ResourceHandlerPtr;
    
    //! Ogre renderer
    /*! Created by OgreRenderingModule. Implements the RenderServiceInterface.
        \ingroup OgreRenderingModuleClient
    */
    class OGRE_MODULE_API Renderer : public Foundation::RenderServiceInterface
    {
        friend class EventListener;
        
    public:
        //! Constructor
        Renderer(Foundation::Framework* framework, const std::string& config, const std::string& plugins, const std::string& window_title);

        //! Destructor
        virtual ~Renderer();

        //! Do raycast into the world from viewport coordinates.
        /*! The coordinates are a position in the render window, not scaled to [0,1].
            \todo Returns raw pointer to entity. Returning smart pointer may take some thinking/design. Maybe just return entity id?

            \param x Horizontal position for the origin of the ray
            \param y Vertical position for the origin of the ray
            \return Raycast result structure
        */
        virtual Foundation::RaycastResult Raycast(int x, int y);

        //! Resizes the window
        virtual void Resize(uint width, uint height);

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
        
        //! Gets a renderer-specific resource
        /*! Does not automatically queue a download request
            \param id Resource id
            \param type Resource type
            \return pointer to resource, or null if not found
         */
        virtual Foundation::ResourcePtr GetResource(const std::string& id, const std::string& type);   
        
        //! Requests a renderer-specific resource to be downloaded from the asset system
        /*! A RESOURCE_READY event will be sent when the resource is ready to use
            \param id Resource id
            \param type Resource type
            \return Request tag, or 0 if request could not be queued
         */        
        virtual request_tag_t RequestResource(const std::string& id, const std::string& type);   
        
        //! Removes a renderer-specific resource
        /*! \param id Resource id
            \param type Resource type
         */
        virtual void RemoveResource(const std::string& id, const std::string& type);                    
        
        //! Callback when renderwindow closed
        /*! Sends event and exits the framework main loop
         */
        void OnWindowClosed();

        //! Sets external window parameter, for embedding the Ogre renderwindow. Usually a child window.
        void SetExternalWindowParameter(const std::string& param) { external_window_parameter_ = param; }

        //! set handle for the main window. This is for the top level window.
        void SetMainWindowHandle(uint hndl) { main_window_handle_ = hndl; }

        //! Returns framework
        Foundation::Framework* GetFramework() const { return framework_; }

        //! Returns initialized state
        bool IsInitialized() const { return initialized_; }

        //! Returns Ogre root
        OgreRootPtr GetRoot() const { return root_; }

        //! Returns Ogre scenemanager
        Ogre::SceneManager* GetSceneManager() const { return scenemanager_; }
        
        //! Returns Ogre viewport
        Ogre::Viewport* GetViewport() const { return viewport_; }
        
        //! Returns active camera
        /*! Note: use with care. Never set the position of the camera, but query rather the camera entity from scene,
            and use the EC_OgreCamera entity component + its placeable
         */ 
        Ogre::Camera* GetCurrentCamera() const { return camera_; }

        //! Returns current render window
        Ogre::RenderWindow* GetCurrentRenderWindow() const { return renderwindow_; }

        //! Returns an unique name to create Ogre objects that require a mandatory name
        std::string GetUniqueObjectName();

        //! Returns resource handler
        ResourceHandlerPtr GetResourceHandler() const { return resource_handler_; }
        
        //! Removes log listener
        void RemoveLogListener();
        
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
        void Update(f64 frametime);

        //! Sets current camera used for rendering the main viewport
        /*! Called by EC_OgreCamera when activating. Null will default to the default camera, so that we don't crash
            when rendering.
         */
        void SetCurrentCamera(Ogre::Camera* camera);

		//! Takes a screenshot and saves it to a file.
		void TakeScreenshot(const std::string& filePath, const std::string& fileName);//const Ogre::String& pyFilePath, const Ogre::String& pyFileName):

        //! Adds a directory into the Ogre resource system, to be able to load local Ogre resources from there
        /*! \param directory Directory path to add
         */
        void AddResourceDirectory(const std::string& directory);

        //! returns the composition handler responsible of the post-processing effects
        CompositionHandler& GetCompositionHandler(){ return c_handler_; }

    private:
        //! Loads Ogre plugins in a manner which allows individual plugin loading to fail
        /*! \param plugin_filename path & filename of the Ogre plugins file
         */
        void LoadPlugins(const std::string& plugin_filename);
        
        //! Sets up Ogre resources based on resources.cfg
        void SetupResources();
        
        //! Creates scenemanager & camera
        void SetupScene();
    
        boost::mutex renderer_;

        //! Successfully initialized flag
        bool initialized_;
        
        //! Ogre root object
        OgreRootPtr root_;
        
        //! Scene manager
        Ogre::SceneManager* scenemanager_;
        
        //! Default camera, used when no other camera exists
        Ogre::Camera* default_camera_;

        //! Current camera
        Ogre::Camera* camera_;
        
        //! Viewport
        Ogre::Viewport* viewport_;
        
        //! Rendering window
        Ogre::RenderWindow* renderwindow_;
        
        //! Framework we belong to
        Foundation::Framework* framework_;
        
        //! Ogre event listener
        EventListenerPtr listener_;

        //! Ogre log listener
        OgreLogListenerPtr log_listener_;
        
        //! Resource handler
        ResourceHandlerPtr resource_handler_;
        
        //! Renderer event category
        event_category_id_t renderercategory_id_;

        //! Counter for unique name creation
        uint object_id_;

        //! Counter for unique resource group creation
        uint group_id_;

        //! External window parameter, to be used when embedding the renderwindow
        std::string external_window_parameter_;

        //! handle for the main window
        uint main_window_handle_;

        //! filename for the Ogre3D configuration file
        std::string config_filename_;
        
        //! filename for the Ogre3D plugins file
        std::string plugins_filename_;

        //! ray for raycasting, reusable
        Ogre::RaySceneQuery *ray_query_;
        
        //! window title to be used when creating renderwindow
        std::string window_title_;
        
        //! added resource directories
        StringVector added_resource_directories_;

        //! Ogre UI View Widget, inherits QGraphicsView
        QOgreUIView *q_ogre_ui_view_;

        //! Ogre World View
        QOgreWorldView *q_ogre_world_view_;

        //! handler for post-processing effects
		CompositionHandler c_handler_;
    };
}

#endif
