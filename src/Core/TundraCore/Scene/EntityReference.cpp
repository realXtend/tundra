// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EntityReference.h"
#include "Entity.h"
#include "Scene/Scene.h"
#include "EC_Name.h"
#include "SceneAPI.h"

#include "Framework.h"
#include "IComponent.h"
#include "CoreStringUtils.h"
#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

void EntityReference::Set(EntityPtr entity)
{
    Set(entity.get());
}

void EntityReference::Set(Entity* entity)
{
    if (!entity)
    {
        ref.clear();
        return;
    }
    
    QString name = entity->Name();
    Scene* scene = entity->ParentScene();
    if (!scene)
    {
        // If entity is not in scene, set directly by ID
        ref = QString::number(entity->Id());
        return;
    }
    if (scene->IsUniqueName(name))
        ref = name;
    else
        ref = QString::number(entity->Id());
}

bool EntityReference::IsEmpty() const
{
    bool ok = false;
    entity_id_t id = ref.toInt(&ok);
    // ID 0 can not be an entity
    if ((ok) && (id == 0))
        return true;
    if (ref.trimmed().isEmpty())
        return true;
    
    return false;
}

EntityPtr EntityReference::Lookup(Scene* scene) const
{    
    if (!scene || ref.isEmpty())
        return EntityPtr();
    // If ref looks like an ID, lookup by ID first
    bool ok = false;
    entity_id_t id = ref.toInt(&ok);
    if (ok)
    {
        EntityPtr entity = scene->GetEntity(id);
        if (entity)
            return entity;
    }
    // Then get by name
    return scene->GetEntityByName(ref.trimmed());
}
