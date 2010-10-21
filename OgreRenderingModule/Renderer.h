// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_Renderer_h
#define incl_OgreRenderer_Renderer_h

#include "RenderServiceInterface.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"
#include "RenderServiceInterface.h"
#include "CompositionHandler.h"
#include "ForwardDefines.h"

#include <QObject>
#include <QVariant>
#include <QTime>
#include <QRect>
#include <QPixmap>
#include <QImage>

class NaaliRenderWindow;

namespace OgreRenderer
{
    /// Shadow quality settings
    enum ShadowQuality
    {
        Shadows_Off = 0, ///< Off
        Shadows_Low, ///< One focused shadow map
        Shadows_High ///< PSSM, Direct3D only
    };

    /// Texture quality settings
    enum TextureQuality
    {
        Texture_Low = 0, ///< Halved resolution
        Texture_Normal ///< Normal
    };

    class OgreRenderingModule;
    class LogListener;
    class ResourceHandler;
    class RenderableListener;
    class CAVEManager;
    class StereoController;
    class CompositionHandler;
    class GaussianListener;

    typedef boost::shared_ptr<Ogre::Root> OgreRootPtr;
    typedef boost::shared_ptr<LogListener> OgreLogListenerPtr;
    typedef boost::shared_ptr<ResourceHandler> ResourceHandlerPtr;
    typedef boost::shared_ptr<RenderableListener> RenderableListenerPtr;

    //! Ogre renderer
    /*! Created by OgreRenderingModule. Implements the RenderServiceInterface.
        \ingroup OgreRenderingModuleClient
    */
    class OGRE_MODULE_API Renderer : public QObject, public Foundation::RenderServiceInterface
    {
        friend class RenderableListener;

        Q_OBJECT

    public slots:
        //! Renders the screen. 
        //as a slot for webserver plugin to render when needed
        virtual void Render();

        //! Do a frustum query to the world from viewport coordinates.
        virtual QVariantList FrustumQuery(QRect &viewrect);

        //! Hides world view
        //also added for the webserver plugin
//        void HideCurrentWorldView(); 

        //! Shows world view
//        void ShowCurrentWorldView();

        //! Returns window width, or 0 if no render window
        virtual int GetWindowWidth() const;

        //! Returns window height, or 0 if no render window
        virtual int GetWindowHeight() const;

        //! Adds a directory into the Ogre resource system, to be able to load local Ogre resources from there
        /*! \param directory Directory path to add
         */
        //for local dotscene loading to be able to load from the dir where the export is
        void AddResourceDirectory(const QString &directory);

    public:
        //! Constructor
        /*! \param framework Framework pointer.
            \param config Config filename.
            \param plugins Plugins filename.
            \param window_title Renderer window title.
        */
        Renderer(
            Foundation::Framework* framework,
            const std::string& config,
            const std::string& plugins,
            const std::string& window_title);

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
        
        //! Subscribe a listener to renderer log. Can be used before renderer is initialized.
        virtual void SubscribeLogListener(const Foundation::LogListenerPtr &listener);

        //! Unsubsribe a listener to renderer log. Can be used before renderer is initialized.
        virtual void UnsubscribeLogListener(const Foundation::LogListenerPtr &listener);

        //! set maximum view distance
        virtual void SetViewDistance(float distance) { view_distance_ = distance; }

        //! get maximum view distance
        virtual float GetViewDistance()const { return view_distance_; }

        //! force UI repaint
        virtual void RepaintUi();

        //!Is window fullscreen?
        bool IsFullScreen() const;

        //! get visible entities last frame
        virtual const std::set<entity_id_t>& GetVisibleEntities() { return visible_entities_; }

        //! Takes a screenshot and saves it to a file.
        //! \param filePath File path.
        //! \param fileName File name.
        virtual void TakeScreenshot(const std::string& filePath, const std::string& fileName);

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
        Ogre::RenderWindow* GetCurrentRenderWindow() const;// { return renderwindow_; }

