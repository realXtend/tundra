/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   RexLogicModule.h
 *  @brief  The main client module of Naali.
 *
 *          RexLogicModule is the main client module of Naali and controls
 *          most of the world logic e.g. the default world scene creation and deletion,
 *          avatars, prims, and camera.
 *
 *  @note   Avoid direct module dependency to RexLogicModule at all costs because
 *          it's likely to cause cyclic dependy which fails the whole application.
 *          Instead use the WorldLogicInterface or different entity-components which
 *          are not within RexLogicModule. 
 *
 *          Currenly e.g. PythonScriptModule is highly dependant of RexLogicModule,
 *          but work is made to overcome this. If some feature you need from RexLogicModule
 *          is missing, please try to find a delicate abstraction/mechanism/design for retrieving it,
 *          instead of just hacking it to RexLogicModule directly.
 */

#ifndef incl_RexLogicModule_RexLogicModule_h
#define incl_RexLogicModule_RexLogicModule_h

#include "IModule.h"
#include "WorldLogicInterface.h"
#include "ModuleLoggingFunctions.h"
#include "RexLogicModuleApi.h"
#include "Quaternion.h"

#include <set>
#include <boost/function.hpp>
#include <QObject>
#include <QMap>
#include <QVariant>

class QString;
class RexUUID;

namespace OgreRenderer
{
    class Renderer;
    typedef boost::shared_ptr<OgreRenderer::Renderer> RendererPtr;
}

namespace ProtocolUtilities
{
    class WorldStream;
    class ProtocolModuleInterface;
}

namespace Avatar
{
    class AvatarHandler;
    class AvatarControllable;
    class AvatarEditor;

    typedef boost::shared_ptr<AvatarHandler> AvatarHandlerPtr;
    typedef boost::shared_ptr<AvatarControllable> AvatarControllablePtr;
    typedef boost::shared_ptr<AvatarEditor> AvatarEditorPtr;
}

typedef boost::shared_ptr<ProtocolUtilities::WorldStream> WorldStreamPtr;

namespace RexLogic
{
    class RexMovementInput;
    class NetworkEventHandler;
    class InputEventHandler;
    class SceneEventHandler;
    class NetworkStateEventHandler;
    class FrameworkEventHandler;
    class AvatarEventHandler;
    class Primitive;
    class CameraControllable;
    class MainPanelHandler;
    class WorldInputLogic;
    class LoginHandler;
    class ObjectCameraController;
    class CameraControl;
    class NotificationWidget;

    namespace InWorldChat { class Provider; }

    typedef boost::shared_ptr<InWorldChat::Provider> InWorldChatProviderPtr;
    typedef boost::shared_ptr<Primitive> PrimitivePtr;
    typedef boost::shared_ptr<CameraControllable> CameraControllablePtr;
    typedef boost::shared_ptr<ObjectCameraController> ObjectCameraControllerPtr;
    typedef boost::shared_ptr<CameraControl> CameraControlPtr;
    typedef boost::shared_ptr<NotificationWidget> NotificationWidgetPtr;

    //! Camera states handled by rex logic
    enum CameraState
    {
        //! Camera follows the avatar (third or first person)
        CS_Follow,
        //! Camera moves around freely
        CS_Free,
        //! Camera tripod
        CS_Tripod,
    };

    class REXLOGIC_MODULE_API RexLogicModule : public Foundation::WorldLogicInterface, public IModule
    {
        Q_OBJECT

    public:
        //! Default constructor.
        RexLogicModule();

        //! Destructor.
        virtual ~RexLogicModule();

        //! ModuleInterfaceImpl overrides.
        virtual void Load();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        virtual void Update(f64 frametime);
        virtual bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);
        static const std::string &NameStatic() { return type_name_static_; }
        MODULE_LOGGING_FUNCTIONS;

        //=============== WorldLogicInterface API ===============/
        Scene::EntityPtr GetUserAvatarEntity() const;
        Scene::EntityPtr GetCameraEntity() const;
        Scene::EntityPtr GetEntityWithComponent(uint entity_id, const QString &component) const;
        const QString &GetAvatarAppearanceProperty(const QString &name) const;
        float GetCameraControllablePitch() const;

        //=============== RexLogicModule API ===============/

        //! Returns world stream pointer.
        WorldStreamPtr GetServerConnection() const { return world_stream_; }

        //! switch current input controller, if using avatar controller, switch to camera controller and vice versa
        void SwitchCameraState();

        //! camera tripod
        void CameraTripod();

        //! @return The avatar handler object that manages reX avatar logic.
        Avatar::AvatarHandlerPtr GetAvatarHandler() const;

        //! @return The avatar editor.
        Avatar::AvatarEditorPtr GetAvatarEditor() const;
        
