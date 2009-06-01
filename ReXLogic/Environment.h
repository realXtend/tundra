/// @file Environment.h
/// @brief Manages environment-related reX-logic, e.g. world time and lightning.
/// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicModule_Environment_h
#define incl_RexLogicModule_Environment_h

namespace Ogre
{
    class Camera;
}

namespace RexLogic
{
    class Environment
    {
    public:
        /// Default constructor.
        /// @param owner The owner module.
        Environment(RexLogicModule *owner);
        
        /// Default destructor.
        virtual ~Environment();

        /// Looks through all the entities in RexLogic's currently active scene to find the Water
        /// entity. Caches it internally. Use GetWaterEntity to obtain it afterwards.
        void FindCurrentlyActiveEnvironment();
        
        /// @return The scene entity that represents the terrain in the currently active world.        
        Scene::EntityWeakPtr Environment::GetEnvironmentEntity();
        
        /// Creates the environment EC.
        void CreateEnvironment();
        
        /// Handles the "SimulatorViewerTimeMessage" packet.
        /// @param data The network event data pointer.
        bool HandleOSNE_SimulatorViewerTimeMessage(OpenSimProtocol::NetworkEventInboundData* data);
        
        /// Updates the visual effects (fog, skybox etc).
        void UpdateVisualEffects();
        
    private:
        /// Creates the global sunlight.
        void CreateGlobalLight();
        
        /// Weak pointer to the entity which has the environment component.
        Scene::EntityWeakPtr cachedEnvironmentEntity_;
        
        /// Pointer to the RexLogicModule which owns this class.
        RexLogicModule *owner_;
        
        /// Server's perception of time (UNIX EPOCH).
        time_t usecSinceStart_;
        
        /// Unknown.
        uint32_t secPerDay_;
        
        /// Unknown.
        uint32_t secPerYear_;
        
        /// Direction of the sunlight.
        Vector3 sunDirection_;
        
        /// Unknown.
        float sunPhase_;
        
        /// Sun's angle velocity.
        Vector3 sunAngVelocity_;
    };
}

#endif
