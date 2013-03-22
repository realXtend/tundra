/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   EntityIdChangeTracker.h
    @brief  EntityIdChangeTracker helper class stores entity id changes that result from QUndoCommand manipulations. */

#pragma once

#include "SceneFwd.h"
#include "CoreTypes.h"
#include "AttributeChangeType.h"

#include <QObject>
#include <QMap>

/// Stores entity id changes that result from QUndoCommand manipulations.
class EntityIdChangeTracker : public QObject
{
    Q_OBJECT

public:
    /// Constructor
    /* @param scene Scene of which entities we're tracking.
       @param parent The parent object to this instance */
    EntityIdChangeTracker(const ScenePtr &scene, QObject * parent = 0);

    /// Appends an unacked ID to unackedToAckedIds_ map
    /* @param oldId The ID to be appended. Could be an unacked ID or local
       @note If oldId is local, then oldId will be stored as both the key and value to the map
             If oldId is unacked, then oldId will be stored as map[oldId] = 0 */
    void AppendUnackedId(entity_id_t oldId);

    /// Stores the previous entity ID as the key and a new ID that an entity is about to be (re)created with
    /* @param oldId The old / previous entity ID 
       @param newId The new / current entity ID */
    void TrackId(entity_id_t oldId, entity_id_t newId);

    /// Retrieve the current ID from a previous one, if a change took place at all.
    /* @param oldId The old / previous entity ID
       @note May return zero if an entity is not yet acked
       @returns The current entity ID */
    entity_id_t RetrieveId(entity_id_t oldId);

    /// Recursively search through changedIds_ map to find the most recent entity ID. Used by 'entity_id_t RetrieveId(entity_id_t)'
    /* @param id The ID to be traced 
       @returns The current entity ID, or 'id' if there are no registered changes */
    entity_id_t TraceId(entity_id_t id);

    /// Clears the maps without prompt
    void Clear();

    QMap<entity_id_t, entity_id_t> unackedToAckedIds_; ///< 'Unacked ID to Acked ID' map
    QMap<entity_id_t, entity_id_t> changedIds_; ///< 'Previous entity ID to current entity ID' map
    SceneWeakPtr scene_; ///< A weak pointer to the main scene

private slots:
    /// Listens to EntityCreated signal of the main camera scene
    // void OnEntityCreated(Entity * entity, AttributeChange::Type change);

    /// Listens to EnittyRemoved signal of the main camera scene
    // void OnEntityRemoved(Entity * entity, AttributeChange::Type change);

    /// Listens to EntityAcked signal of the main camera scene, and makes changes in the map for the corresponding unacked entity IDs
    void OnEntityAcked(Entity * entity, entity_id_t oldId);
};