        //! Returns an unique name to create Ogre objects that require a mandatory name
        ///\todo Generates object names, not material or billboardset names, but anything unique goes.
        /// Perhaps would be nicer to just have a GetUniqueName(string prefix)?
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

        //! returns the composition handler responsible of the post-processing effects
        CompositionHandler *GetCompositionHandler() const { return c_handler_; }

        //! Returns the main window.
//        Foundation::MainWindow *GetMainWindow() const { return main_window_; }

        /// Returns the backbuffer image that contains the UI layer of the application screen.
        /// Used to perform alpha-keying based input.
//        QImage &GetBackBuffer() { return backBuffer; }

        //! Returns shadow quality
        ShadowQuality GetShadowQuality() const { return shadowquality_; }

        //! Sets shadow quality. Note: changes need viewer restart to take effect due to Ogre resource system
        void SetShadowQuality(ShadowQuality newquality);

        //! Returns texture quality
        TextureQuality GetTextureQuality() const { return texturequality_; }

        //! Sets texture quality. Note: changes need viewer restart to take effect
        void SetTextureQuality(TextureQuality newquality);

        NaaliRenderWindow *GetRenderWindow() const { return renderWindow; }

    public slots:
        //! Toggles fullscreen
        void SetFullScreen(bool value);

        //! Render current main window content to texture
        virtual QPixmap RenderImage(bool use_main_camera = true);

        //! Render current main window with focus on the avatar
        //! @todo make this focus non hard coded but as param
        virtual QPixmap RenderAvatar(const Vector3df &avatar_position, const Quaternion &avatar_orientation);

        //! Prepapres the texture and entities used in texture rendering
        void PrepareImageRendering(int width, int height);

        //! Reset the texture
        void ResetImageRendering();

        QImage CreateQImageFromTexture(Ogre::RenderTexture *render_texture, int width, int height);

    private:
        //! Initialises the events related info for this module
        void InitializeEvents();

        //! Loads Ogre plugins in a manner which allows individual plugin loading to fail
        /*! \param plugin_filename path & filename of the Ogre plugins file
         */
        void LoadPlugins(const std::string& plugin_filename);

        //! Sets up Ogre resources based on resources.cfg
        void SetupResources();

        //! Creates scenemanager & camera
        void SetupScene();

        //! Initializes shadows. Called by SetupScene().
        void InitShadows();

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

        //! Maximum view distance
        float view_distance_;

        //! Viewport
        Ogre::Viewport* viewport_;

        NaaliRenderWindow *renderWindow;

        //! Framework we belong to
        Foundation::Framework* framework_;

        //! Ogre log listener
        OgreLogListenerPtr log_listener_;

        //! Ogre renderable listener
        RenderableListenerPtr renderable_listener_;

        //! Resource handler
        ResourceHandlerPtr resource_handler_;

        //! Renderer event category
        event_category_id_t renderercategory_id_;

        //! Counter for unique name creation
        uint object_id_;

        //! Counter for unique resource group creation
        uint group_id_;

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

        //! handler for post-processing effects
        CompositionHandler *c_handler_;

        //! last width/height
        int last_height_;
        int last_width_;

        //! resized dirty count
        int resized_dirty_;

        //! For render function
        QImage ui_buffer_;
        QRect last_view_rect_;
        QTime ui_update_timer_;

        //! Visible entities
        std::set<entity_id_t> visible_entities_;

        //! Shadow quality
        ShadowQuality shadowquality_;

        //! Texture quality
        TextureQuality texturequality_;

        //! Soft shadow gaussian listeners
        std::list<GaussianListener *> gaussianListeners_;

        //! RenderImage() services texture
        std::string image_rendering_texture_name_;

        Scene::EntityPtr texture_rendering_cam_entity_;

        // Pixel buffer used with screen captures
        Ogre::uchar *capture_screen_pixel_data_;
    };
}

#endif
