// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Scene_EntityReference_h
#define incl_Scene_EntityReference_h

#include "SceneFwd.h"

#include <QString>
#include <QMetaType>

/// Represents a reference to an entity, either by name or ID This structure can be used as a parameter type to an EC attribute.
struct EntityReference
{
    EntityReference() {}
    
    EntityReference(const QString &entityName) : ref(entityName.trimmed()) {}

    EntityReference(entity_id_t id) : ref(QString::number(id)) {}

    /// Set from an entity. If the name is unique within its parent scene, the name will be set, otherwise ID.
    void Set(Entity* entity);
    
    /// Lookup an entity from the scene according to the ref. Return null pointer if not found
    EntityPtr Lookup(SceneManager* scene) const;
    
    /// Return whether the ref does not refer to an entity
    bool IsEmpty() const;

    bool operator ==(const EntityReference &rhs) const { return this->ref == rhs.ref; }

    bool operator !=(const EntityReference &rhs) const { return !(*this == rhs); }

    bool operator <(const EntityReference &rhs) const { return ref < rhs.ref; }

    /// The entity pointed to. This can be either an entity ID, or an entity name
    QString ref;
};

Q_DECLARE_METATYPE(EntityReference)

#endif

