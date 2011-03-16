/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   WorldLogicInterface.h
 *  @brief  
 */

#ifndef incl_Interfaces_WorldLogicInterface_h
#define incl_Interfaces_WorldLogicInterface_h

#include "IService.h"
#include "ForwardDefines.h"
#include "SceneFwd.h"

#include <QObject>

class QString;

namespace Foundation
{
    class WorldLogicInterface : public QObject, public IService
    {
        Q_OBJECT

    public slots:
        /// @note Deprecated in Tundra.
        virtual Scene::Entity* GetUserAvatarEntityRaw() const 
        {
            return GetUserAvatarEntity().get();
        }

        /// @note Deprecated in Tundra.
        virtual Scene::Entity* GetCameraEntityRaw() const 
        {
            return GetCameraEntity().get();
        }

    public:
        /// Default constructor.
        WorldLogicInterface() {}

        /// Destructor.
        virtual ~WorldLogicInterface() {}

        /// Returns user's avatar entity.
        /// @note Deprecated in Tundra.
        virtual Scene::EntityPtr GetUserAvatarEntity() const = 0;

        /// Returns currently active camera entity.
        /// @note Deprecated in Tundra.
        virtual Scene::EntityPtr GetCameraEntity() const = 0;

        /// Returns entity with certain entity component in it or null if not found.
        /// @param entity_id Entity ID.
        /// @param component Type name of the component.
        virtual Scene::EntityPtr GetEntityWithComponent(uint entity_id, const QString &component) const = 0;

        /// Hack function for getting EC_AvatarAppearance info to UiModule
        virtual const QString &GetAvatarAppearanceProperty(const QString &name) const = 0;

        // Hack function to get camera pitch into AvatarModule, can be removed when made better
        virtual float GetCameraControllablePitch() const = 0;

    signals:
        /// Emitted just before we start to delete world (scene).
        void AboutToDeleteWorld();
    };
}

#endif
