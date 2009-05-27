/// @file Environment.h
/// @brief Manages environment-related reX-logic, e.g. world time and lightning.
/// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicModule_Environment_h
#define incl_RexLogicModule_Environment_h

namespace RexLogic
{

class Environment
{
public:
    Environment(RexLogicModule *owner);
    ~Environment();
    
    /// Sets the entity pointer for this environment entity.
    /// @param entity Entity pointer.
    void SetEntity(Scene::EntityPtr entity) { entity_ = entity; }
    
    /// Handles the OpenSim "SimulatorViewerTimeMessage" packet.
    /// @param data The network event data pointer.
    bool HandleOSNE_SimulatorViewerTimeMessage(OpenSimProtocol::NetworkEventInboundData* data);
    
private:
    /// Creates the global sunlight.
    void CreateGlobalLight();
    
    /// Pointer to the RexLogicModule which owns this class.
    RexLogicModule *owner_;
    
    /// Entity pointer of the environment.
    Scene::EntityPtr entity_;
    
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
