/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *  @file   EnvironmentModule.h
 *  @brief  Environment module. Environment module is be responsible for visual environment features like terrain, sky & water.
 */

#ifndef incl_EnvironmentModule_EnvironmentModule_h
#define incl_EnvironmentModule_EnvironmentModule_h

#include "EnvironmentModuleApi.h"
#include "IModule.h"

#include "SceneFwd.h"

#ifdef CAELUM
namespace Caelum
{
    class CaelumSystem;
}
#endif

namespace Environment
{
    class Terrain;
    class Water;
    class Environment;
    class Sky;
    class EnvironmentEditor;
    class TerrainWeightEditor;

    typedef boost::shared_ptr<Terrain> TerrainPtr;
    typedef boost::shared_ptr<Water> WaterPtr;
    typedef boost::shared_ptr<Sky> SkyPtr;
    typedef boost::shared_ptr<Environment> EnvironmentPtr;

    /// Environment Module.
    /** \defgroup EnvironmentModuleClient EnvironmentModule Client interface.

        Interface for environment module. Environment module implements terrain, sky and water generation.
        Also module will handle all environment editing via EnvironmentEditor. Module receives network
        messages and send them forward to other components that will need them.
    */
    class ENVIRONMENT_MODULE_API EnvironmentModule : public QObject, public IModule
    {
        Q_OBJECT

    public:
        /// Constructor.
        EnvironmentModule();

        /// Destructor 
        virtual ~EnvironmentModule();

        void Load();
        void Initialize();
        void PostInitialize();
        void Uninitialize();
        void Update(f64 frametime);
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

        /// Creates a local environment entity.
        /** This is called if there does not exist outside environment entity (entity, which has EC_NAME component which attribute
            name is entity_name + Environment, f.ex. WaterEnviroment).
            If there exist that kind of entity it will be used to synchronize water, fog etc. things to other clients/server. 
            @param entity_name is entity which exist in world.
            @param component_name is name of component which is added into local environment entity.
        */
        EntityPtr CreateEnvironmentEntity(const QString& entity_name, const QString& component_name);

        /// Removes local dump environment entity. 
        void RemoveLocalEnvironment();

        /// @return Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return type_name_static_; }

#ifdef CAELUM
        Caelum::CaelumSystem* GetCaelum();
#endif

    public slots:
        /// Shows terrain texture editor.
        void ShowTerrainWeightEditor();

    private slots:
        /// Handles the recreation of environment (relevant to Tundra only) when scene is cleared.
        /// @todo Hopefully this will only be a temporary solution.
        void OnSceneCleared(Scene::SceneManager* scene);

    private:
        Q_DISABLE_COPY(EnvironmentModule);

        /// @return Returns type of this module. Needed for logging.
        static std::string type_name_static_;

        /// Create the terrain.
        void CreateTerrain();

        /// Create the water.
        void CreateWater();

        /// Create the environment.
        void CreateEnvironment();

        /// Create the sky
        void CreateSky();

        void ReleaseTerrain();
        void ReleaseWater();
        void ReleaseEnvironment();
        void ReleaseSky();

        /// Editor for terrain texture weights
        TerrainWeightEditor* terrainWeightEditor_;

        /// Event manager pointer.
        EventManagerPtr event_manager_;

        /// Id for Tundra event category
        event_category_id_t tundra_event_category_;

        /// Water ptr.
        WaterPtr water_;

        /// Environment ptr.
        EnvironmentPtr environment_;

        /// Sky ptr.
        SkyPtr sky_;
    };
}

#endif
