// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicModule_h
#define incl_RexLogicModule_h

#include <Foundation.h>
#include "ModuleInterface.h"
#include "RexLogicModuleApi.h"

#include <WorldStream.h>

namespace CoreUi
{
    class LoginContainer;
}

namespace RexLogic
{
    class NetworkEventHandler;
    class InputEventHandler;
    class SceneEventHandler;
    class NetworkStateEventHandler;
    class FrameworkEventHandler;
    class Avatar;
    class AvatarEditor;
    class Primitive;
    class AvatarControllable;
    class CameraControllable;
    class OpenSimLoginHandler;
    class TaigaLoginHandler;
    class MainPanelHandler;

    typedef boost::shared_ptr<ProtocolUtilities::WorldStream> WorldStreamConnectionPtr;
    typedef boost::shared_ptr<Avatar> AvatarPtr;
    typedef boost::shared_ptr<AvatarEditor> AvatarEditorPtr;
    typedef boost::shared_ptr<Primitive> PrimitivePtr;
    typedef boost::shared_ptr<AvatarControllable> AvatarControllablePtr;
    typedef boost::shared_ptr<CameraControllable> CameraControllablePtr;
    typedef boost::shared_ptr<ProtocolUtilities::InventorySkeleton> InventoryPtr;

    //! Camera states handled by rex logic
    enum CameraState
    {
        //! Camera follows the avatar (third or first person)
        CS_Follow,
        //! Camera moves around freely
        CS_Free
    };

    //! interface for modules
    class REXLOGIC_MODULE_API RexLogicModule : public Foundation::ModuleInterfaceImpl
    {
    public:
        RexLogicModule();
        virtual ~RexLogicModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        virtual void SubscribeToNetworkEvents(boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> currentProtocolModule);
        virtual void Update(Core::f64 frametime);

        virtual bool HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data);

        MODULE_LOGGING_FUNCTIONS;

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_WorldLogic;

        WorldStreamConnectionPtr GetServerConnection() const { return world_stream_; }

        //! switch current input controller, if using avatar controller, switch to camera controller and vice versa
        void SwitchCameraState();

        //! @return The avatar handler object that manages reX avatar logic.
        AvatarPtr GetAvatarHandler();

        //! @return The avatar editor.
        AvatarEditorPtr GetAvatarEditor();

        //! @return The primitive handler object that manages reX primitive logic.
        PrimitivePtr GetPrimitiveHandler();

        /// @return Invetory pointer.
        InventoryPtr GetInventory() const;

        /// @return Login UI
        CoreUi::LoginContainer *GetLogin() { return login_ui_; }

        //! Returns the camera controllable
        CameraControllablePtr GetCameraControllable() { return camera_controllable_; }

        //! Returns the avatar controllable
        AvatarControllablePtr GetAvatarControllable() { return avatar_controllable_; }

        //! The scene system can store multiple scenes. Only one scene is active at a time, that is the one
        //! that is currently being rendered. You may pass a null pointer to erase the currently active scene.
        void SetCurrentActiveScene(Scene::ScenePtr scene);

        //! @return The currently viewed scene, or 0 if not connected. (Don't use as an indicator of connection state!)
        Scene::ScenePtr GetCurrentActiveScene();

        //! Creates a new scene and sets that as active. Also creates the core entities to that scene that 
        //! are always to be present in an reX world, like terrain.
        Scene::ScenePtr CreateNewActiveScene(const std::string &name);

        //! Deletes the scene with the given name. If that was the current active scene, the active scene will be
        //! set to null.
        void DeleteScene(const std::string &name);

        //! @return The entity corresponding to given scene entityid, or null if not found. 
        //!         This entity is guaranteed to have an existing EC_OpenSimPrim component.
        __inline Scene::EntityPtr GetPrimEntity(Core::entity_id_t entityid) { return GetEntityWithComponent(entityid,"EC_OpenSimPrim"); }
        Scene::EntityPtr GetPrimEntity(const RexUUID &fullid);

        //! @return The entity corresponding to given scene entityid, or null if not found. 
        //!         This entity is guaranteed to have an existing EC_OpenSimAvatar component,
        //!         and EC_OpenSimPresence component.
        __inline Scene::EntityPtr GetAvatarEntity(Core::entity_id_t entityid) { return GetEntityWithComponent(entityid,"EC_OpenSimAvatar"); }
        Scene::EntityPtr GetAvatarEntity(const RexUUID &fullid); 

        //! Register uuid - localid pair
        void RegisterFullId(const RexTypes::RexUUID &fullid, Core::entity_id_t entityid);

