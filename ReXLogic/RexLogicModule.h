// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicModule_h
#define incl_RexLogicModule_h

#include "ModuleInterface.h"
#include "RexServerConnection.h"
#include "AvatarController.h"

namespace RexLogic
{
    class NetworkEventHandler;
    class InputEventHandler;
    class SceneEventHandler;
    class NetworkStateEventHandler;
    class CameraController;
    class Terrain;
    class Avatar;    class Primitive;    class Sky;    class Water;    class RexLoginWindow;    typedef boost::shared_ptr<RexServerConnection> RexServerConnectionPtr;
    typedef boost::shared_ptr<AvatarController> AvatarControllerPtr;
    typedef boost::shared_ptr<CameraController> CameraControllerPtr;

    typedef boost::weak_ptr<Terrain> TerrainWeakPtr;
    typedef boost::shared_ptr<Terrain> TerrainPtr;
    typedef boost::shared_ptr<Water> WaterPtr;
    typedef boost::shared_ptr<Avatar> AvatarPtr;
    typedef boost::shared_ptr<Primitive> PrimitivePtr;
    typedef boost::shared_ptr<Sky> SkyPtr;

    enum InputController
    {
        Controller_Avatar,
        Controller_Camera
    };

    //! interface for modules
    class MODULE_API RexLogicModule : public Foundation::ModuleInterfaceImpl
    {
    public:
        RexLogicModule();
        virtual ~RexLogicModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();

        virtual void Update(Core::f64 frametime);
        
        virtual bool HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data);
        
        MODULE_LOGGING_FUNCTIONS;

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_WorldLogic;
        
        RexServerConnectionPtr GetServerConnection() const { return rexserver_connection_; }

        AvatarControllerPtr GetAvatarController() const { return avatar_controller_; }

        //! switch current input controller, if using avatar controller, switch to camera controller and vice versa
        void SwitchController();

        //! @return The logic object that manages the water-related parts of the scene.
        WaterPtr GetWaterHandler();
        
        //! @return The terrain handler object that manages reX terrain logic.
        TerrainPtr GetTerrainHandler();

        //! @return The avatar handler object that manages reX avatar logic.
        AvatarPtr GetAvatarHandler();

        //! @return The primitive handler object that manages reX primitive logic.        
        PrimitivePtr GetPrimitiveHandler();
        
        //! @return The sky handler object that manages reX sky logic.
        SkyPtr GetSkyHandler();
        
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
        //!         This entity is guaranteed to have an existing EC_OpenSimAvatar component.
        __inline Scene::EntityPtr GetAvatarEntity(Core::entity_id_t entityid) { return GetEntityWithComponent(entityid,"EC_OpenSimAvatar"); }
        Scene::EntityPtr GetAvatarEntity(const RexUUID &fullid); 

        //! Register uuid - localid pair
        void RegisterFullId(const RexTypes::RexUUID &fullid, Core::entity_id_t entityid);
        
        //! Unregister uuid
        void UnregisterFullId(const RexTypes::RexUUID &fullid);

        //! Handle a resource event. Needs to be passed to several receivers (Prim, Terrain etc.)
        bool HandleResourceEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);
        
    private:
        //! Event handler for network events.
        NetworkEventHandler *network_handler_;
        
        //! Event handler for network events.
        InputEventHandler *input_handler_; 
        
        //! Event handler for network state events.
        NetworkStateEventHandler *network_state_handler_;
        
        //! Event handler for scene events.
        SceneEventHandler *scene_handler_;
        
        //! Server connection
        RexServerConnectionPtr rexserver_connection_;

        //! Local avatar controller for this client
        AvatarControllerPtr avatar_controller_;

        //! (local) camera controller for this client
        CameraControllerPtr camera_controller_;

        typedef boost::function<bool(Core::event_id_t,Foundation::EventDataInterface*)> LogicEventHandlerFunction;
        typedef std::map<Core::event_category_id_t, LogicEventHandlerFunction> LogicEventHandlerMap;
        
        LogicEventHandlerMap event_handlers_;

        //! current input controller
        InputController current_controller_;

        WaterPtr water_;

        TerrainPtr terrain_;
        
        AvatarPtr avatar_;
        
        PrimitivePtr primitive_;
        
        SkyPtr sky_;

        Scene::ScenePtr activeScene_;

        void CreateWater();

        //! Recreates the terrain. Called at startup.
        void CreateTerrain();
        
        //! Recreates the sky. Called at startup.
        void CreateSky();
        
        //! workaround for not being able to send events during initialization
        bool send_input_state_;

        //! Get a component with certain entitycomponent in it
        Scene::EntityPtr GetEntityWithComponent(Core::entity_id_t entityid, const std::string &requiredcomponent);

        //! Mapping for full uuids - localids
        typedef std::map<RexUUID, Core::entity_id_t> IDMap;
        IDMap UUIDs_;
        
        //! The login window.
        RexLoginWindow *loginWindow_;
        
        //! The connection state which is shown in the login window.
        OpenSimProtocol::Connection::State connectionState_;
    };
}

#endif
