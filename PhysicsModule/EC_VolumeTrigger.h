// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Physics_EC_VolumeTrigger_h
#define incl_Physics_EC_VolumeTrigger_h

#include "IComponent.h"
#include "Declare_EC.h"
#include "Core.h"

//! Physics volume trigger component
/**
<table class="header">
<tr>
<td>
<h2>VolumeTrigger</h2>
Physics volume trigger component

Registered by Physics::PhysicsModule.

<b>Attributes</b>:
<ul>
<li>bool: byPivot
<div>If false (default), triggers by entity volume. If true, triggers by entity pivot point (ie. entity pivot points enters/leaves the volume).</div>
<li>QVariantList: entities
<div>List of interesting entities by name. Events are dispatched only for entities in this list, other entities are ignore. Leave empty to get events for all entities in the scene.</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li> "Activate": force the body to activate (wake up)
<li> "IsActive": return whether body is active
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on the component RigitBody.</b>.

\note If you use 'byPivot' -option, the pivot point shouldn't be outside the mesh because physics collisions are used for efficiency even in this case.
\todo If you add an entity to the 'interesting entities list', no signals may get send for that entity,
      and it may not show up in any list of entities contained in this volume trigger.

</table>
*/

// forward declares
namespace Physics { class PhysicsModule; class PhysicsWorld; }
namespace Scene { class Entity; }
class EC_RigidBody;

class EC_VolumeTrigger : public IComponent
{
    friend class Physics::PhysicsWorld;
    
    Q_OBJECT
    DECLARE_EC(EC_VolumeTrigger)

public:
    //! Pivot trigger flag. If false (default), triggers by entity volume. If true, triggers by entity pivot point (ie. entity pivot points enters/leaves the volume).
    Q_PROPERTY(bool byPivot READ getbyPivot WRITE setbyPivot)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, byPivot)

    //! Key sequence - action name mappings.
    Q_PROPERTY(QVariantList entities READ getentities WRITE setentities);
    DEFINE_QPROPERTY_ATTRIBUTE(QVariantList, entities);

    virtual ~EC_VolumeTrigger();

    //! Set component as serializable.
    virtual bool IsSerializable() const { return true; }
    
    //bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData *data);
signals:
    void EntityEnter(Scene::Entity* entity/*, const Vector3df& position*/);
    void EntityLeave(Scene::Entity* entity/*, const Vector3df& position*/);

public slots:
    //! Get a list of entities currently residing inside the volume.
    /*! \return list of entities
     */
    QList<Scene::EntityWeakPtr> GetEntitiesInside() const;

    //! Returns a list of entities by name which currently reside inside the volume.
    /*! \return list of entity names
     */
    QStringList GetEntityNamesInside() const;

    //! Returns an approximate percent of how much of the entity is inside this volume, [0,1]
    /*! If entity is not inside this volume at all, returns 0, if entity is completely inside this volume, returns 1.
        \note Uses axis aligned bounding boxes for calculations, so it is not accurate.
        \param entity entity
        \return approximated percent of how much of the entity is inside this volume
    */
    float GetEntityInsidePercent(Scene::Entity *entity) const;

    //! Returns an approximate percent of how much of the entity is inside this volume, [0,1]
    /*! If entity is not inside this volume at all, returns 0, if entity is completely inside this volume, returns 1.
        \note Uses axis aligned bounding boxes for calculations, so it is not accurate.
        \param name entity name
        \return approximated percent of how much of the entity is inside this volume
    */
    float GetEntityInsidePercentByName(const QString &name) const;

    //! Returns true if specified entity can be found in the 'interesting entities' list
    bool IsInterestingEntity(const QString &entityName) const;

    //! Returns true if the pivot point of the specified entity is inside this volume trigger
    bool IsPivotInside(Scene::Entity *entity) const;

private slots:
    //! Called when some of the attributes has been changed.
    void AttributeUpdated(IAttribute *attribute);

    void UpdateSignals();

    //! Check for rigid body component and connect to its signal
    void CheckForRigidBody();

    //! Collisions have been processed for the scene the parent entity is in
    void OnPhysicsUpdate();

    //! Called when physics collisions occurs.
    void OnPhysicsCollision(Scene::Entity* otherEntity, const Vector3df& position, const Vector3df& normal, float distance, float impulse, bool newCollision);

    //! Called when entity inside this volume is removed from the scene
    void OnEntityRemoved(Scene::Entity* entity);

private:
    //! constructor
    /*! \param module Physics module
     */
    EC_VolumeTrigger(IModule* module);

    //! Rigid body component that is needed for collision signals
    boost::weak_ptr<EC_RigidBody> rigidbody_;

    //! Set of entities inside this volume
    QMap<Scene::EntityWeakPtr, bool> entities_;

    //! Owner module of this component
    Physics::PhysicsModule *owner_;
};

#endif