        //! Unregister uuid
        void UnregisterFullId(const RexTypes::RexUUID &fullid);

        //! Handle a resource event. Needs to be passed to several receivers (Prim, Terrain etc.)
        bool HandleResourceEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);

        //! Handle an inventory event.
        bool HandleInventoryEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);

        //! Handle an asset event.
        bool HandleAssetEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);

        //! Handle real-time update of scene objects
        /*! Performs dead-reckoning and damped motion for all scene entities which have an OgrePlaceable and a NetworkPosition
            component. If the OgrePlaceable position/rotation is set anywhere else, it will be overridden by the next
            call to this, so it should be avoided.

            Performs animation update to all objects that have an OgreAnimationController component.
         */
        void UpdateObjects(Core::f64 frametime);

        //! login through console
        Console::CommandResult ConsoleLogin(const Core::StringVector &params);

        //! logout through console
        Console::CommandResult ConsoleLogout(const Core::StringVector &params);

        //! login from py - temp while loginui misses dllexport
        void StartLoginOpensim(QString qfirstAndLast, QString qpassword, QString qserverAddressWithPort);

        //! toggle fly mode through console
        Console::CommandResult ConsoleToggleFlyMode(const Core::StringVector &params);

        //! logout from server and delete current scene
        void LogoutAndDeleteWorld();

        //! Update sound listener position
        /*! Uses current camera for now
         */
        void UpdateSoundListener();

        //! XXX have linking probs to AvatarController so trying this wrapper
        void SetAvatarYaw(Core::Real newyaw);
        void SetAvatarRotation(Core::Quaternion newrot);
        void SetCameraYawPitch(Core::Real newyaw, Core::Real newpitch);

        Core::entity_id_t GetUserAvatarId();

        Core::Vector3df GetCameraUp();
        Core::Vector3df GetCameraRight();
        Core::Vector3df GetCameraPosition();
        Core::Real GetCameraViewportWidth();
        Core::Real GetCameraViewportHeight();

        Core::Real GetCameraFOV();

        void SendRexPrimData(Core::entity_id_t entityid);

        void SendModifyLandMessage(Core::f32 x, Core::f32 y, Core::u8 brush, Core::u8 action, Core::Real seconds, Core::Real height);

    private:
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

        //! Server connection
        WorldStreamConnectionPtr world_stream_;

        //! Movement damping constant
        Core::Real movement_damping_constant_;

        //! How long to keep doing dead reckoning
        Core::f64 dead_reckoning_time_;

        typedef boost::function<bool(Core::event_id_t,Foundation::EventDataInterface*)> LogicEventHandlerFunction;
        typedef std::vector<LogicEventHandlerFunction> EventHandlerVector;
        typedef std::map<Core::event_category_id_t, EventHandlerVector> LogicEventHandlerMap;

        //! Event handler map.
        LogicEventHandlerMap event_handlers_;

        //! Avatar pointer.
        AvatarPtr avatar_;

        //! Avatar editor pointer.
        AvatarEditorPtr avatar_editor_;

        //! Primitive pointer.
        PrimitivePtr primitive_;

        //! Active scene pointer.
        Scene::ScenePtr activeScene_;

#ifdef _DEBUG
        /// Checks the currently active camera that its transformation is correct, and logs into debug output
        /// if not. Thanks go to Ogre for having issues with this..
        void DebugSanityCheckOgreCameraTransform();
#endif

        //! workaround for not being able to send events during initialization
        bool send_input_state_;

        bool login_failed_showed_;

        //! Get a component with certain entitycomponent in it
        Scene::EntityPtr GetEntityWithComponent(Core::entity_id_t entityid, const std::string &requiredcomponent);

        //! Mapping for full uuids - localids
        typedef std::map<RexUUID, Core::entity_id_t> IDMap;
        IDMap UUIDs_;

        //! The connection state which is shown in the login window.
        ProtocolUtilities::Connection::State connectionState_;

        //! An avatar controllable
        AvatarControllablePtr avatar_controllable_;

        //! Camera controllable
        CameraControllablePtr camera_controllable_;

        //! current camera state
        CameraState camera_state_;

        //! Pointer to the login window
        CoreUi::LoginContainer *login_ui_;

        //! OpenSim login handler
        OpenSimLoginHandler *os_login_handler_;

        //! Taiga login handler
        TaigaLoginHandler *taiga_login_handler_;

        //! MainPanel handler
        MainPanelHandler *main_panel_handler_;
    };
}

#endif
