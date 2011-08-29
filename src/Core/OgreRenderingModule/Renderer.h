// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IRenderer.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"
#include "SceneFwd.h"
#include "HighPerfClock.h"

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <OgrePrerequisites.h>

#include <boost/enable_shared_from_this.hpp>

class QRect;
class RenderWindow;
class Framework;

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

    class OgreLogListener;

    /// Ogre renderer
    /** Created by OgreRenderingModule. Implements the IRenderer.
        \ingroup OgreRenderingModuleClient */
    class OGRE_MODULE_API Renderer : public QObject, public IRenderer
    {
        Q_OBJECT

    public slots:
        /// Renders the screen. Advances Ogre's time internally by the frameTime specified
        virtual void Render(float frameTime);

        /// Returns window width, or 0 if no render window
        virtual int GetWindowWidth() const;

        /// Returns window height, or 0 if no render window
        virtual int GetWindowHeight() const;

        /// Takes a screen shot and saves it to a file.
        /// @note Remove this when EC_Camera implements screen shot functionality.
        /// @param filePath Directory to save file.
        /// @param fileName File name in directory.
        virtual void TakeScreenshot(const QString& filePath, const QString& fileName);

        /// Adds a directory into the Ogre resource system, to be able to load local Ogre resources from there
        /** @param directory Directory path to add. */
        void AddResourceDirectory(const QString &directory);

        /// Toggles fullscreen
        void SetFullScreen(bool value);

        /// Render current main window content to texture
//        virtual QPixmap RenderImage(bool use_main_camera = true);

        /// Prepapres the texture and entities used in texture rendering
//        void PrepareImageRendering(int width, int height);

        /// Reset the texture
//        void ResetImageRendering();

//        QImage CreateQImageFromTexture(Ogre::RenderTexture *render_texture, int width, int height);

        /// Performs a full UI repaint with Qt and re-fills the GPU surface accordingly.
        void DoFullUIRedraw();

        /// Do raycast into the currently active world from viewport coordinates, using all selection layers
        /// \todo This function will be removed and replaced with a function Scene::Intersect.
        /** The coordinates are a position in the render window, not scaled to [0,1].
            @param x Horizontal position for the origin of the ray
            @param y Vertical position for the origin of the ray
            @return Raycast result structure */
        virtual RaycastResult* Raycast(int x, int y);

        /// Do raycast into the currently active world from viewport coordinates, using specific selection layer(s)
        /// \todo This function will be removed and replaced with a function Scene::Intersect.
        /** The coordinates are a position in the render window, not scaled to [0,1].
            @param x Horizontal position for the origin of the ray
            @param y Vertical position for the origin of the ray
            @param layerMask Which selection layer(s) to use (bitmask)
            @return Raycast result structure */
        virtual RaycastResult* Raycast(int x, int y, unsigned layerMask);

        /// Do a raycast from a world coordinate to another
        /** Takes two absolute position vectors as parameters
            @param pos The origin of the generated ray
            @param dir Direction of the ray, automatically normalized
            @return Raycast result structure */
        virtual RaycastResult* RaycastFromTo(const float3 &pos, const float3 &dir);

        /// Do a frustum query to the currently active world from viewport coordinates.
        /// \todo This function will be removed and replaced with a function Scene::Intersect.
        /** @param viewrect The query rectangle in 2d window coords. */
        virtual QList<Entity*> FrustumQuery(QRect &viewrect);

        /// Returns currently active camera component. Returned as IComponent* for scripting convenience.
        IComponent* GetActiveCamera() const;

    public:
        /// Constructor
        /** @param framework Framework pointer.
            @param config Config filename.
            @param plugins Plugins filename.
            @param window_title Renderer window title. */
        Renderer(Framework* framework, const std::string& config, const std::string& plugins, const std::string& window_title);

        /// Destructor
        virtual ~Renderer();

        /// Set maximum view distance
        virtual void SetViewDistance(float distance);

        /// Get maximum view distance
        virtual float ViewDistance()const { return view_distance_; }

        /// Is window full screen?
        bool IsFullScreen() const;

        /// Returns framework
        Framework* GetFramework() const { return framework_; }

        /// Returns initialized state
        bool IsInitialized() const { return initialized_; }

        /// Returns Ogre root
        OgreRootPtr GetRoot() const { return root_; }

        /// Returns Ogre viewport
        Ogre::Viewport* GetViewport() const { return viewport_; }

        /// Returns current render window
        Ogre::RenderWindow* GetCurrentRenderWindow() const;

        /// Returns currently active Ogre camera
        /** @note in case there is no active camera, will not return the default (dummy) camera, but 0 */
        Ogre::Camera* GetActiveOgreCamera() const;

        /// Returns the OgreWorld of the currently active camera
        OgreWorldPtr GetActiveOgreWorld() const;

        /// Returns an unique name to create Ogre objects that require a mandatory name
        /** @param prefix Prefix for the name. */
        std::string GetUniqueObjectName(const std::string &prefix);

        /// Initializes renderer. Called by OgreRenderingModule
        void Initialize();

        /// Sets current camera component used for rendering the main viewport
        /** Called by EC_Camera when activating. Null will default to the default camera, so that we don't crash when rendering. */
        void SetActiveCamera(EC_Camera* camera);

        /// returns the composition handler responsible of the post-processing effects
        CompositionHandler *GetCompositionHandler() const { return c_handler_; }

        /// Returns shadow quality
        ShadowQuality GetShadowQuality() const { return shadowquality_; }

        /// Sets shadow quality. Note: changes need viewer restart to take effect due to Ogre resource system
        void SetShadowQuality(ShadowQuality newquality);

        /// Returns texture quality
        TextureQuality GetTextureQuality() const { return texturequality_; }

        /// Sets texture quality. Note: changes need viewer restart to take effect
        void SetTextureQuality(TextureQuality newquality);

        RenderWindow *GetRenderWindow() const { return renderWindow; }

    private:
        friend class OgreRenderingModule;

        /// Sleeps the main thread to throttle the main loop execution speed.
        void DoFrameTimeLimiting();

        /// Loads Ogre plugins in a manner which allows individual plugin loading to fail
        /** @param plugin_filename path & filename of the Ogre plugins file. 
            @return Successfully loaded plugin names in a QStringList */
        QStringList LoadPlugins(const std::string& plugin_filename);

        /// Sets up Ogre resources based on resources.cfg
        void SetupResources();

        /// Prepare the config with needed default values if they are not there.
        void PrepareConfig();

        /// Successfully initialized flag
        bool initialized_;

        /// Ogre root object
        OgreRootPtr root_;

        /// Default hardware buffer manager for headless mode
        Ogre::DefaultHardwareBufferManager* buffermanager_;
        
        /// All created OgreWorlds (scene managers)
        std::map<Scene*, OgreWorldPtr> ogreWorlds_;
        
        /// Currently active camera component
        EC_Camera* cameraComponent_;

        /// Maximum view distance
        float view_distance_;

        /// Dummy scenemanager when we have no scene
        Ogre::SceneManager* defaultScene_;
        /// Dummy camera when we have no scene / no active camera
        Ogre::Camera* defaultCamera_;

        /// Viewport
        Ogre::Viewport* viewport_;

        RenderWindow *renderWindow;

        /// Framework we belong to
        Framework* framework_;

        /// Counter for unique name creation
        uint object_id_;

        /// Counter for unique resource group creation
        uint group_id_;

        /// filename for the Ogre3D configuration file
        std::string config_filename_;

        /// filename for the Ogre3D plugins file
        std::string plugins_filename_;

        /// window title to be used when creating renderwindow
        std::string window_title_;

        /// added resource directories
        StringVector added_resource_directories_;

        /// handler for post-processing effects
        CompositionHandler *c_handler_;
        
        /// last width/height
        int last_height_;
        int last_width_;

        /// resized dirty count
        int resized_dirty_;

        /// Shadow quality
        ShadowQuality shadowquality_;

        /// Texture quality
        TextureQuality texturequality_;
        
        /// Pixel buffer used with screen captures
        Ogre::uchar *capture_screen_pixel_data_;
        
        /// Stores the wall clock time that specifies when the last frame was displayed.
        tick_t lastPresentTime;
        /// Caches the system clock frequency.
        tick_t timerFrequency;

        /// We use a custom OgreLogListener to classify and filter the Ogre output.
        /// The normal Ogre output to cerr is disabled.
        OgreLogListener *logListener;
    };
}
