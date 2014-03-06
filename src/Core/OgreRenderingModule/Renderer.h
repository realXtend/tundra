// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IRenderer.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"
#include "SceneFwd.h"
#include "HighPerfClock.h"

#include <QObject>
#include <QVariant>
#include <QDir>

class QScriptEngine;
class Framework;

namespace Ogre
{
    class OverlaySystem;

#ifdef ANDROID
    class StaticPluginLoader;
    namespace RTShader
    {
        class ShaderGenerator;
    }
#endif
}

namespace OgreRenderer
{
    class OgreLogListener;

    /// Ogre renderer
    /** Created by OgreRenderingModule. Implements the IRenderer.
        @ingroup OgreRenderingModuleClient */
    class OGRE_MODULE_API Renderer : public QObject, public IRenderer
    {
        Q_OBJECT
        Q_ENUMS(ShadowQualitySetting)
        Q_ENUMS(TextureQualitySetting)
        Q_PROPERTY(float viewDistance READ ViewDistance WRITE SetViewDistance)
        Q_PROPERTY(bool fullScreen READ IsFullScreen WRITE SetFullScreen)
        Q_PROPERTY(ShadowQualitySetting shadowQuality READ ShadowQuality WRITE SetShadowQuality)
        Q_PROPERTY(TextureQualitySetting textureQuality READ TextureQuality WRITE SetTextureQuality)
        Q_PROPERTY(int textureBudget READ TextureBudget WRITE SetTextureBudget)

    public:
        /// Constructor
        /** @param Framework pointer. */
        Renderer(Framework *framework);

        virtual ~Renderer();

        /// Returns framework
        Framework* GetFramework() const { return framework; }

        /// Returns initialized state
        bool IsInitialized() const { return initialized; }

        /// Returns Ogre root
        OgreRootPtr OgreRoot() const { return ogreRoot; }

        /// Returns Ogre viewport
        Ogre::Viewport *MainViewport() const { return mainViewport; }

        /// Returns current render window
        Ogre::RenderWindow* CurrentRenderWindow() const;

        /// Returns currently active Ogre camera
        /** @note in case there is no active camera, will not return the default (dummy) camera, but 0 */
        Ogre::Camera* MainOgreCamera() const;

        /// Returns the OgreWorld of the currently active camera
        OgreWorldPtr ActiveOgreWorld() const;

        /// Returns an unique name to create Ogre objects that require a mandatory name
        /** @param prefix Prefix for the name. */
        std::string GenerateUniqueObjectName(const std::string &prefix);

        /// Initializes renderer. Called by OgreRenderingModule
        void Initialize();

        /// returns the composition handler responsible of the post-processing effects
        OgreCompositionHandler *CompositionHandler() const { return compositionHandler; }

        /// Returns the globally used Ogre overlay system
        Ogre::OverlaySystem* OgreOverlaySystem() const { return overlaySystem; }

        /// Returns RenderWindow used to display the 3D scene in.
        RenderWindow *GetRenderWindow() const { return renderWindow; }

        /// Shadow quality settings
        enum ShadowQualitySetting
        {
            Shadows_Off = 0, ///< Off
            Shadows_Low, ///< One focused shadow map
            Shadows_High ///< PSSM, Direct3D only
        };

        /// Texture quality settings
        enum TextureQualitySetting
        {
            Texture_Low = 0, ///< Halved resolution
            Texture_Normal ///< Normal
        };

        /// Returns window width, or 0 if no render window
        int WindowWidth() const;

        /// Returns window height, or 0 if no render window
        int WindowHeight() const;

        /// Set maximum view distance.
        virtual void SetViewDistance(float distance);

        /// Returns maximum view distance.
        virtual float ViewDistance() const { return viewDistance; }

        /// Toggles fullscreen.
        void SetFullScreen(bool value);

        /// Is window fullscreen?
        bool IsFullScreen() const;

        /// Sets shadow quality.
        /** @note Changes need application restart to take effect due to Ogre resource system */
        void SetShadowQuality(ShadowQualitySetting newquality);

        /// Returns shadow quality.
        ShadowQualitySetting ShadowQuality() const { return shadowQuality; }

        /// Sets texture quality.
        /** @note Changes need application restart to take effect. */
        void SetTextureQuality(TextureQualitySetting newquality);

        /// Sets texture budget in megabytes
        void SetTextureBudget(int budget);

        /// Returns texture quality.
        TextureQualitySetting TextureQuality() const { return textureQuality; }

        /// Returns texture budget in megabytes.
        int TextureBudget() const { return textureBudget; }