        //! Returns the avatar controllable
        Avatar::AvatarControllablePtr GetAvatarControllable() const;

        //! @return The primitive handler object that manages reX primitive logic.
        PrimitivePtr GetPrimitiveHandler() const;

        //! Returns the camera controllable
        CameraControllablePtr GetCameraControllable() const { return camera_controllable_; }

        //! Creates a new scene and sets that as active. Also creates the core entities to that scene that 
        //! are always to be present in an reX world, like terrain.
        Scene::ScenePtr CreateNewActiveScene(const QString &name);

        //! Deletes the scene with the given name. If that was the current active scene, the active scene will be
        //! set to null.
        void DeleteScene(const QString &name);

        //! @return The entity corresponding to given scene entityid, or null if not found. 
        //!         This entity is guaranteed to have an existing EC_OpenSimPrim component.
        __inline Scene::EntityPtr GetPrimEntity(entity_id_t entityid) const{ return GetEntityWithComponent(entityid, "EC_OpenSimPrim"); }
        Scene::EntityPtr GetPrimEntity(const RexUUID &fullid) const;

        //! @return The entity corresponding to given scene entityid, or null if not found. 
        //!         This entity is guaranteed to have an existing EC_OpenSimAvatar component,
        //!         and EC_OpenSimPresence component.
        __inline Scene::EntityPtr GetAvatarEntity(entity_id_t entityid) const { return GetEntityWithComponent(entityid, "EC_OpenSimAvatar"); }
        Scene::EntityPtr GetAvatarEntity(const RexUUID &fullid) const;

        InWorldChatProviderPtr GetInWorldChatProvider() const;

        //! Register uuid - localid pair
        void RegisterFullId(const RexUUID &fullid, entity_id_t entityid);

        //! Unregister uuid
        void UnregisterFullId(const RexUUID &fullid);

        //! handles assignment of object as a child of other object, if applicable
        void HandleObjectParent(entity_id_t entityid);

        //! handles assignment of child objects, who were previously missing this object as a parent
        void HandleMissingParent(entity_id_t entityid);

        //! login from py - temp while loginui misses dllexport
        void StartLoginOpensim(const QString &firstAndLast, const QString &password, const QString &serverAddressWithPort);

        //! XXX have linking probs to AvatarController so trying this wrapper
        //! \todo figure workarounds for these functions so that dependency to RexLogicModule
        //! is not needed anymore.
        void SetAvatarYaw(float newyaw);
        void SetAvatarRotation(const Quaternion &newrot);
        void SetCameraYawPitch(float newyaw, float newpitch);

        //! Sets visibility for all name display overlays, used e.g. in screenshot taking
        //! Only functions if #define EC_HoveringText_ENABLED is present. Otherwise performs no function.
        void SetAllTextOverlaysVisible(bool visible);

        //!Checks if ray hits an infoicon billboard, normal rayquery ignores billboards. Only functions if #define EC_HoveringWidget_ENABLED was present
        //! when RexLogicModules was built. Otherwise is a no-op that returns false immediately.
        /*! \param x screen coordinate
            \param y screen coordinate
            \param entity this is the entity that was hit with normal raycast. 
            \return returns true if infoicon was hit, false otherwise
        */
        bool CheckInfoIconIntersection(int x, int y, Foundation::RaycastResult *result);

        //! Launch estateownermessage event
        void EmitIncomingEstateOwnerMessageEvent(QVariantList params);

        ObjectCameraControllerPtr GetObjectCameraController() { return obj_camera_controller_; }
        CameraControlPtr GetCameraControlWidget() { return camera_control_widget_; }

    public slots:
        //! logout from server and delete current scene
        void LogoutAndDeleteWorld();

        /// Sends RexPrimData of a prim entity to server
        ///\todo Move to WorldStream?
        void SendRexPrimData(uint entityid);

    signals:
        //! Estate Info event
        void OnIncomingEstateOwnerMessage(QVariantList params);

    private:
        Q_DISABLE_COPY(RexLogicModule);

        //! Handle real-time update of scene objects
        /*! Performs dead-reckoning and damped motion for all scene entities which have an Placeable and a NetworkPosition
            component. If the Placeable position/rotation is set anywhere else, it will be overridden by the next
            call to this, so it should be avoided.

            Performs animation update to all objects that have an AnimationController component.
         */
        void UpdateObjects(f64 frametime);

        //! Update sound listener position. Only functions if #define EC_SoundListener_ENABLED was present when RexLogicModule was built,
        //! otherwise is a no-op.
        /*! Uses the default camera or avatar for now.
         */
        void UpdateSoundListener();

