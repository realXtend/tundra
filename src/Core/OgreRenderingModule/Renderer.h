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
class QScriptEngine;
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
        int GetWindowWidth() const;

        /// Returns window height, or 0 if no render window
        int GetWindowHeight() const;

        /// Adds a directory into the Ogre resource system, to be able to load local Ogre resources from there
        /** @param directory Directory path to add. */
        void AddResourceDirectory(const QString &directory);

        /// Toggles fullscreen
        void SetFullScreen(bool value);

        /// Performs a full UI repaint with Qt and re-fills the GPU surface accordingly.
        void DoFullUIRedraw();

        /// Do raycast into the currently active world from viewport coordinates, using all selection layers
        /// \todo This function is deprecated. You should use the OgreWorld::Raycast function instead.
        /** The coordinates are a position in the render window, not scaled to [0,1].
            @param x Horizontal position for the origin of the ray
            @param y Vertical position for the origin of the ray
            @return Raycast result structure */
        virtual RaycastResult* Raycast(int x, int y);

        /// Returns the Entity which contains the currently active camera that is used to render on the main window.
        /// The returned Entity is guaranteed to have an EC_Camera component, and it is guaranteed to be attached to a scene.
        Entity *MainCamera();

        /// Returns the EC_Camera of the main camera, or 0 if no main camera is active.
        EC_Camera *MainCameraComponent();

        /// Returns the Scene the current active main camera is in, or 0 if no main camera is active.
        Scene *MainCameraScene();

        /// Sets the given Entity as the main camera for the main window.
        /// This function fails if the given Entity does not have an EC_Camera component, or if the given Entity is not attached to a scene.
        /// Whenever the main camera is changed, the signal MainCameraChanged is triggered.
        void SetMainCamera(Entity *mainCameraEntity);

    signals:
        /// Emitted every time the main window active camera changes.
        /// The pointer specified in this signal may be null, if the main camera was set to null.
        /// If the specified entity is non-zero, it is guaranteed to have an EC_Camera component, and it is attached to some scene.
        void MainCameraChanged(Entity *newMainWindowCamera);

    public:
        /// Constructor
        /** @param framework Framework pointer.
            @param config Config filename.
            @param plugins Plugins filename.
            @param window_title Renderer window title. */
        Renderer(Framework* framework, const std::string& config, const std::string& plugins, const std::string& window_title);

        /// Destructor
        virtual ~Renderer();

        /// set maximum view distance
        virtual void SetViewDistance(float distance);

        /// get maximum view distance
        virtual float ViewDistance()const { return view_distance_; }

        ///Is window fullscreen?
        bool IsFullScreen() const;

        /// Returns framework
        Framework* GetFramework() const { return framework_; }

        /// Returns initialized state
        bool IsInitialized() const { return initialized_; }

        /// Returns Ogre root
        OgreRootPtr GetRoot() const { return root_; }

        /// Returns Ogre viewport
        Ogre::Viewport *MainViewport() const { return mainViewport; }

        /// Returns current render window
        Ogre::RenderWindow* GetCurrentRenderWindow() const;

        /// Returns currently active Ogre camera
        /** @note in case there is no active camera, will not return the default (dummy) camera, but 0 */
        Ogre::Camera* MainOgreCamera() const;

        /// Returns the OgreWorld of the currently active camera
        OgreWorldPtr GetActiveOgreWorld() const;

        /// Returns an unique name to create Ogre objects that require a mandatory name
        /** @param prefix Prefix for the name. */
        std::string GetUniqueObjectName(const std::string &prefix);

        /// Initializes renderer. Called by OgreRenderingModule
        void Initialize();

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

    private slots:
        /// Embeds the Renderer types to the given script engine.
        void OnScriptEngineCreated(QScriptEngine* engine);

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
        
        /// Stores the camera that is active in the main window.
        boost::weak_ptr<Entity> activeMainCamera;

        /// Dummy camera when we have no scene / no active camera.
        /// This is never shown outside to the client, but used as a placeholder
        /// so that Ogre doesn't crash internally when we don't have an active main camera in Tundra.
        Ogre::Camera *dummyDefaultCamera;

        /// Stores the main window viewport.
        Ogre::Viewport *mainViewport;

        /// Maximum view distance
        float view_distance_;

        /// Dummy scenemanager when we have no scene
        Ogre::SceneManager* defaultScene_;

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

        /// Stores the wall clock time that specifies when the last frame was displayed.
        tick_t lastPresentTime;
        /// Caches the system clock frequency.
        tick_t timerFrequency;

        /// We use a custom OgreLogListener to classify and filter the Ogre output.
        /// The normal Ogre output to cerr is disabled.
        OgreLogListener *logListener;
    };
}
