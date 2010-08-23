/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   WorldLogicInterface.h
 *  @brief  
 */

#include "ServiceInterface.h"

#ifndef incl_Interfaces_WorldLogicInterface_h
#define incl_Interfaces_WorldLogicInterface_h

#include "ForwardDefines.h"
 #include <QString>

namespace Foundation
{
    class WorldLogicInterface : public ServiceInterface
    {
    public:
        /// Default constructor.
        WorldLogicInterface() {}

        /// Destructor.
        virtual ~WorldLogicInterface() {}

        /// Returns user's avatar entity.
        virtual Scene::EntityPtr GetUserAvatarEntity() const = 0;

        /// Returns currently active camera entity.
        virtual Scene::EntityPtr GetCameraEntity() const = 0;

        /// Returns entity with certain entity component in it or null if not found.
        /// @param entity_id Entity ID.
        /// @param component Type name of the component.
        virtual Scene::EntityPtr GetEntityWithComponent(uint entity_id, const QString &component) const = 0;

        // 
    };
}

#endif
