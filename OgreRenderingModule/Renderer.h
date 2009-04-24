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
    /*! Created by OgreRenderingModule.
     */
    class Renderer : public Foundation::RenderServiceInterface
    {
        friend class EventListener;
        
    public:
        //! constructor
        Renderer(Foundation::Framework* framework);

        //! destructor
        virtual ~Renderer();

        //! initializes renderer
        /*! creates render window. if render window is to be embedded, call SetExternalWindowParameter() before.
         */
        void Initialize();

        //! post-initializes renderer
        /*! queries event categories it needs
         */
        void PostInitialize();

        //! sets external window parameter, for embedding the Ogre renderwindow
        void SetExternalWindowParameter(const std::string& param) { external_window_parameter_ = param; }

        //! returns initialized state
        bool IsInitialized() const { return initialized_; }

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

        Foundation::Framework* GetFramework() const { return framework_; }

        //! subscribe a listener to renderer log. Can be used before renderer is initialized.
        void SubscribeLogListener(const Foundation::LogListenerPtr &listener);
        //! unsubsribe a listener to renderer log. Can be used before renderer is initialized.
        void UnsubscribeLogListener(const Foundation::LogListenerPtr &listener);

        //! performs update.
        /*! pumps Ogre window events.
         */
        void Update(Core::f64 frametime);
        
        //! resizes the window
        void Resize(Core::uint width, Core::uint height);

        //! callback when renderwindow closed
        /*! sends event and exits the framework main loop
         */
        void OnWindowClosed();

        //! renders the screen
        void Render();
        
        //! handles an asset system event
        bool HandleAssetEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);

        //! handles a resource event
        bool HandleResourceEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);


        //! returns an Ogre texture resource, null if not found
        /*! does not automatically make a request to the asset system
         */
        Foundation::ResourcePtr GetTexture(const std::string& id);

        //! requests a texture to be downloaded & decoded
        /*! a resource event will be sent once each texture quality level is decoded
         */
        void RequestTexture(const std::string& id);

        //! deletes an Ogre texture resource
        void RemoveTexture(const std::string& id);


        //! returns an Ogre mesh resource, null if not found
        /*! does not automatically make a request to the asset system
         */
        Foundation::ResourcePtr GetMesh(const std::string& id);

        //! requests a mesh to be downloaded
        /*! a resource event will be sent once the mesh asset is ready
         */
        void RequestMesh(const std::string& id);

        //! deletes an Ogre mesh resource
        void RemoveMesh(const std::string& id);


    private:
        //! loads Ogre plugins in a manner which allows individual plugin loading to fail
        /*! \param plugin_filename path & filename of the Ogre plugins file
         */
        void LoadPlugins(const std::string& plugin_filename);
        
        //! sets up Ogre resources based on resources.cfg
        void SetupResources();
        
        //! creates scenemanager & camera
        void SetupScene();
    
        //! creates or updates a texture, based on a source raw texture resource
        /*! \param source raw texture
            \return true if successful
         */
        bool UpdateTexture(Foundation::ResourcePtr source);

        //! creates or updates a mesh, based on source asset data
        /*! \param source asset
            \return true if successful
         */
        bool UpdateMesh(Foundation::AssetPtr source);


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
        Core::event_category_id_t renderercategory_id_;
        
        //! Resource event category
        Core::event_category_id_t resourcecategory_id_;

        //! counter for unique name creation
        Core::uint object_id_;

        //! Ogre texture resources
        Foundation::ResourceMap textures_;

        //! Ogre mesh resources
        Foundation::ResourceMap meshes_;

        //! External window parameter, to be used when embedding the renderwindow
        std::string external_window_parameter_;
    };
}

#endif
