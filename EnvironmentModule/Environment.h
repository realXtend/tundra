/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   Environment.cpp
 *  @brief  Manages environment-related reX-logic, e.g. world time and lighting.
 */

#ifndef incl_EnvironmentModule_Environment_h
#define incl_EnvironmentModule_Environment_h

#include "EnvironmentModuleApi.h"

#include <QVector>
#include <QObject>

#include "EC_EnvironmentLight.h"

class EC_OgreEnvironment;

namespace Environment
{
    class EnvironmentModule;
    class EC_EnvironmentLight;
    class EC_Fog;

    /// Environment contain information about water, fog and lighting. Also Caelum implementation code is included in this class.
    /// \ingroup EnvironmentModuleClient.
    class ENVIRONMENT_MODULE_API Environment : public QObject
    {
        Q_OBJECT

    public:
        /// Overloaded constructor.
        /** @param owner The owner module.
        */
        explicit Environment(EnvironmentModule *owner);
        virtual ~Environment();

    public:
        /**
         * @return The scene entity that represents the environment in the currently active world.        
         **/
        Scene::EntityWeakPtr GetEnvironmentEntity();

        /**
         * @return Enviroment entity component, or 0 if does not exist
         **/
        EC_OgreEnvironment* GetEnvironmentComponent();

        EC_EnvironmentLight* GetEnvironmentLight();

        /**
         * Creates the environment EC to current active scene and adjust it using default parameters.
         **/
        void CreateEnvironment();

        /// Updates the visual effects (fog, skybox etc).
        void Update(f64 frametime);

        /// @return true if caelum library is used.
        bool IsCaelum();

        /// Converts string vector to QVector.
        template<typename T> QVector<T> ConvertToQVector(const StringVector& vector) 
        {
            int elements = vector.size();
            QVector<T> vec(elements);
            try
            {
                for(int i = 0; i < elements; ++i)
                {
                    std::istringstream stream(vector[i]);
                    stream >> vec[i];
                }
            }
            catch(...)
            {
                return QVector<T>(0);
            }

            return vec;
        }

    private:

        /// Pointer to the environment module which owns this class.
        EnvironmentModule *owner_;

        /// Weak pointer to the entity which has the environment component.
        Scene::EntityWeakPtr activeEnvEntity_;

        /// Bit mask of Caelum components we use.
        int caelumComponents_;
    };
}

#endif
