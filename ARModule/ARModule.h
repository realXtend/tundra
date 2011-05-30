// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AR_ARModule_h
#define incl_AR_ARModule_h

#include "ARModuleApi.h"
#include "IModule.h"
#include "ModuleLoggingFunctions.h"
#include "ForwardDefines.h"
#include "ARFwd.h"

#include "AttributeChangeType.h"

#include <QImage>
#include <QAction>

namespace AR
{
    /**
    <table class="header"><tr><td>
    <h2>ARModule</h2>

    ARModule implements augmented reality entity components and rendering.

    ARModule provides script accessible functions to start camera input to be redirected to the back of the ogre scene. This is a essential part in doing mixed/augmented reality applications. 
    ARModule declares EC_ARGeometry that is a key component. The component can be instucted to be a "see through" object that does not draw anything to the color buffer in ogre rendering.

    This module depends on OgreRenderingModule and CameraInputModule.

    </td></tr></table>
    */
    class AR_MODULE_API ARModule : public QObject, public IModule
    {
    
    Q_OBJECT

    public:
        /// ARModule ctor.
        ARModule();

        /// ARModule dtor.
        virtual ~ARModule();

        /// IModule override.
        void Load();
        
        /// IModule override.
        void Initialize();
        
        /// IModule override.
        void PostInitialize();

        /// IModule override.
        void Update(f64 frametime);

        /// IModule override.
        void Uninitialize();

        /// Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return moduleName_; }

        /// Module logging functions
        MODULE_LOGGING_FUNCTIONS

    public slots:
        /// Returns if reality from camera is being fed to rendering.
        /// \return bool True if feeding, false otherwise.
        bool IsFeedingReality();

        /// Stop reality feeding.
        void StartRealityFeed();

        /// Start reality feeding.
        void StopRealityFeed();

    signals:
        /// Signal that informs the new feeding reality state.
        /// \param bool True if feeding, false otherwise.
        void FeedingReality(bool feeding);

    private slots:
        /// Handler for our feed ui action triggered signal.
        void OnFeedAction();

        /// Handler for listening to connected events from the client api.
        void ClientConnected();

        /// Handler for listening to capture state of the camera input.
        /// \param bool True if capturing, false otherwise.
        void RealityCaptureStateChanged(bool capturing);

        /// Handler for frame updates from the camera input.
        /// \param QImage Frame image data.
        void RealityFrameUpdate(const QImage &frame);

        /// Handler for default scene set signal from SceneAPI.
        /// \param Scene::SceneManager* The current scene.
        void DefaultSceneAdded(Scene::SceneManager* mngr);

        /// Handler for attribute changed signal from the current scene.
        void ComponentAttributeChanged(IComponent* comp, IAttribute* attribute, AttributeChange::Type change);   

    private:
        /// Name of this module.
        static const std::string moduleName_;

        /// Pointer to ARRenderer, that is responsible for managing objects to be renderered
        ARRenderer* arRenderer_;

        /// The actual renderer (a basic Tundra renderer)*/
        OgreRenderer::Renderer* renderer_;

        /// Camera input api object.
        CameraInput *cameraInput_;

        /// Tracking if we are in a camera data feeding state.
        bool isFeeding_;

        /// Action to the client toolbar to enable reality feed.
        QAction *actionFeed_;
    };
}

#endif
