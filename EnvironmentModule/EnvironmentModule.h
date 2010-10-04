/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *  @file   EnvironmentModule.h
 *  @brief  Environment module. Environment module is be responsible for visual environment features like terrain, sky & water.
 */

#ifndef incl_EnvironmentModule_EnvironmentModule_h
#define incl_EnvironmentModule_EnvironmentModule_h

#include "EnvironmentModuleApi.h"
#include "IModule.h"
#include "ModuleLoggingFunctions.h"
#include "WorldStream.h"

namespace Environment
{
    class Terrain;
    class Water;
    class Environment;
    class Sky;
    class EnvironmentEditor;
    class PostProcessWidget;

    typedef boost::shared_ptr<Terrain> TerrainPtr;
    typedef boost::shared_ptr<Water> WaterPtr;
    typedef boost::shared_ptr<Sky> SkyPtr;
    typedef boost::shared_ptr<Environment> EnvironmentPtr;

    //! Environment Module.
    /*! \defgroup EnvironmentModuleClient EnvironmentModule Client interface.

        Inteface for environment module. Environment module implements terrain, sky and water generation.
        Also module will handle all environment editing via EnvironmentEditor. Module receives network
        messages and send them forward to other components that will need them.
    */
    class ENVIRONMENT_MODULE_API EnvironmentModule : public IModule
    {
    public:
        //! Constructor.
        EnvironmentModule();

        //! Destructor 
        virtual ~EnvironmentModule();

        void Load();
        void Initialize();
        void PostInitialize();
        void Uninitialize();
        void Update(f64 frametime);
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

        //! Handles resouce event category.
        //! @param event_id event id
        //! @param data event data pointer
        //! @return Should return true if the event was handled and is not to be propagated further
        bool HandleResouceEvent(event_id_t event_id, IEventData* data);

        //! Handles framework event category.
        //! @param event_id event id
        //! @param data event data pointer
        //! @return Should return true if the event was handled and is not to be propagated further
        bool HandleFrameworkEvent(event_id_t event_id, IEventData* data);

        //! Handles resouce event category.
        //! @param event_id event id
        //! @param data event data pointer
        //! @return Should return true if the event was handled and is not to be propagated further
        bool HandleNetworkEvent(event_id_t event_id, IEventData* data);

        //! Handles input event category.
        //! @param event_id event id
        //! @param data event data pointer
        //! @return Should return true if the event was handled and is not to be propagated further
        bool HandleInputEvent(event_id_t event_id, IEventData* data);

        //! Get terrain texture ids, terrain height and water height values.
        //! @param network data pointer.
        //! @return Should return true if the event was handled and is not to be propagated further
        bool HandleOSNE_RegionHandshake(ProtocolUtilities::NetworkEventInboundData* data);

        //! @return The terrain handler object that manages reX terrain logic.
        TerrainPtr GetTerrainHandler() const;

        //! @return The environment handler.
        EnvironmentPtr GetEnvironmentHandler() const;

        //! @return The sky handler.
        SkyPtr GetSkyHandler() const;

        //! @return The water handler.
        WaterPtr GetWaterHandler() const;

        //! Sends new terrain texture into the server.
        //! @param new_texture_id texture asset id that we want to use.
        //! @param texture_index switch texture we want to change range [0 - 3].
        void SendTextureDetailMessage(const RexTypes::RexAssetID &new_texture_id, uint texture_index);

        //! Change new terrain height value,
        //! @Param start_height what height texture starts at (meters).
        //! @Param height_range how many meters texture will go up from the texture_start_height 
        //! @Param corner what texture height we want to change.
        //! @todo Seems like older OpenSim server (tested 0.6.5) won't inform all the users that terrain heights has been updated,
        //! if needed add custom code that will Request a new ReqionInfo message from the server. 
        //! That message contain all the infomation releated to environment like terrain and water.
        void SendTextureHeightMessage(float start_height, float height_range, uint corner);

        /*! Sends modify land message into the server.
         *  @param x coordinate of terrain texture.
         *  @param y coordinate of terrain texture.
         *  @param brush brush size OpenSim supports following brush sizes (small = 0, medium = 1 and large = 2)
         *  @param action what terrain modification operation is used. OpenSim supports following actions: 
         *          Flatten = 0, Raise = 1, Lower = 2, Smooth = 3, Roughen = 4 and Revert = 5)
         *  @param seconds how long has the modify land operation been executed.
         *  @param previous height value for spesific texture coordinate
         */
        void SendModifyLandMessage(f32 x, f32 y, u8 brush, u8 action, float seconds, float height);

        /**
         * Creates a local enviroment entity. This is called if there does not exist outside enviroment entity (entity, which has EC_NAME component which attribute name is entity_name + Enviroment example WaterEnviroment).
         * IF there exist a that kind entity it will be used to syncronize water, fog etc. things to other clients/server. 
         * @param entity_name is entity which exist in world.
         * @param component_name is name of component which is added into local enviroment entity.
         */
        Scene::EntityPtr CreateEnvironmentEntity(const QString& entity_name, const QString& component_name);
        /** 
         * Removes local dump enviroment entity. 
         */ 
        void RemoveLocalEnvironment();

        MODULE_LOGGING_FUNCTIONS

        //! @return Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return type_name_static_; }



    private:
        EnvironmentModule(const EnvironmentModule &);
        void operator=(const EnvironmentModule &);

        //! @return Returns type of this module. Needed for logging.
        static std::string type_name_static_;

        //! Create the terrain.
        void CreateTerrain();

        //! Create the water.
        void CreateWater();

        //! Create the environment.
        void CreateEnvironment();

        //! Create the sky
        void CreateSky();

        void ReleaseTerrain();
        void ReleaseWater();
        void ReleaseEnvironment();
        void ReleaseSky();

        //! Event manager pointer.
        Foundation::EventManagerPtr event_manager_;

        //! Id for Framework event category
        event_category_id_t framework_event_category_;

        //! Id for Resource event category
        event_category_id_t resource_event_category_;

        //! Id for Scene event category
        event_category_id_t scene_event_category_;

        //! Id for NetworkIn event category
        event_category_id_t network_in_event_category_;

        //! Id for NetworkState event category
        event_category_id_t network_state_event_category_;

        //! Id for Input event category
        event_category_id_t input_event_category_;

        //! Id for Tundra event category
        event_category_id_t tundra_event_category_;

        //! Terrain geometry ptr.
        TerrainPtr terrain_;

        //! Water ptr.
        WaterPtr water_;

        //! Environment ptr.
        EnvironmentPtr environment_;

        //! Sky ptr.
        SkyPtr sky_;

        //! Terrain editor pointer.
        EnvironmentEditor *environment_editor_;

        //! PostProcess dialog pointer
        PostProcessWidget *postprocess_dialog_;

        //! WorldStream will handle those network messages that we are wishing to send.
        ProtocolUtilities::WorldStreamPtr currentWorldStream_;

        //! Wait for new terrain heightmap information.
        bool waiting_for_regioninfomessage_;

        bool firstTime_;
    };
}

#endif