        /// Calculate current texture usage ratio (1 = budget completely in use).
        /** @param loadDataSize Can be used to specify a data size in bytes which is going to be loaded, and will be added to the calculation. */
        float TextureBudgetUse(size_t loadDataSize = 0) const;

#ifdef ANDROID
        /// Returns the shader generator for converting fixed-function materials (Android only)
        Ogre::RTShader::ShaderGenerator* GetShaderGenerator() { return shaderGenerator; }
#endif
        // DEPRECATED
        /// @cond PRIVATE
        Ogre::RenderWindow* GetCurrentRenderWindow() const { return CurrentRenderWindow(); }
        Ogre::OverlaySystem* GetOverlaySystem() const { return OgreOverlaySystem(); }
        OgreWorldPtr GetActiveOgreWorld() const { return ActiveOgreWorld(); }
        virtual std::string GetUniqueObjectName(const std::string &prefix) { return GenerateUniqueObjectName(prefix); }
        /// @endcond

    public slots:
        /// Renders the screen. Advances Ogre's time internally by the frameTime specified
        virtual void Render(float frameTime);

        /// Add or remove Ogre resource location.
        /** @note Relative paths will be resolved to Application::InstallationDirectory.
            @see InitializeOgreResourceGroup.
            @param Path to the resource directory.
            @param Ogre resource group name to add the resource location to.
            @param True if you want to remove this location, false will add the location if it does not exist yet in this for the Ogre group name. */
        void ProcessOgreResourceLocation(const QString &directoryPath, const QString &ogreResourceGroup, bool removeLocation = false);

        /// Add or remove Ogre resource locations from a map of data.
        /** This map needs to have the same format as the rendering config passed to --ogreConfig.
            @code
            {
                "MyResourceGroupName" :
                [
                    { "Zip"        : "C:/my/resources.zip" },
                    { "FileSystem" : "media/custom-meshes" },
                ]
            }
            @endcode
            @see InitializeOgreResourceGroup.
            @param Data map. Keys are read as the Ogre resource group name. Value needs to be a list of maps, mapping resource location type to a path.
            @param Root directory where to resolve relative paths to.
            @param True if you want to remove this location, false will add the location if it does not exist yet in for the Ogre group name. */
        void ProcessOgreResourceLocations(const QVariantMap &resources, const QDir &rootDirectory, bool removeLocation = false);

        /// Initializes a Ogre resourcegroup.
        /** @see ProcessOgreResourceLocation and ProcessOgreResourceLocations.
            @param Ogre resource group name.
            @return True if resource group exists and the group is either already initialized or was initialized now. False if group does not exist. */
        bool InitializeOgreResourceGroup(const QString &ogreResourceGroup);

        /// Performs a full UI repaint with Qt and re-fills the GPU surface accordingly.
        void DoFullUIRedraw();

        /// Returns the Entity which contains the currently active camera that is used to render on the main window.
        /// The returned Entity is guaranteed to have an EC_Camera component, and it is guaranteed to be attached to a scene.
        Entity *MainCamera();

        /// Returns the EC_Camera of the main camera, or 0 if no main camera is active.
        EC_Camera *MainCameraComponent();

        /// Returns the Scene the current active main camera is in, or 0 if no main camera is active.
        Scene *MainCameraScene();

        /// Sets the given Entity as the main camera for the main window.
        /** This function fails if the given Entity does not have an EC_Camera component, or if the given Entity is not attached to a scene.
            Whenever the main camera is changed, the signal MainCameraChanged is triggered. */
        void SetMainCamera(Entity *mainCameraEntity);

        /// Creates a new hidden UiPlane with the given name.
        /** Remember to specify the Z order of the new plane, add some content to it, and call Show() when you want to display the UiPlane.
            The memory of the returned object is owned by Renderer. Call Renderer::DeleteUiPlane() to remove the UiPlane when you no longer need it. */
        UiPlane *CreateUiPlane(const QString &name);

        void DeleteUiPlane(UiPlane *plane);

        // DEPRECATED
        /// @cond PRIVATE
        /// Do raycast into the currently active world from viewport coordinates, using all selection layers
        /** The coordinates are a position in the render window, not scaled to [0,1].
            @deprecated This function is deprecated. You should use the OgreWorld::Raycast function instead.
            @todo Remove.
            @param x Horizontal position for the origin of the ray
            @param y Vertical position for the origin of the ray
            @return Raycast result structure */
        RaycastResult* Raycast(int x, int y);
        /// @endcond

