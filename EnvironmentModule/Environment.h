/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   Environment.cpp
 *  @brief  Manages environment-related reX-logic, e.g. world time and lighting.
 */

#ifndef incl_EnvironmentModule_Environment_h
#define incl_EnvironmentModule_Environment_h

#include "EnvironmentModuleApi.h"
#include "ForwardDefines.h"
#include "RexTypes.h"

#include <QVector>
#include <QObject>

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

    //! Environment contain information about water, fog and lighting. Also Caelum implementation code is included in this class.
    //! \ingroup EnvironmentModuleClient.
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

    public:
        /**
         * @return The scene entity that represents the environment in the currently active world.        
         **/
        Scene::EntityWeakPtr GetEnvironmentEntity();

        /**
         * @return Enviroment entity component, or 0 if does not exist
         **/
        OgreRenderer::EC_OgreEnvironment* GetEnvironmentComponent();

        /**
         * Creates the environment EC to current active scene and adjust it using default parameters.
         **/
        void CreateEnvironment();

        /**
         * Handles the "SimulatorViewerTimeMessage" packet.
         * @param data The network event data pointer.
         **/
        bool HandleSimulatorViewerTimeMessage(ProtocolUtilities::NetworkEventInboundData* data);

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
         * Set new ground fog color.
         * @param new color value.
         **/
        void SetGroundFogColor(const QVector<float>& color);

        /**
         * Set new water fog color.
         * @param new color value.
         **/
        void SetWaterFogColor(const QVector<float>& color);

        /**
         * Set new ground fog distance.
         * @param fogStart start distance from the viewpoint.
         * @param fogEnd end distance from the viewpoint.
         **/
        void SetGroundFogDistance(float fogStart, float fogEnd);

        /**
         * Set new underwater fog distance.
         * @param fogStart start distance from the viewpoint.
         * @param fogEnd end distance from the viewpoint.
         **/
        void SetWaterFogDistance(float fogStart, float fogEnd);

        /**
         * @return underwater fog start distance. 
         */
        float GetWaterFogStartDistance();

        /**
         * @return underwater fog end distance. 
         */
        float GetWaterFogEndDistance();

        /**
         * @return ground fog start distance. 
         */
        float GetGroundFogStartDistance();

        /**
         * @return ground fog end distance. 
         */
        float GetGroundFogEndDistance();

        /**
         * Returns current fog ground color. 
         */
        QVector<float> GetFogGroundColor();
        
        /** 
         * Returns current fog water color.
         **/
        QVector<float> GetFogWaterColor();

        /**
         * Updates the visual effects (fog, skybox etc).
         **/
        void Update(f64 frametime);

        /**
         * @return true if caelum library is used.
         **/
        bool IsCaelum();

        //! Set new sunlight direction
        //! @param vector new sun light direction.
        void SetSunDirection(const QVector<float>& vector);

        //! Get sunlight direction
        //! @return sun light direction.
        QVector<float> GetSunDirection();

        //! Set new sunlight color.
        //! @param vector new sunlight color.
        void SetSunColor(const QVector<float>& vector);

        //! Get sunlight color
        //! @return sun light color.
        QVector<float> GetSunColor();

        //! Get ambient light color
        //! @return ambient light color.
        QVector<float> GetAmbientLight();

        //! Set new ambient light color.
        //! @param vector new ambient light color value.
        void SetAmbientLight(const QVector<float>& vector);

        /// Converts string vector to QVector.
        template<typename T> QVector<T> ConvertToQVector(const StringVector& vector) 
        {
            int elements = vector.size();
            QVector<T> vec(elements);
            try
            {
                for ( int i = 0; i < elements; ++i)
                {
                    std::istringstream stream(vector[i]);
                    stream >> vec[i];
                }
            }
            catch (...)
            {
                return QVector<T>(0);
            }

            return vec;
        }

    public slots:
        //! Setter/getter for bool local override of server time
        void SetTimeOverride(bool enabled) { time_override_ = enabled; }
        bool GetTimeOverride() { return time_override_; }

    signals:
        //! Emitted when water fog is adjusted.
        void WaterFogAdjusted(float fogStart, float fogEnd, const QVector<float>& color);

        //! Emitted when ground fog is adjusted.
        void GroundFogAdjusted(float fogStart, float fogEnd, const QVector<float>& color);

    private:
        /// Creates the global sunlight.
        void CreateGlobalLight();

        /// Pointer to the environment module which owns this class.
        EnvironmentModule *owner_;

        /// Weak pointer to the entity which has the environment component.
        Scene::EntityWeakPtr activeEnvEntity_;

        /// Time override, default false
        bool time_override_;

        /// Server's perception of time (UNIX EPOCH). Not used currently.
        time_t usecSinceStart_;

        /// Unknown. Not used currently.
        uint32_t secPerDay_;

        /// Unknown. Not used currently.
        uint32_t secPerYear_;

        /// Direction of the sunlight.
        RexTypes::Vector3 sunDirection_;

        /// Sun phase.
        float sunPhase_;

        /// Sun's angle velocity.
        RexTypes::Vector3 sunAngVelocity_;

    };
}

#endif
