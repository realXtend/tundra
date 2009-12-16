/// @file Environment.h
/// @brief Manages environment-related reX-logic, e.g. world time and lightning.
/// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicModule_Environment_h
#define incl_RexLogicModule_Environment_h

#include <Foundation.h>
#include <RexTypes.h>
#include <QObject>
#include "EnvironmentModuleApi.h"

namespace ProtocolUtilities
{
    class NetworkEventInboundData;
}


namespace OgreRenderer
{
    class EC_OgreEnvironment;
}


namespace Environment
{
    class EnvironmentModule;

    class ENVIRONMENT_MODULE_API Environment : public QObject
    {
        Q_OBJECT

    public:
        
        /** 
         * Overloaded constructor.
         * @param owner The owner module.
         **/
        Environment(EnvironmentModule *owner);
        virtual ~Environment();

        /**
         * @return The scene entity that represents the environment in the currently active world.        
         **/
        Scene::EntityWeakPtr GetEnvironmentEntity();

        /**
         * Creates the environment EC to current active scene and adjust it using default parameters.
         **/
        void CreateEnvironment();

        /**
         * Handles the "SimulatorViewerTimeMessage" packet.
         * @param data The network event data pointer.
         **/
        bool DecodeSimulatorViewerTimeMessage(ProtocolUtilities::NetworkEventInboundData* data);

        /** 
         * Sets a water fog for current active environment.
         * @param fogStart distance in world unit at which linear fog start ot encroach. 
         * @param fogEnd distance in world units at which linear fog becomes completely opaque.
         * @param color the colour of the fog. 
         **/
        void SetWaterFog(float fogStart, float fogEnd, const QVector<float>& color);
 
        /** 
         * Sets a ground fog for current active environment.
         * @param fogStart distance in world unit at which linear fog start ot encroach. 
         * @param fogEnd distance in world units at which linear fog becomes completely opaque.
         * @param color the colour of the fog. 
         **/

        void SetGroundFog(float fogStart, float fogEnd, const QVector<float>& color);

        /**
         * Enables or disables fog color override. 
         * @param enabled boolean defines state of override.
         **/
         
        void SetFogColorOverride(bool enabled);
        
        /**
         * Returns information is fog color controlled by user or caelum.
         * @return true if it is fog color is controlled by user, else false.
         **/
        
        bool GetFogColorOverride();


        /**
         * Updates the visual effects (fog, skybox etc).
         **/
        void Update(Core::f64 frametime);

        /**
         * @return true if caelum library is used.
         **/
        bool IsCaelum();

        //! Set server time override
        void SetTimeOverride(bool enabled) { time_override_ = enabled; }      
     
    signals:
        
         void WaterFogAdjusted(float fogStart, float fogEnd, const QVector<float>& color);
         void GroundFogAdjusted(float fogStart, float fogEnd, const QVector<float>& color);

    private:
       
        /// Looks through all the entities in RexLogic's currently active scene to find the Water
        /// entity. Caches it internally. Use GetWaterEntity to obtain it afterwards.
        Scene::EntityWeakPtr FindActiveEnvironment();

        /// Creates the global sunlight.
        void CreateGlobalLight();

        /// Pointer to the environment module which owns this class.
        EnvironmentModule *owner_;
      
        OgreRenderer::EC_OgreEnvironment* activeEnvComponent_;

        /// Weak pointer to the entity which has the environment component.
        Scene::EntityWeakPtr activeEnvEntity_;

        /// Time override, default false
        bool time_override_;

        /// Server's perception of time (UNIX EPOCH).
        time_t usecSinceStart_;

        /// Unknown/not needed. \todo delete?
        uint32_t secPerDay_;

        /// Unknown/not needed. \todo delete?
        uint32_t secPerYear_;

        /// Direction of the sunlight.
        RexTypes::Vector3 sunDirection_;

        /// Unknown/not needed. \todo delete?
        float sunPhase_;

        /// Sun's angle velocity.
        RexTypes::Vector3 sunAngVelocity_;
    };
}

#endif