    signals:
        /// Emitted every time the main window active camera changes.
        /** The pointer specified in this signal may be null, if the main camera was set to null.
            If the specified entity is non-zero, it is guaranteed to have an EC_Camera component, and it is attached to some scene. */
        void MainCameraChanged(Entity *newMainWindowCamera);

        /// Rendering device was created.
        /** @note If you intend to re-create resources, listen to this signal. */
        void DeviceCreated();

        /// Rendering device was released.
        void DeviceReleased();

        /// Rendering device was lost.
        void DeviceLost();

        /// Rendering device was restored.
        void DeviceRestored();

    private slots:
        /// Embeds the Renderer types to the given script engine.
        void OnScriptEngineCreated(QScriptEngine* engine);

    private:
        friend class OgreRenderingModule;
        friend class OgreRenderSystemListener;

        /// Sleeps the main thread to throttle the main loop execution speed.
        void DoFrameTimeLimiting();

        /// Returns the headless renderer for this platform specified by the Ogre config file, empty string if not defined.
        QString HeadlessRenderingPluginName(const QString &renderingConfig) const;

        /// Returns the headless renderer dynamic library filename for this platform specified by the Ogre config file, empty string if not defined.
        QString HeadlessRenderingPluginFilename(const QString &renderingConfig) const;

        /// Loads Ogre plugins specified by the Ogre config file in a manner which allows individual plugin loading to fail.
        QStringList LoadOgrePlugins(const QString &renderingConfig);

        /// Loads Ogre plugin from the plugin dir specified in the Ogre config file.
        bool LoadOgrePlugin(const QString &renderingConfig, const QString &pluginName);

        /// Load Ogre resource locations specified by the Ogre config file and initialize them.
        /** This does not actually load the assets, the files are just being recognized by the Ogre resource system. */
        void LoadOgreResourceLocations(const QString &renderingConfig);

        /// Create instancing variants of all vertex shaders
        void CreateInstancingShaders();

        /// Returns platform string that is used in --ogreConfig files.
        QString RenderingConfigPlatform() const;

        /// Emits renderer device loss/restore signals.
        void EmitDeviceLost();
        void EmitDeviceRestored();
        void EmitDeviceCreated();
        void EmitDeviceReleased();

        /// Successfully initialized flag
        bool initialized;

        /// Ogre root object
        OgreRootPtr ogreRoot;

        /// All created OgreWorlds (scene managers)
        std::map<Scene*, OgreWorldPtr> ogreWorlds;
        
        /// Stores all the created Ogre overlays.
        std::vector<UiPlane*> uiPlanes;

        /// Stores the camera that is active in the main window.
        EntityWeakPtr activeMainCamera;

        /// Dummy camera when we have no scene / no active camera.
        /// This is never shown outside to the client, but used as a placeholder
        /// so that Ogre doesn't crash internally when we don't have an active main camera in Tundra.
        Ogre::Camera *dummyDefaultCamera;

        /// Stores the main window viewport.
        Ogre::Viewport *mainViewport;

        /// Maximum view distance
        float viewDistance;

        /// Dummy scenemanager when we have no scene
        Ogre::SceneManager* defaultScene;

        RenderWindow *renderWindow;
        Ogre::OverlaySystem* overlaySystem;

        #ifdef ANDROID
        Ogre::StaticPluginLoader* staticPluginLoader;
        Ogre::RTShader::ShaderGenerator* shaderGenerator;
        #endif
        
        /// Framework we belong to
        Framework* framework;

        /// Counter for unique name creation
        uint uniqueObjectId;

        /// handler for post-processing effects
        OgreCompositionHandler *compositionHandler;

        int lastHeight; ///< Last render window height
        int lastWidth; ///< Last render window width
        int resizedDirty; ///< Resized dirty count
        ShadowQualitySetting shadowQuality; ///< Shadow quality setting.
        TextureQualitySetting textureQuality; ///< Texture quality setting.
        int textureBudget; ///< Texture budget in megabytes.

        /// Stores the wall clock time that specifies when the last frame was displayed.
        tick_t lastPresentTime;
        /// Caches the system clock frequency.
        tick_t timerFrequency;

        /// We use a custom OgreLogListener to classify and filter the Ogre output.
        /// The normal Ogre output to cerr is disabled.
        OgreLogListener *logListener;
    };
}

Q_DECLARE_METATYPE(OgreRenderer::Renderer::ShadowQualitySetting)
Q_DECLARE_METATYPE(OgreRenderer::Renderer::TextureQualitySetting)