        //! Handle a resource event. Needs to be passed to several receivers (Prim, Terrain etc.)
        bool HandleResourceEvent(event_id_t event_id, IEventData* data);

        //! Gets a map of all avatars in world and the distance from users avatar,
        //! for updating the name tag fades after certain distance.
        void UpdateAvatarNameTags(Scene::EntityPtr users_avatar);

        //! login through console
        Console::CommandResult ConsoleLogin(const StringVector &params);

        //! logout through console
        Console::CommandResult ConsoleLogout(const StringVector &params);

        //! toggle fly mode through console
        Console::CommandResult ConsoleToggleFlyMode(const StringVector &params);

        //! Console command for test EC_Highlight. Adds EC_Highlight for every avatar.
        Console::CommandResult ConsoleHighlightTest(const StringVector &params);

        /// Returns Ogre renderer pointer. Convenience function for making code cleaner.
        OgreRenderer::RendererPtr GetOgreRendererPtr() const;

        //! Type name of the module.
        static std::string type_name_static_;

        //! Event handler for network events.
        NetworkEventHandler *network_handler_;

        //! Event handler for network events.
        InputEventHandler *input_handler_; 

        //! Event handler for network state events.
        NetworkStateEventHandler *network_state_handler_;

        //! Event handler for scene events.
        SceneEventHandler *scene_handler_;

        //! event handler for framework events
        FrameworkEventHandler *framework_handler_;

        //! event handler for avatar events
        AvatarEventHandler *avatar_event_handler_;

        //! Server connection
        WorldStreamPtr world_stream_;

        //! Movement damping constant
        float movement_damping_constant_;

        //! How long to keep doing dead reckoning
        f64 dead_reckoning_time_;

        typedef boost::function<bool(event_id_t,IEventData*)> LogicEventHandlerFunction;
        typedef std::vector<LogicEventHandlerFunction> EventHandlerVector;
        typedef std::map<event_category_id_t, EventHandlerVector> LogicEventHandlerMap;

        //! Event handler map.
        LogicEventHandlerMap event_handlers_;

        //! Primitive handler pointer.
        PrimitivePtr primitive_;

        //! Current camera entity
        Scene::EntityWeakPtr camera_entity_;

#ifdef _DEBUG
        /// Checks the currently active camera that its transformation is correct, and logs into debug output
        /// if not. Thanks go to Ogre for having issues with this..
        void DebugSanityCheckOgreCameraTransform();
#endif

        //! Mapping for full uuids - localids
        typedef std::map<RexUUID, entity_id_t> IDMap;
        IDMap UUIDs_;

        //! pending parent object assignments, keyed by parent id
        //! the parent id will be added as a key if it is not missing, and the children id's are added to the set.
        //! once the parent prim appears, the children will be assigned the parent and the key will be removed from here.
        typedef std::map<entity_id_t, std::set<entity_id_t> > ObjectParentMap;
        ObjectParentMap pending_parents_;

        //! Camera controllable
        CameraControllablePtr camera_controllable_;

        ObjectCameraControllerPtr obj_camera_controller_;

        CameraControlPtr camera_control_widget_;

        //! Avatar entities found this frame. Needed so that we can update name overlays last, after all other updates
        std::vector<Scene::EntityWeakPtr> found_avatars_;

        //! The input context that responds to avatar-related input and moves the avatar accordingly.
        boost::shared_ptr<RexMovementInput> avatarInput;

        //! current camera state
        CameraState camera_state_;

        //! MainPanel handler
        MainPanelHandler *main_panel_handler_;

        InWorldChatProviderPtr in_world_chat_provider_;

        //! Login service.
        boost::shared_ptr<LoginHandler> login_service_;

        //! List of possible sound listeners (entitities which have EC_SoundListener).
        QList<Scene::Entity *> soundListeners_;

        //! Currently active sound listener.
        Scene::EntityWeakPtr activeSoundListener_;

        //! Creates a new camera entity to the scene. Marked as private since this function has some side-effects, like adding the camera
        //! as the default current camera.
        void CreateOpenSimViewerCamera(Scene::ScenePtr scene);

        NotificationWidgetPtr notification_widget_;

    private slots:
        /** Called when new component is added to the active scene.
         *  Currently used for handling sound listener EC's.
         *  @param entity Entity for which the component was added.
         *  @param component The added component.
         */
        void NewComponentAdded(Scene::Entity *entity, IComponent *component);

        /** Called when component is removed from the active scene.
         *  Currently used for handling sound listener EC's.
         *  @param entity Entity from which the component was removed.
         *  @param component The removed component.
         */
        void ComponentRemoved(Scene::Entity *entity, IComponent *component);

        /// Finds entity with active sound listener component and stores it.
        void FindActiveListener();
    };
}

#endif
