// For conditions of distribution and use, see copyright notice in license.txt

//! @file Water.h
//! @brief Manages Water-related environment module.

#ifndef incl_Water_h
#define incl_Water_h

#include "EnvironmentModuleApi.h"
#include "ForwardDefines.h"

#include <QObject>

namespace Environment
{
    class EnvironmentModule;
    //class EC_Water;
    class EC_WaterPlane;

    //! \ingroup EnvironmentModuleClient.
    class ENVIRONMENT_MODULE_API Water : public QObject
    {
        Q_OBJECT 

        public:
            //! Constructor.
            //! @param owner EnvironmentModule pointer.
            Water(EnvironmentModule *owner);

            //! Destructor.
            virtual ~Water();

           
            //! @return The scene entity that represents the water in the currently active world.
            Scene::EntityWeakPtr GetWaterEntity();

            /**
             * Creates water geometry, uses a given value to water height. This implementation assumes that 
             * there can be only one water in scene. @todo Change implementation to support multiwaters. 
             * @param height is a water height for newly created water plane.
             *
             **/
            
            void CreateWaterGeometry(float height = 20.f);

            /** 
             * Removes water geometry totally. 
             */
            void RemoveWaterGeometry();

            //! @return The scene water height.
            //! @note If error occuers returns 0.0
            float GetWaterHeight() const;

        public slots:

            //! Sets a new water height to scene water.
            //! height New water height.
            void SetWaterHeight(float height);

        signals:
            //! Emited when water height has been changed.
            //! height New water height.
            void HeightChanged(double height);

            //! Emited when water has been removed
            void WaterRemoved();

            //! Emitted when water has been created
            void WaterCreated();

        private:

            //! Looks through all the entities in RexLogic's currently active scene to find the Water
            //! entity. Caches it internally. Use GetWaterEntity to obtain it afterwards.
            Scene::EntityWeakPtr GetActiveWater();

            //! EnvironmentModule pointer.
            EnvironmentModule *owner_;

            //! Water EC pointer.
            EC_WaterPlane* activeWaterComponent_;

            //! Cached water entity pointer.
            Scene::EntityWeakPtr activeWaterEntity_;
            
    };
}

#endif
