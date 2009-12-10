// For conditions of distribution and use, see copyright notice in license.txt

/// @file Water.h
/// @brief Manages Water-related Rex logic.

#ifndef incl_Water_h
#define incl_Water_h

#include "EnvironmentModuleApi.h"
#include <Foundation.h>

#include <QObject>

namespace Environment
{
    class EnvironmentModule;
    class EC_Water;

    class ENVIRONMENT_MODULE_API Water : public QObject
    {
        Q_OBJECT 

        public:
            /// Constructor.
            /// @param owner EnvironmentModule pointer.
            Water(EnvironmentModule *owner);

            /// Destructor.
            virtual ~Water();

            //! Looks through all the entities in RexLogic's currently active scene to find the Water
            //! entity. Caches it internally. Use GetWaterEntity to obtain it afterwards.
            void FindCurrentlyActiveWater();

            //! @return The scene entity that represents the terrain in the currently active world.
            Scene::EntityWeakPtr GetWaterEntity();

            //! Creates water geometry, uses a default value to water height
            void CreateWaterGeometry();

            //! @return The scene water height.
            //! @note If error occuers returns -1.0
            float GetWaterHeight() const;

        public slots:

            //! Sets a new water height to scene water.
            //! height New water height.
            void SetWaterHeight(float height);

        signals:
            //! Emited when water height has been changed.
            //! height New water height.
            void HeightChanged(double height);

        private:
            /// EnvironmentModule pointer.
            EnvironmentModule *owner_;

            /// Water EC pointer.
            EC_Water* waterComponent_;

            /// Cached water entity pointer.
            Scene::EntityWeakPtr cachedWaterEntity_;
    };
}

#endif
