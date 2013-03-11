// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IComponent.h"
#include "CoreDefines.h"
#include "Math/float3.h"
#include "PhysicsModuleFwd.h"

#include <map>

/// Physics volume trigger component
/** <table class="header">
    <tr>
    <td>
    <h2>VolumeTrigger</h2>
    Physics volume trigger component

    Registered by Physics::PhysicsModule.

    <b>Attributes</b>:
    <ul>
    <li>bool: byPivot
    <div>@copydoc byPivot</div>
    <li>QVariantList: entities
    <div>@copydoc entities</div>
    </ul>

    <b>Exposes the following scriptable functions:</b>
    <ul>
    <li> GetEntitiesInside: @copydoc GetEntitiesInside
    <li> "GetNumEntitiesInside": @copydoc GetNumEntitiesInside
    <li> "GetEntityInside": @copydoc GetEntityInside
    <li> "GetEntityInsidePercent": @copydoc GetEntityInsidePercent
    <li> "GetEntityInsidePercentByName": @copydoc GetEntityInsidePercentByName
    <li> "IsInterestingEntity": @copydoc IsInterestingEntity
    <li> "IsPivotInside": @copydoc IsPivotInside
    <li> "IsInsideVolume":@copydoc IsInsideVolume
    </ul>

    <b>Reacts on the following actions:</b>
    <ul>
    <li> None.
    </ul>
    </td>
    </tr>

    Does not emit any actions.

    <b>Depends on the component RigitBody.</b>.

    @note If you use 'byPivot' -option or use IsPivotInside-function, the pivot point shouldn't be outside the mesh 
        (or physics collision primitive) because physics collisions are used for efficiency even in this case.
    @todo If you add an entity to the 'interesting entities list', no signals may get send for that entity,
          and it may not show up in any list of entities contained in this volume trigger until that entity moves.
          Also if you enable/disable 'byPivot' option when entities are inside the volume, no signals may get send for those entities,
          and they may not show up in any list of entities contained in this volume trigger until the entities move.

    </table> */
class EC_VolumeTrigger : public IComponent
{
    friend class Physics::PhysicsWorld;
    
    Q_OBJECT
    COMPONENT_NAME("EC_VolumeTrigger", 24)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_VolumeTrigger(Scene* scene);

    virtual ~EC_VolumeTrigger();

    /// Pivot trigger flag.
    /** If false (default), triggers by entity volume. If true, triggers by entity pivot point (ie. entity pivot points enters/leaves the volume). */
    Q_PROPERTY(bool byPivot READ getbyPivot WRITE setbyPivot)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, byPivot)

    /// List of interesting entities by name.
    /** Events are dispatched only for entities in this list, other entities are ignored. Leave empty to get events for all entities in the scene */
    Q_PROPERTY(QVariantList entities READ getentities WRITE setentities);
    DEFINE_QPROPERTY_ATTRIBUTE(QVariantList, entities);

signals:
    /// Note: needs to be lowercase for QML to accept connections to it
    /// @todo Make signature uppercase, QML support is deprecated. */
    void entityEnter(Entity* entity/*, const float3& position*/);
    /// Note: needs to be lowercase for QML to accept connections to it
    /// @todo Make signature uppercase, QML support is deprecated. */
    void entityLeave(Entity* entity/*, const float3& position*/);

public slots:
    /// Get a list of entities currently residing inside the volume.
    /** @note Return value is invalidated by physics update.
        @return list of entities */
    QList<EntityWeakPtr> GetEntitiesInside() const;

    /// Returns number of entities inside this volume trigger. 
    /** Use with GetEntityInside() to get all entities inside this volume.
        @note Return value is invalidated by physics update.
        @return Number of entities inside this volume */
    int GetNumEntitiesInside() const;

    /// Gets entity that is inside this volume trigger with specified index.
    /** Use with GetNumEntitiesInside() to get all entities inside this volume.
        @note Use together with GetNumEntitiesInside() during the same physics
              update frame, because physics update may change the number of 
              entities inside the volume. */
    Entity* GetEntityInside(int idx) const;

    /// Returns a list of entities by name which currently reside inside the volume.
    /** @note Return value is invalidated by physics update.
        @return list of entity names */
    QStringList GetEntityNamesInside() const;

    /// Returns an approximate percent of how much of the entity is inside this volume, [0,1]
    /** If entity is not inside this volume at all, returns 0, if entity is completely inside this volume, returns 1.
        @note Uses axis aligned bounding boxes for calculations, so it is not accurate.
        @note Return value is invalidated by physics update.

        @param entity entity
        @return approximated percent of how much of the entity is inside this volume */
    float GetEntityInsidePercent(const Entity* entity) const;

    /// Returns an approximate percent of how much of the entity is inside this volume, [0,1]
    /** If entity is not inside this volume at all, returns 0, if entity is completely inside this volume, returns 1.
        @note Uses axis aligned bounding boxes for calculations, so it is not accurate.
        @note Return value is invalidated by physics update.
        @param name entity name
        @return approximated percent of how much of the entity is inside this volume */
    float GetEntityInsidePercentByName(const QString &name) const;

    /// Returns true if specified entity can be found in the 'interesting entities' list
    /** If list of entities for this volume trigger is empty, returns always true for any entity name
        (even non-existing ones)
        @param name entity name
        @return true if events are triggered for the names entity, false otherwise */
    bool IsInterestingEntity(const QString &name) const;

    /// Returns true if the pivot point of the specified entity is inside this volume trigger
    /** @note Return value is invalidated by physics update.
        @return true if the pivot point of the specified entity is inside the volume, false otherwise */
    bool IsPivotInside(Entity *entity) const;

    /// Returns true if given world coordinate point is inside volume. 
    bool IsInsideVolume(const float3& point) const;

private slots:

    void UpdateSignals();

    /// Check for rigid body component and connect to its signal
    void CheckForRigidBody();

    /// Collisions have been processed for the scene the parent entity is in
    void OnPhysicsUpdate();

    /// Called when physics collisions occurs.
    void OnPhysicsCollision(Entity* otherEntity, const float3& position, const float3& normal, float distance, float impulse, bool newCollision);

    /// Called when entity inside this volume is removed from the scene
    void OnEntityRemoved(Entity* entity);

private:
    /// Called when some of the attributes has been changed.
    void AttributesChanged();

    /// Rigid body component that is needed for collision signals
    weak_ptr<EC_RigidBody> rigidbody_;

    /// As C++ standard weak_ptr doesn't provide less than operator (or any comparison operators for that matter), we need to provide it ourselves.
    struct EntityWeakPtrLessThan
    {
        bool operator() (const EntityWeakPtr &a, const EntityWeakPtr &b) const { return WEAK_PTR_LESS_THAN(a, b); }
    };
    typedef std::map<EntityWeakPtr, bool, EntityWeakPtrLessThan> EntitiesWithinVolumeMap;
    /// Map of entities inside this volume. 
    /** The value is used in physics update to see if the entity is still inside
        this volume or if it left the volume during last physics update. */
    EntitiesWithinVolumeMap entities_;
};
