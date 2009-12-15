// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   EnvironmentModule.h
 *  @brief  Blaa blaa
 */

#ifndef incl_EnvironmentModule_h
#define incl_EnvironmentModule_h

#include <Foundation.h>
#include "ModuleInterface.h"
#include "EnvironmentModuleApi.h"
#include "EventDataInterface.h"
#include "WorldStream.h"

namespace Environment
{
    class Terrain;
    class Water;
    class Environment;
    class Sky;
    class EnvironmentEditor;

    typedef boost::shared_ptr<Terrain> TerrainPtr;
    typedef boost::shared_ptr<Water> WaterPtr;
    typedef boost::shared_ptr<EnvironmentEditor> EnvironmentEditorPtr;
    typedef boost::shared_ptr<Sky> SkyPtr;
    typedef boost::shared_ptr<Environment> EnvironmentPtr;

    /// Inteface for environment module.
    class ENVIRONMENT_MODULE_API EnvironmentModule : public Foundation::ModuleInterfaceImpl
    {
    public:
        /// Default constructor.
        EnvironmentModule();

        /// Default destructor 
        virtual ~EnvironmentModule();

        /// Owerrides from ModuleInterface
        void Load();
        void Unload();
        void Initialize();
        void PreInitialize();
        void PostInitialize();
        void Uninitialize();
        void SubscribeToNetworkEvents();
        void Update(Core::f64 frametime);
        bool HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data);

        bool HandleNetworkEvent(Foundation::EventDataInterface* data);
        bool HandleNetworkStateEvent(Foundation::EventDataInterface* data);
        bool HandleOSNE_RegionHandshake(ProtocolUtilities::NetworkEventInboundData* data);

        /// @return The terrain handler object that manages reX terrain logic.
        TerrainPtr GetTerrainHandler();

        /// @return The environment editor.
        EnvironmentEditorPtr GetEnvironmentEditor();

        /// @return The environment handler.
        EnvironmentPtr GetEnvironmentHandler();

        /// @return The sky handler.
        SkyPtr GetSkyHandler();

        /// @return The water handler.
        WaterPtr GetWaterHandler();

        void SendTextureDetailMessage(const RexTypes::RexAssetID &new_texture_id, Core::uint texture_index);

        /// Change new terrain height value,
        /// @Param start_height what height texture starts at (meters).
        /// @Param height_range how many meters texture will go up from the texture_start_height 
        /// @Param corner what texture height we want to change.
        void SendTextureHeightMessage(Core::Real start_height, Core::Real height_range, Core::uint corner);

        /*! Modify land message Used to send a to server.
         *  @param x coordinate of terrain texture.
         *  @param y coordinate of terrain texture.
         *  @param brush brush size OpenSim supports following brush sizes (small = 0, medium = 1 and large = 2)
         *  @param action what terrain modification operation is used. OpenSim supports following actions (Flatten = 0, Raise = 1, Lower = 2, Smooth = 3, Roughen = 4 and Revert = 5)
         *  @param seconds how long has the modify land operation been executed.
         *  @param previous height value for spesific texture coordinate
         */
        void SendModifyLandMessage(Core::f32 x, Core::f32 y, Core::u8 brush, Core::u8 action, Core::Real seconds, Core::Real height);

        /// Creates a new scene and sets that as active. Also creates the core entities to that scene that 
        /// are always to be present in an reX world, like terrain.
        //Scene::ScenePtr CreateNewActiveScene(const std::string &name);

        /// The scene system can store multiple scenes. Only one scene is active at a time, that is the one
        /// that is currently being rendered. You may pass a null pointer to erase the currently active scene.
        //void SetCurrentActiveScene(Scene::ScenePtr scene);

        //! @return The currently viewed scene, or 0 if not connected. (Don't use as an indicator of connection state!)
        //Scene::ScenePtr GetCurrentActiveScene();

        MODULE_LOGGING_FUNCTIONS

        /// Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        /// Returns type of this module. Needed for logging.
        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Environment;

    private:
        /// No copying is allowed.
        EnvironmentModule(const EnvironmentModule &);
        void operator=(const EnvironmentModule &);

        /// Create the terrain.
        void CreateTerrain();

        /// Create the water.
        void CreateWater();

        /// Create the environment.
        void CreateEnvironment();

        /// Create the sky
        void CreateSky();

        /// Event manager pointer.
        Foundation::EventManagerPtr event_manager_;

        /// Id for Framework event category
        Core::event_category_id_t framework_evet_category_;

        /// Id for Resource event category
        Core::event_category_id_t resource_event_category_;

        /// Id for Scene event category
        Core::event_category_id_t scene_event_category_;

        /// Id for NetworkIn event category
        Core::event_category_id_t network_in_event_category_;

        /// Id for NetworkState event category
        Core::event_category_id_t network_state_event_category_;

        /// Terrain geometry ptr.
        TerrainPtr terrain_;

        /// Water ptr.
        WaterPtr water_;

        /// Environment ptr.
        EnvironmentPtr environment_;

        /// Sky ptr.
        SkyPtr sky_;

        /// Terrain editor pointer.
        EnvironmentEditorPtr environment_editor_;

        ProtocolUtilities::WorldStreamPtr currentWorldStream_;

        bool waiting_for_reqioninfomessage_;

        /// Active scene ptr
        //Scene::ScenePtr activeScene_;
    };
}

#endif
