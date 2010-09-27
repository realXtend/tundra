// For conditions of distribution and use, see copyright notice in license.txt

//! @file Water.h
//! @brief Manages Water-related environment module.

#ifndef incl_Water_h
#define incl_Water_h

#include "EnvironmentModuleApi.h"
#include "ForwardDefines.h"
#include <AttributeChangeType.h>
#include <QObject>

namespace Environment
{
    class EnvironmentModule;
    class EC_WaterPlane;

    /**
     * Water class is used to control default water plane, this is depricated way to control environment water. 
     * This class handles all environment editor changes, and deals RexDrawWater-message information from server. 
     */

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
             * Creates water geometry, uses a given value to water height. 
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
        
             /** 
              * Sets a water fog for default water plane.
              * @param fogStart distance in world unit at which linear fog start ot encroach. 
              * @param fogEnd distance in world units at which linear fog becomes completely opaque.
              * @param color the colour of the fog. 
              **/
            
            void SetWaterFog(float fogStart, float fogEnd, const QVector<float>& color);
 
             /**
              * Set new water fog color.
              * @param new color value.
              **/
            
            void SetWaterFogColor(const QVector<float>& color);

             /**
              * Set new underwater fog distance.
              * @param fogStart start distance from the viewpoint.
              * @param fogEnd end distance from the viewpoint.
              **/
            
            void SetWaterFogDistance(float fogStart, float fogEnd);

             /**
              * @return underwater fog start distance. 
              */
            float GetWaterFogStartDistance() const;

            /**
             * @return underwater fog end distance. 
             */
            float GetWaterFogEndDistance() const;

             /** 
              * Returns current fog water color.
              **/
            QVector<float> GetFogWaterColor() const;


        public slots:

            //! Sets a new water height to scene water.
            //! height New water height.
            void SetWaterHeight(float height, AttributeChange::Type type = AttributeChange::Network);

        signals:
            //! Emited when water height has been changed.
            //! height New water height.
            void HeightChanged(double height);

            //! Emited when water has been removed
            void WaterRemoved();

            //! Emitted when water has been created
            void WaterCreated();

            //! Emitted when water fog is adjusted.
            void WaterFogAdjusted(float fogStart, float fogEnd, const QVector<float>& color);


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
