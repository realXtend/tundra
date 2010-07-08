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

#include "ModuleInterface.h"
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

namespace Foundation
{
    struct RaycastResult;
}

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

typedef boost::shared_ptr<ProtocolUtilities::WorldStream> WorldStreamPtr;

namespace RexLogic
{
    class RexMovementInput;
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
    class WorldInputLogic;
    namespace InWorldChat
    {
        class Provider;
    }
    typedef boost::shared_ptr<InWorldChat::Provider> InWorldChatProviderPtr;

    typedef boost::shared_ptr<Avatar> AvatarPtr;
    typedef boost::shared_ptr<AvatarEditor> AvatarEditorPtr;
    typedef boost::shared_ptr<Primitive> PrimitivePtr;
    typedef boost::shared_ptr<AvatarControllable> AvatarControllablePtr;
    typedef boost::shared_ptr<CameraControllable> CameraControllablePtr;

    //! Camera states handled by rex logic
    enum CameraState
    {
        //! Camera follows the avatar (third or first person)
        CS_Follow,
        //! Camera moves around freely
        CS_Free,
        //! Camera tripod
        CS_Tripod,
        //! Focus on object
        CS_FocusOnObject
    };

    class REXLOGIC_MODULE_API RexLogicModule : public QObject, public Foundation::ModuleInterface, public Foundation::WorldLogicInterface
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
        virtual bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);
        static const std::string &NameStatic() { return type_name_static_; }
        MODULE_LOGGING_FUNCTIONS;

        //=============== WorldLogicInterface API ===============/
        Scene::EntityPtr GetUserAvatarEntity() const;
        Scene::EntityPtr GetCameraEntity() const;
        Scene::EntityPtr GetEntityWithComponent(uint entity_id, const std::string &component) const;

        //=============== RexLogicModule API ===============/

        //! Returns world stream pointer.
        WorldStreamPtr GetServerConnection() const { return world_stream_; }

        //! switch current input controller, if using avatar controller, switch to camera controller and vice versa
        void SwitchCameraState();

        //! camera tripod
        void CameraTripod();

        //! focus on object, and rotate around it
        void FocusOnObject(float, float, float);

        //! return camera state
        CameraState GetCameraState() { return camera_state_; }

        //! reset camera state to CS_Follow
        void ResetCameraState();

        //! @return The avatar handler object that manages reX avatar logic.
        AvatarPtr GetAvatarHandler() const;

        //! @return The avatar editor.
        AvatarEditorPtr GetAvatarEditor() const;

        //! @return The primitive handler object that manages reX primitive logic.
        PrimitivePtr GetPrimitiveHandler() const;

        //! Returns the camera controllable
        CameraControllablePtr GetCameraControllable() const { return camera_controllable_; }

        //! Returns the avatar controllable
        AvatarControllablePtr GetAvatarControllable() const { return avatar_controllable_; }

        //! The scene system can store multiple scenes. Only one scene is active at a time, that is the one
        //! that is currently being rendered. You may pass a null pointer to erase the currently active scene.
        void SetCurrentActiveScene(Scene::ScenePtr scene);

        //! @return The currently viewed scene, or 0 if not connected. (Don't use as an indicator of connection state!)
        Scene::ScenePtr GetCurrentActiveScene() const;

        //! Creates a new scene and sets that as active. Also creates the core entities to that scene that 
        //! are always to be present in an reX world, like terrain.
        Scene::ScenePtr CreateNewActiveScene(const std::string &name);

        //! Deletes the scene with the given name. If that was the current active scene, the active scene will be
        //! set to null.
        void DeleteScene(const std::string &name);

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
        void SetAvatarYaw(Real newyaw);
        void SetAvatarRotation(const Quaternion &newrot);
        void SetCameraYawPitch(Real newyaw, Real newpitch);

        ///\todo Remove. Get this information using WorldStream and/or EC_OpenSimPresence.
        entity_id_t GetUserAvatarId() const;
        ///\todo Remove. Get this information from other modules using EC_OgrePlaceable and/or EC_OgreCamera.
        Vector3df GetCameraUp() const;
        ///\todo Remove. Get this information from other modules using EC_OgrePlaceable and/or EC_OgreCamera.
        Vector3df GetCameraRight() const;
        ///\todo Remove. Get this information from other modules using EC_OgrePlaceable and/or EC_OgreCamera.
        Vector3df GetCameraPosition() const;
        ///\todo Remove. Get this information from other modules using EC_OgrePlaceable and/or EC_OgreCamera.
        Quaternion GetCameraOrientation() const;
        ///\todo Remove. Get this information from other modules using EC_OgreCamera and/or Renderer.
        Real GetCameraViewportWidth() const;
        ///\todo Remove. Get this information from other modules using EC_OgreCamera and/or Renderer.
        Real GetCameraViewportHeight() const;
        ///\todo Remove. Get this information from other modules using EC_OgreCamera.
        Real GetCameraFOV() const;

        //! Sets visibility for all name display overlays, used e.g. in screenshot taking
        void SetAllTextOverlaysVisible(bool visible);

        //! Handles a click event for entity, namely showing the name tag
        void EntityClicked(Scene::Entity* entity);

        //!Checks if ray hits an infoicon billboard, normal rayquery ignores billboards.
        /*! \param x screen coordinate
            \param y screen coordinate
            \param entity this is the entity that was hit with normal raycast. 
            \return returns true if infoicon was hit, false otherwise
        */
        bool CheckInfoIconIntersection(int x, int y, Foundation::RaycastResult *result);

        //! Launch estateownermessage event
        void EmitIncomingEstateOwnerMessageEvent(QVariantList params);

    public slots:
        //! logout from server and delete current scene
        void LogoutAndDeleteWorld();

        //! called when entity is hovered over with mouse
        void EntityHovered(Scene::Entity* entity);

        /// Sends RexPrimData of a prim entity to server
        ///\todo Move to WorldStream?
        void SendRexPrimData(uint entityid);

        /// Returns Ogre renderer pointer. Convenience function for making code cleaner.
        OgreRenderer::RendererPtr GetOgreRendererPtr() const;

    signals:
        //! Estate Info event
        void OnIncomingEstateOwnerMessage(QVariantList params);

    private:
        Q_DISABLE_COPY(RexLogicModule);

        //! Handle real-time update of scene objects
        /*! Performs dead-reckoning and damped motion for all scene entities which have an OgrePlaceable and a NetworkPosition
            component. If the OgrePlaceable position/rotation is set anywhere else, it will be overridden by the next
            call to this, so it should be avoided.

            Performs animation update to all objects that have an OgreAnimationController component.
         */
        void UpdateObjects(f64 frametime);

        //! Update avatar overlays. Must be done after other object update
        void UpdateAvatarOverlays();
        
        //! Update sound listener position
        /*! Uses current camera for now
         */
        void UpdateSoundListener();

        //! Handle a resource event. Needs to be passed to several receivers (Prim, Terrain etc.)
        bool HandleResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data);

        //! Handle an inventory event.
        bool HandleInventoryEvent(event_id_t event_id, Foundation::EventDataInterface* data);

        //! Handle an asset event.
        bool HandleAssetEvent(event_id_t event_id, Foundation::EventDataInterface* data);

        //! Does preparations before logout/delete of scene
        //! For example: Takes ui screenshots of world/avatar with rendering service.
        //! Add functionality if you need something done before logout.
        void AboutToDeleteWorld();

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

        //! Server connection
        WorldStreamPtr world_stream_;

        //! Movement damping constant
        Real movement_damping_constant_;

        //! How long to keep doing dead reckoning
        f64 dead_reckoning_time_;

        typedef boost::function<bool(event_id_t,Foundation::EventDataInterface*)> LogicEventHandlerFunction;
        typedef std::vector<LogicEventHandlerFunction> EventHandlerVector;
        typedef std::map<event_category_id_t, EventHandlerVector> LogicEventHandlerMap;

        //! Event handler map.
        LogicEventHandlerMap event_handlers_;

        //! Avatar handler pointer.
        AvatarPtr avatar_;

        //! Avatar editor pointer.
        AvatarEditorPtr avatar_editor_;

        //! Primitive handler pointer.
        PrimitivePtr primitive_;

        //! Active scene pointer.
        Scene::ScenePtr activeScene_;

        //! Current camera entity
        Scene::EntityWeakPtr camera_entity_;

#ifdef _DEBUG
        /// Checks the currently active camera that its transformation is correct, and logs into debug output
        /// if not. Thanks go to Ogre for having issues with this..
        void DebugSanityCheckOgreCameraTransform();
#endif

        //! workaround for not being able to send events during initialization
        bool send_input_state_;

        //! Mapping for full uuids - localids
        typedef std::map<RexUUID, entity_id_t> IDMap;
        IDMap UUIDs_;

        //! pending parent object assignments, keyed by parent id
        //! the parent id will be added as a key if it is not missing, and the children id's are added to the set.
        //! once the parent prim appears, the children will be assigned the parent and the key will be removed from here.
        typedef std::map<entity_id_t, std::set<entity_id_t> > ObjectParentMap;
        ObjectParentMap pending_parents_;

        //! An avatar controllable
        AvatarControllablePtr avatar_controllable_;

        //! Camera controllable
        CameraControllablePtr camera_controllable_;

        //! Avatar entities found this frame. Needed so that we can update name overlays last, after all other updates
        std::vector<Scene::EntityWeakPtr> found_avatars_;

        //! The input context that responds to avatar-related input and moves the avatar accordingly.
        boost::shared_ptr<RexMovementInput> avatarInput;

        //! current camera state
        CameraState camera_state_;

        //! OpenSim login handler
        OpenSimLoginHandler *os_login_handler_;

        //! Taiga login handler
        TaigaLoginHandler *taiga_login_handler_;

        //! MainPanel handler
        MainPanelHandler *main_panel_handler_;

        InWorldChatProviderPtr in_world_chat_provider_;
    };
}

#endif
