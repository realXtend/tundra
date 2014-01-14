// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#define MATH_OGRE_INTEROP
#define MATH_BULLET_INTEROP
#include "EC_RigidBody.h"
#include "ConvexHull.h"
#include "PhysicsModule.h"
#include "PhysicsUtils.h"
#include "PhysicsWorld.h"

#include "Profiler.h"
#include "OgreMeshAsset.h"
#include "Entity.h"
#include "Scene/Scene.h"
#include "EC_Mesh.h"
#include "EC_Placeable.h"
#include "EC_Terrain.h"
#include "AssetAPI.h"
#include "IAssetTransfer.h"
#include "AttributeMetadata.h"
#include "LoggingFunctions.h"

#include <btBulletDynamicsCommon.h>
#include <LinearMath/btMotionState.h>
#include <BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.h>
#include <set>

#include <OgreSceneNode.h>

using namespace Physics;

static const float cForceThresholdSq = 0.0005f * 0.0005f;
static const float cImpulseThresholdSq = 0.0005f * 0.0005f;
static const float cTorqueThresholdSq = 0.0005f * 0.0005f;

struct EC_RigidBody::Impl : public btMotionState
{
    Impl(EC_RigidBody *rb) :
        body(0),
        world(0),
        shape(0),
        childShape(0),
        heightField(0),
        disconnected(false),
        cachedShapeType(-1),
        cachedSize(float3::zero),
        clientExtrapolating(false),
        rigidBody(rb)
    {
    }

    /// btMotionState override. Called when Bullet wants us to tell the body's initial transform
    void getWorldTransform(btTransform &worldTrans) const
    {
        if (placeable.expired())
            return;

        float3 position = placeable.lock()->WorldPosition();
        Quat orientation = placeable.lock()->WorldOrientation();
    
        worldTrans.setOrigin(position);
        worldTrans.setRotation(orientation);
    }

    /// btMotionState override. Called when Bullet wants to tell us the body's current transform
    void setWorldTransform(const btTransform &worldTrans)
    {
        /// \todo For a large scene, applying the changed transforms of rigid bodies is slow (slower than the physics simulation itself,
        /// or handling collisions) due to the large number of Qt signals being fired.
    
        PROFILE(EC_RigidBody_SetWorldTransform);
    
        // Cannot modify server-authoritative physics object, rather get the transform changes through placeable attributes
        const bool hasAuthority = rigidBody->HasAuthority();
        if (!hasAuthority && !clientExtrapolating)
            return;
    
        if (placeable.expired())
            return;
        shared_ptr<EC_Placeable> p = placeable.lock();
        // Important: disconnect our own response to attribute changes to not create an endless loop!
        disconnected = true;
    
        AttributeChange::Type changeType = hasAuthority ? AttributeChange::Default : AttributeChange::LocalOnly;

        // Set transform
        float3 position = worldTrans.getOrigin();
        Quat orientation = worldTrans.getRotation();
    
        // Non-parented case
        if (p->parentRef.Get().IsEmpty())
        {
            Transform newTrans = p->transform.Get();
            newTrans.SetPos(position.x, position.y, position.z);
            newTrans.SetOrientation(orientation);
            p->transform.Set(newTrans, changeType);
        }
        else
        // The placeable has a parent itself
        {
            if (p->IsAttached())
            {
                Ogre::Node* parent = p->OgreSceneNode()->getParent();
                if (parent)
                {
                    position = parent->convertWorldToLocalPosition(position);
                    orientation = parent->convertWorldToLocalOrientation(orientation);
                    Transform newTrans = p->transform.Get();
                    newTrans.SetPos(position);
                    newTrans.SetOrientation(orientation);
                    p->transform.Set(newTrans, changeType);
                }
            }
        }
        // Set linear & angular velocity
        if (body)
        {
            // Performance optimization: because applying each attribute causes signals to be fired, which is slow in a large scene
            // (and furthermore, on a server, causes each connection's sync state to be accessed), do not set the linear/angular
            // velocities if they haven't changed
            float3 linearVel = body->getLinearVelocity();
            float3 angularVel = RadToDeg(body->getAngularVelocity());
            if (!linearVel.Equals(rigidBody->linearVelocity.Get()))
                rigidBody->linearVelocity.Set(linearVel, changeType);
            if (!angularVel.Equals(rigidBody->angularVelocity.Get()))
                rigidBody->angularVelocity.Set(angularVel, changeType);
        }
    
        disconnected = false;
    }

    /// Calculate mass, shape & static/dynamic-classification dependant properties
    void GetProperties(btVector3& localInertia, float& m, int& collisionFlags)
    {
        localInertia = btVector3(0.0f, 0.0f, 0.0f);
        m = rigidBody->mass.Get();
        if (m < 0.0f)
            m = 0.0f;
        // Trimesh shape can not move
        if (rigidBody->shapeType.Get() == TriMesh)
            m = 0.0f;
        // On client, all server-side entities become static to not desync or try to send updates we should not
        //if (!HasAuthority())
        //   m = 0.0f;
    
        if (shape && m > 0.0f)
            shape->calculateLocalInertia(m, localInertia);
    
        bool isDynamic = m > 0.0f;
        bool isPhantom = rigidBody->phantom.Get();
        bool isKinematic = rigidBody->kinematic.Get();
        collisionFlags = 0;
        if (!isDynamic)
            collisionFlags |= btCollisionObject::CF_STATIC_OBJECT;
        if (isKinematic)
            collisionFlags |= btCollisionObject::CF_KINEMATIC_OBJECT;
        if (isPhantom)
            collisionFlags |= btCollisionObject::CF_NO_CONTACT_RESPONSE;
        if (!rigidBody->drawDebug.Get())
            collisionFlags |= btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT;
    }

    EC_RigidBody *rigidBody;
    /// Placeable pointer
    weak_ptr<EC_Placeable> placeable;
    /// Terrain pointer
    weak_ptr<EC_Terrain> terrain;
    /// Internal disconnection of attribute changes. True during the time we're setting attributes ourselves due to Bullet update, to prevent endless loop
    bool disconnected;
    /// On the client side, this field is used to track whether the rigid body is being interpolated from network input events (false), or extrapolated
    /// using local physics computations (true).
    /// On the server side, this flag is not used.
    bool clientExtrapolating;
    /// Bullet body
    btRigidBody* body;
    /// Bullet collision shape
    btCollisionShape* shape;
    /// Bullet collision child shape. This is needed to use btScaledBvhTriangleMeshShape
    btCollisionShape* childShape;
    /// Physics world. May be 0 if the scene does not have a physics world. In that case most of EC_RigidBody's functionality is a no-op
    PhysicsWorld* world;
    /// PhysicsModule pointer
    PhysicsModule* owner;
    /// Cached shapetype (last created)
    int cachedShapeType;
    /// Cached shapesize (last created)
    float3 cachedSize;
    /// Bullet triangle mesh
    shared_ptr<btTriangleMesh> triangleMesh;
    /// Convex hull set
    shared_ptr<ConvexHullSet> convexHullSet;
    /// Bullet heightfield shape. Note: this is always put inside a compound shape (impl->shape)
    btHeightfieldTerrainShape* heightField;
    /// Heightfield values, for the case the shape is a heightfield.
    std::vector<float> heightValues;
};

EC_RigidBody::EC_RigidBody(Scene* scene) :
    IComponent(scene),
    INIT_ATTRIBUTE_VALUE(mass, "Mass", 0.0f),
    INIT_ATTRIBUTE_VALUE(shapeType, "Shape type", (int)Box),
    INIT_ATTRIBUTE_VALUE(size, "Size", float3(1,1,1)),
    INIT_ATTRIBUTE_VALUE(collisionMeshRef, "Collision mesh ref", AssetReference("", "OgreMesh")),
    INIT_ATTRIBUTE_VALUE(friction, "Friction", 0.5f),
    INIT_ATTRIBUTE_VALUE(restitution, "Restitution", 0.0f),
    INIT_ATTRIBUTE_VALUE(linearDamping, "Linear damping", 0.0f),
    INIT_ATTRIBUTE_VALUE(angularDamping, "Angular damping", 0.0f),
    INIT_ATTRIBUTE_VALUE(linearFactor, "Linear factor", float3(1,1,1)),
    INIT_ATTRIBUTE_VALUE(angularFactor, "Angular factor", float3(1,1,1)),
    INIT_ATTRIBUTE_VALUE(linearVelocity, "Linear velocity", float3(0,0,0)),
    INIT_ATTRIBUTE_VALUE(angularVelocity, "Angular velocity", float3(0,0,0)),
    INIT_ATTRIBUTE_VALUE(phantom, "Phantom", false),
    INIT_ATTRIBUTE_VALUE(kinematic, "Kinematic", false),
    INIT_ATTRIBUTE_VALUE(drawDebug, "Draw Debug", false),
    INIT_ATTRIBUTE_VALUE(collisionLayer, "Collision Layer", -1),
    INIT_ATTRIBUTE_VALUE(collisionMask, "Collision Mask", -1),
    INIT_ATTRIBUTE_VALUE(rollingFriction, "Rolling friction", 0.5f),
    INIT_ATTRIBUTE_VALUE(useGravity, "Use gravity", true),
    impl(new Impl(this))
{
    static AttributeMetadata shapemetadata;
    static bool metadataInitialized = false;
    if(!metadataInitialized)
    {
        shapemetadata.enums[Box] = "Box";
        shapemetadata.enums[Sphere] = "Sphere";
        shapemetadata.enums[Cylinder] = "Cylinder";
        shapemetadata.enums[Capsule] = "Capsule";
        shapemetadata.enums[TriMesh] = "TriMesh";
        shapemetadata.enums[HeightField] = "HeightField";
        shapemetadata.enums[ConvexHull] = "ConvexHull";
        shapemetadata.enums[Cone] = "Cone";
        metadataInitialized = true;
    }
    shapeType.SetMetadata(&shapemetadata);

    connect(this, SIGNAL(ParentEntitySet()), SLOT(UpdateSignals()));
}

EC_RigidBody::~EC_RigidBody()
{
    RemoveBody();
    RemoveCollisionShape();
    if (impl->world)
        impl->world->debugRigidBodies_.erase(this);
    delete impl;
}

bool EC_RigidBody::SetShapeFromVisibleMesh()
{
    Entity* parent = ParentEntity();
    if (!parent)
        return false;
    EC_Mesh* mesh = parent->Component<EC_Mesh>().get();
    if (!mesh)
        return false;
    mass.Set(0.0f, AttributeChange::Default);
    shapeType.Set(TriMesh, AttributeChange::Default);
    collisionMeshRef.Set(mesh->meshRef.Get(), AttributeChange::Default);
    return true;
}

void EC_RigidBody::SetLinearVelocity(const float3& velocity)
{
    // Cannot modify server-authoritative physics object
    if (!HasAuthority())
        return;
    
    linearVelocity.Set(velocity, AttributeChange::Default);
}

void EC_RigidBody::SetAngularVelocity(const float3& velocity)
{
    // Cannot modify server-authoritative physics object
    if (!HasAuthority())
        return;
    
    angularVelocity.Set(velocity, AttributeChange::Default);
}

void EC_RigidBody::ApplyForce(const float3& force, const float3& position)
{
    // Cannot modify server-authoritative physics object
    if (!HasAuthority())
        return;
    
    // If force is very small, do not wake up the body and apply
    if (force.LengthSq() < cForceThresholdSq)
        return;
    
    if (!impl->body)
        CreateBody();
    if (impl->body)
    {
        Activate();
        if (position == float3::zero)
            impl->body->applyCentralForce(force);
        else
            impl->body->applyForce(force, position);
    }
}

void EC_RigidBody::ApplyTorque(const float3& torque)
{
    // Cannot modify server-authoritative physics object
    if (!HasAuthority())
        return;
    
    // If torque is very small, do not wake up the body and apply
    if (torque.LengthSq() < cTorqueThresholdSq)
        return;
        
    if (!impl->body)
        CreateBody();
    if (impl->body)
    {
        Activate();
        impl->body->applyTorque(torque);
    }
}

void EC_RigidBody::ApplyImpulse(const float3& impulse, const float3& position)
{
    // Cannot modify server-authoritative physics object
    if (!HasAuthority())
        return;
    
    // If impulse is very small, do not wake up the body and apply
    if (impulse.LengthSq() < cImpulseThresholdSq)
        return;
    
    if (!impl->body)
        CreateBody();
    if (impl->body)
    {
        Activate();
        if (position == float3::zero)
            impl->body->applyCentralImpulse(impulse);
        else
            impl->body->applyImpulse(impulse, position);
    }
}

void EC_RigidBody::ApplyTorqueImpulse(const float3& torqueImpulse)
{
    // Cannot modify server-authoritative physics object
    if (!HasAuthority())
        return;
    
    // If impulse is very small, do not wake up the body and apply
    if (torqueImpulse.LengthSq() < cTorqueThresholdSq)
        return;
        
    if (!impl->body)
        CreateBody();
    if (impl->body)
    {
        Activate();
        impl->body->applyTorqueImpulse(torqueImpulse);
    }
}

void EC_RigidBody::Activate()
{
    // Cannot modify server-authoritative physics object
    if (!HasAuthority())
        return;
    
    if (!impl->body)
        CreateBody();
    if (impl->body)
        impl->body->activate();
}

void EC_RigidBody::KeepActive()
{
    if (impl->body)
        impl->body->activate(true);
}

bool EC_RigidBody::IsActive()
{
    if (impl->body)
        return impl->body->isActive();
    else
        return false;
}

void EC_RigidBody::ResetForces()
{
    // Cannot modify server-authoritative physics object
    if (!HasAuthority())
        return;
    
    if (!impl->body)
        CreateBody();
    if (impl->body)
        impl->body->clearForces();
}

void EC_RigidBody::UpdateSignals()
{
    Entity* parent = ParentEntity();
    if (!parent)
        return;

    connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), this, SLOT(CheckForPlaceableAndTerrain()));

    impl->owner = framework->Module<PhysicsModule>();
    Scene* scene = parent->ParentScene();
    impl->world = scene->Subsystem<PhysicsWorld>().get();
    if (impl->world)
        connect(impl->world, SIGNAL(AboutToUpdate(float)), this, SLOT(OnAboutToUpdate()));
}

void EC_RigidBody::CheckForPlaceableAndTerrain()
{
    Entity* parent = ParentEntity();
    if (!parent)
        return;
    
    if (!impl->placeable.lock())
    {
        shared_ptr<EC_Placeable> placeable = parent->Component<EC_Placeable>();
        if (placeable)
        {
            impl->placeable = placeable;
            connect(placeable.get(), SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(PlaceableUpdated(IAttribute*)));
        }
    }
    if (!impl->terrain.lock())
    {
        shared_ptr<EC_Terrain> terrain = parent->Component<EC_Terrain>();
        if (terrain)
        {
            impl->terrain = terrain;
            connect(terrain.get(), SIGNAL(TerrainRegenerated()), this, SLOT(OnTerrainRegenerated()));
            connect(terrain.get(), SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(TerrainUpdated(IAttribute*)));
        }
    }
}

void EC_RigidBody::CreateCollisionShape()
{
    RemoveCollisionShape();
    
    float3 sizeVec = size.Get();
    // Sanitize the size
    if (sizeVec.x < 0)
        sizeVec.x = 0;
    if (sizeVec.y < 0)
        sizeVec.y = 0;
    if (sizeVec.z < 0)
        sizeVec.z = 0;
        
    switch (shapeType.Get())
    {
    case Box:
        // Note: Bullet uses box halfsize
        impl->shape = new btBoxShape(btVector3(sizeVec.x * 0.5f, sizeVec.y * 0.5f, sizeVec.z * 0.5f));
        break;
    case Sphere:
        impl->shape = new btSphereShape(sizeVec.x * 0.5f);
        break;
    case Cylinder:
        impl->shape = new btCylinderShape(btVector3(sizeVec.x * 0.5f, sizeVec.y * 0.5f, sizeVec.z * 0.5f));
        break;
    case Capsule:
        impl->shape = new btCapsuleShape(sizeVec.x * 0.5f, sizeVec.y * 0.5f);
        break;
    case TriMesh:
        if (impl->triangleMesh)
        {
            // Need to first create a bvhTriangleMeshShape, then a scaled version of it to allow for individual scaling.
            impl->childShape = new btBvhTriangleMeshShape(impl->triangleMesh.get(), true, true);
            impl->shape = new btScaledBvhTriangleMeshShape(static_cast<btBvhTriangleMeshShape*>(impl->childShape), btVector3(1.0f, 1.0f, 1.0f));
        }
        break;
    case HeightField:
        CreateHeightFieldFromTerrain();
        break;
    case ConvexHull:
        CreateConvexHullSetShape();
        break;
    case Cone:
        impl->shape = new btConeShape(sizeVec.x * 0.5f, sizeVec.y);
        break;
    }
    
    UpdateScale();
    
    // If body already exists, set the new collision shape, and remove/readd the body to the physics world to make sure Bullet's internal representations are updated
    ReaddBody();
}

void EC_RigidBody::RemoveCollisionShape()
{
    if (impl->shape)
    {
        if (impl->body)
            impl->body->setCollisionShape(0);
        SAFE_DELETE(impl->shape);
    }
    SAFE_DELETE(impl->childShape);
    SAFE_DELETE(impl->heightField);
}

void EC_RigidBody::CreateBody()
{
    if (!impl->world || !ParentEntity() || impl->body)
        return;
    
    CheckForPlaceableAndTerrain();
    
    CreateCollisionShape();
    
    btVector3 localInertia;
    float m;
    int collisionFlags;
    
    impl->GetProperties(localInertia, m, collisionFlags);
    
    impl->body = new btRigidBody(m, impl, impl->shape, localInertia);
    // TEST: Adjust the threshold of when to sleep the object - for reducing network bandwidth.
//    impl->body->setSleepingThresholds(0.2f, 0.5f); // Bullet defaults are 0.8 and 1.0.
    impl->body->setUserPointer(this);
    impl->body->setCollisionFlags(collisionFlags);
    impl->world->BulletWorld()->addRigidBody(impl->body, collisionLayer.Get(), collisionMask.Get());
    impl->body->activate();
    
    UpdateGravity();
}

void EC_RigidBody::ReaddBody()
{
    if (!impl->world || !ParentEntity() || !impl->body)
        return;

    btVector3 localInertia;
    float m;
    int collisionFlags;
    impl->GetProperties(localInertia, m, collisionFlags);

    impl->world->BulletWorld()->removeRigidBody(impl->body);

    impl->body->setCollisionShape(impl->shape);
    impl->body->setMassProps(m, localInertia);
    impl->body->setCollisionFlags(collisionFlags);

    // We have changed the inertia tensor properties of the object, so recompute it.
    // http://www.bulletphysics.org/Bullet/phpBB3/viewtopic.php?f=9&t=5194&hilit=inertia+tensor#p18820
    impl->body->updateInertiaTensor();

    impl->world->BulletWorld()->addRigidBody(impl->body, collisionLayer.Get(), collisionMask.Get());
    impl->body->clearForces();
    impl->body->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
    impl->body->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
    impl->body->activate();
}

void EC_RigidBody::RemoveBody()
{
    if (impl->body && impl->world)
    {
        impl->world->BulletWorld()->removeRigidBody(impl->body);
        SAFE_DELETE(impl->body);
    }
}

void EC_RigidBody::SetClientExtrapolating(bool isClientExtrapolating)
{
    impl->clientExtrapolating = isClientExtrapolating;
}

btRigidBody* EC_RigidBody::BulletRigidBody() const
{
    return impl->body;
}

void EC_RigidBody::OnTerrainRegenerated()
{
    if (shapeType.Get() == HeightField)
        CreateCollisionShape();
}

void EC_RigidBody::OnCollisionMeshAssetLoaded(AssetPtr asset)
{
    OgreMeshAssetPtr meshAsset = dynamic_pointer_cast<OgreMeshAsset>(asset);
    if (!meshAsset || !meshAsset->ogreMesh.get())
        LogError("EC_RigidBody::OnCollisionMeshAssetLoaded: Mesh asset load finished for asset \"" +
            asset->Name() + "\", but Ogre::Mesh pointer was null!");

    Ogre::Mesh *mesh = meshAsset->ogreMesh.get();

    if (mesh)
    {
        if (shapeType.Get() == TriMesh)
        {
            impl->triangleMesh = impl->owner->GetTriangleMeshFromOgreMesh(mesh);
            CreateCollisionShape();
        }
        if (shapeType.Get() == ConvexHull)
        {
            impl->convexHullSet = impl->owner->GetConvexHullSetFromOgreMesh(mesh);
            CreateCollisionShape();
        }

        impl->cachedShapeType = shapeType.Get();
        impl->cachedSize = size.Get();
    }
}

void EC_RigidBody::AttributesChanged()
{
    if (impl->disconnected)
        return;
    
    // Create body now if does not exist yet
    if (!impl->body)
        CreateBody();
    // If body was not created (we do not actually have a physics world), exit
    if (!impl->body)
        return;
    
    if (mass.ValueChanged() || collisionLayer.ValueChanged() || collisionMask.ValueChanged())
        // Readd body to the world in case static/dynamic classification changed, or if collision mask changed
        ReaddBody();
    
    if (friction.ValueChanged())
        impl->body->setFriction(friction.Get());
    
    if (rollingFriction.ValueChanged())
        impl->body->setRollingFriction(rollingFriction.Get());
    
    if (restitution.ValueChanged())
        impl->body->setRestitution(friction.Get());
    
    if (linearDamping.ValueChanged() || angularDamping.ValueChanged())
         impl->body->setDamping(linearDamping.Get(), angularDamping.Get());
    
    if (linearFactor.ValueChanged())
        impl->body->setLinearFactor(linearFactor.Get());
    
    if (angularFactor.ValueChanged())
        impl->body->setAngularFactor(angularFactor.Get());
    
    if (shapeType.ValueChanged() || size.ValueChanged())
    {
        if (shapeType.Get() != impl->cachedShapeType || size.Get() != impl->cachedSize)
        {
            // If shape does not involve mesh, can create it directly. Otherwise request the mesh
            if (shapeType.Get() != TriMesh && shapeType.Get() != ConvexHull)
            {
                CreateCollisionShape();
                impl->cachedShapeType = shapeType.Get();
                impl->cachedSize = size.Get();
            }
            else
                RequestMesh();
        }
    }
    
    // Request mesh if its id changes
    if (collisionMeshRef.ValueChanged())
    {
        if (shapeType.Get() == TriMesh || shapeType.Get() == ConvexHull)
            RequestMesh();
    }
    
    if (phantom.ValueChanged() || kinematic.ValueChanged())
        // Readd body to the world in case phantom or kinematic classification changed
        ReaddBody();
    
    if (drawDebug.ValueChanged())
    {
        bool enable = drawDebug.Get();
        if (impl->body)
        {
            int collisionFlags = impl->body->getCollisionFlags();
            if (enable)
                collisionFlags &= ~btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT;
            else
                collisionFlags |= btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT;
            impl->body->setCollisionFlags(collisionFlags);
        }
        
        // Refresh PhysicsWorld's knowledge of debug-enabled rigidbodies
        if (impl->world)
        {
            if (enable)
                impl->world->debugRigidBodies_.insert(this);
            else
                impl->world->debugRigidBodies_.erase(this);
        }
    }
    
    if (linearVelocity.ValueChanged() && !impl->disconnected)
    {
        impl->body->setLinearVelocity(linearVelocity.Get());
        impl->body->activate();
    }
    
    if (angularVelocity.ValueChanged() && !impl->disconnected)
    {
        impl->body->setAngularVelocity(DegToRad(angularVelocity.Get()));
        impl->body->activate();
    }
    
    if (useGravity.ValueChanged())
        UpdateGravity();
}

void EC_RigidBody::PlaceableUpdated(IAttribute* attribute)
{
    // Do not respond to our own change
    if (impl->disconnected || !impl->body)
        return;
    
    EC_Placeable* placeable = impl->placeable.lock().get();
    if (!placeable)
        return;
    
    if (attribute == &placeable->transform)
    {
        // Important: when changing both transform and parent, always set parentref first, then transform
        // Otherwise the physics simulation may interpret things wrong and the object ends up
        // in an unintended location
        UpdatePosRotFromPlaceable();
        UpdateScale();

        // Since we programmatically changed the orientation of the object outside the simulation, we must recompute the 
        // inertia tensor matrix of the object manually (it's dependent on the world space orientation of the object)
        impl->body->updateInertiaTensor();
    }
}

void EC_RigidBody::OnAboutToUpdate()
{
    PROFILE(EC_RigidBody_OnAboutToUpdate);
    
    // If the placeable is parented, we forcibly update world transform from it before each simulation step
    // However, we do not update scale, as that is expensive
    EC_Placeable* placeable = impl->placeable.lock().get();
    if (placeable && !placeable->parentRef.Get().IsEmpty() && placeable->IsAttached())
        UpdatePosRotFromPlaceable();
}

void EC_RigidBody::SetRotation(const float3& rotation)
{
    // Cannot modify server-authoritative physics object
    if (!HasAuthority())
        return;
    
    impl->disconnected = true;
    
    EC_Placeable* placeable = impl->placeable.lock().get();
    if (placeable)
    {
        Transform trans = placeable->transform.Get();
        trans.rot = rotation;
        placeable->transform.Set(trans, AttributeChange::Default);
        
        if (impl->body)
        {
            btTransform& worldTrans = impl->body->getWorldTransform();
            btTransform interpTrans = impl->body->getInterpolationWorldTransform();
            worldTrans.setRotation(trans.Orientation());
            interpTrans.setRotation(worldTrans.getRotation());
            impl->body->setInterpolationWorldTransform(interpTrans);
        }
    }
    
    impl->disconnected = false;
}

void EC_RigidBody::Rotate(const float3& rotation)
{
    // Cannot modify server-authoritative physics object
    if (!HasAuthority())
        return;
    
    impl->disconnected = true;
    
    EC_Placeable* placeable = impl->placeable.lock().get();
    if (placeable)
    {
        Transform trans = placeable->transform.Get();
        trans.rot += rotation;
        placeable->transform.Set(trans, AttributeChange::Default);
        
        if (impl->body)
        {
            btTransform& worldTrans = impl->body->getWorldTransform();
            btTransform interpTrans = impl->body->getInterpolationWorldTransform();
            worldTrans.setRotation(trans.Orientation());
            interpTrans.setRotation(worldTrans.getRotation());
            impl->body->setInterpolationWorldTransform(interpTrans);
        }
    }
    
    impl->disconnected = false;
}

PhysicsWorld* EC_RigidBody::World() const
{
    return impl->world;
}

float3 EC_RigidBody::GetLinearVelocity()
{
    if (impl->body)
        return impl->body->getLinearVelocity();
    else 
        return linearVelocity.Get();
}

float3 EC_RigidBody::GetAngularVelocity()
{
    if (impl->body)
        return RadToDeg(impl->body->getAngularVelocity());
    else
        return angularVelocity.Get();
}

void EC_RigidBody::GetAabbox(float3 &outAabbMin, float3 &outAabbMax)
{
    btVector3 aabbMin, aabbMax;
    impl->body->getAabb(aabbMin, aabbMax);
    outAabbMin.Set(aabbMin.x(), aabbMin.y(), aabbMin.z());
    outAabbMax.Set(aabbMax.x(), aabbMax.y(), aabbMax.z());
}

bool EC_RigidBody::HasAuthority() const
{
    if (!impl->world || (impl->world->IsClient() && !ParentEntity()->IsLocal()))
        return false;
    
    return true;
}

AABB EC_RigidBody::ShapeAABB() const
{
    btVector3 aabbMin, aabbMax;
    impl->body->getAabb(aabbMin, aabbMax);
    return AABB(aabbMin, aabbMax);
}

bool EC_RigidBody::IsPrimitiveShape() const
{
    switch(static_cast<ShapeType>(shapeType.Get()))
    {
    case TriMesh:
    case HeightField:
    case ConvexHull:
        return false;
    default:
        return true;
    }
}

PhysicsWorld* EC_RigidBody::GetPhysicsWorld() const
{
    LogWarning("EC_RigidBody:GetPhysicsWorld: this functions is deprecated and will be removed. Use World() instead.");
    return World();
}

void EC_RigidBody::TerrainUpdated(IAttribute* attribute)
{
    EC_Terrain* terrain = impl->terrain.lock().get();
    if (!terrain)
        return;
    /// \todo It is suboptimal to regenerate the whole heightfield when just the terrain's transform changes
    if (attribute == &terrain->nodeTransformation && shapeType.Get() == HeightField)
        CreateCollisionShape();
}

void EC_RigidBody::RequestMesh()
{    
    Entity *parent = ParentEntity();

    QString collisionMesh = collisionMeshRef.Get().ref.trimmed();
    if (collisionMesh.isEmpty() && parent) // We use the mesh ref in EC_Mesh as the collision mesh ref if no collision mesh is set in EC_RigidBody.
    {
        shared_ptr<EC_Mesh> mesh = parent->Component<EC_Mesh>();
        if (!mesh)
            return;
        collisionMesh = mesh->meshRef.Get().ref.trimmed();
    }

    if (!collisionMesh.isEmpty())
    {
        // Do not create shape right now, but request the mesh resource
        AssetTransferPtr transfer = GetFramework()->Asset()->RequestAsset(collisionMesh);
        if (transfer)
            connect(transfer.get(), SIGNAL(Succeeded(AssetPtr)), SLOT(OnCollisionMeshAssetLoaded(AssetPtr)), Qt::UniqueConnection);
    }
}

void EC_RigidBody::UpdateScale()
{
   PROFILE(EC_RigidBody_UpdateScale);
    
   float3 sizeVec = size.Get();
    // Sanitize the size
    if (sizeVec.x < 0)
        sizeVec.x = 0;
    if (sizeVec.y < 0)
        sizeVec.y = 0;
    if (sizeVec.z < 0)
        sizeVec.z = 0;
    
    // If placeable exists, set local scaling from its scale
    EC_Placeable* placeable = impl->placeable.lock().get();
    if (placeable && impl->shape)
    {
        // Note: for now, world scale is purposefully NOT used, because it would be problematic to change the scale when a parenting change occurs
        const float3& scale = placeable->transform.Get().scale;
        // Trianglemesh or convexhull does not have scaling of its own in the shape, so multiply with the size
        if (shapeType.Get() != TriMesh && shapeType.Get() != ConvexHull)
            impl->shape->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
        else
            impl->shape->setLocalScaling(btVector3(sizeVec.x * scale.x, sizeVec.y * scale.y, sizeVec.z * scale.z));
    }
}

void EC_RigidBody::UpdateGravity()
{
    if (!impl->body || !impl->world)
        return;
    
    int flags = impl->body->getFlags();
    if (useGravity.Get())
    {
        impl->body->setGravity(impl->world->BulletWorld()->getGravity());
        impl->body->activate(); // Activate in case body was sleeping
        flags &= ~BT_DISABLE_WORLD_GRAVITY;
    }
    else
    {
        impl->body->setGravity(btVector3(0.0f, 0.0f, 0.0f));
        flags |= BT_DISABLE_WORLD_GRAVITY;
    }
    impl->body->setFlags(flags);
}

void EC_RigidBody::CreateHeightFieldFromTerrain()
{
    CheckForPlaceableAndTerrain();
    
    EC_Terrain* terrain = impl->terrain.lock().get();
    if (!terrain)
        return;
    
    uint width = terrain->PatchWidth() * EC_Terrain::cPatchSize;
    uint height = terrain->PatchHeight() * EC_Terrain::cPatchSize;
    if (!width || !height)
        return;
    
    impl->heightValues.resize(width * height);
    
    float xzSpacing = 1.0f;
    float ySpacing = 1.0f;
    float minY = 1000000000;
    float maxY = -1000000000;
    for(uint z = 0; z < height; ++z)
        for(uint x = 0; x < width; ++x)
        {
            float value = terrain->GetPoint(x, z);
            if (value < minY)
                minY = value;
            if (value > maxY)
                maxY = value;
            impl->heightValues[z * width + x] = value;
        }

    float3 scale = terrain->nodeTransformation.Get().scale;
    float3 bbMin(0, minY, 0);
    float3 bbMax(xzSpacing * (width - 1), maxY, xzSpacing * (height - 1));
    float3 bbCenter = scale.Mul((bbMin + bbMax) * 0.5f);
    
    impl->heightField = new btHeightfieldTerrainShape(width, height, &impl->heightValues[0], ySpacing, minY, maxY, 1, PHY_FLOAT, false);
    
    /** \todo EC_Terrain uses its own transform that is independent of the placeable. It is not nice to support, since rest of EC_RigidBody assumes
        the transform is in the placeable. Right now, we only support position & scaling. Here, we also counteract Bullet's nasty habit to center 
        the heightfield on its own. Also, Bullet's collisionshapes generally do not support arbitrary transforms, so we must construct a "compound shape"
        and add the heightfield as its child, to be able to specify the transform.
     */
    impl->heightField->setLocalScaling(scale);
    
    float3 positionAdjust = terrain->nodeTransformation.Get().pos;
    positionAdjust += bbCenter;
    
    btCompoundShape* compound = new btCompoundShape();
    impl->shape = compound;
    compound->addChildShape(btTransform(btQuaternion(0,0,0,1), positionAdjust), impl->heightField);
}

void EC_RigidBody::CreateConvexHullSetShape()
{
    if (!impl->convexHullSet)
        return;
    
    // Avoid creating a compound shape if only 1 hull in the set
    if (impl->convexHullSet->hulls_.size() > 1)
    {
        btCompoundShape* compound = new btCompoundShape();
        impl->shape = compound;
        for (uint i = 0; i < impl->convexHullSet->hulls_.size(); ++i)
            compound->addChildShape(btTransform(btQuaternion(0,0,0,1), impl->convexHullSet->hulls_[i].position_), impl->convexHullSet->hulls_[i].hull_.get());
    }
    else if (impl->convexHullSet->hulls_.size() == 1)
    {
        btConvexHullShape* original = impl->convexHullSet->hulls_[0].hull_.get();
        btConvexHullShape* convex = new btConvexHullShape(reinterpret_cast<const btScalar*>(original->getUnscaledPoints()), original->getNumVertices());
        impl->shape = convex;
    }
}

void EC_RigidBody::UpdatePosRotFromPlaceable()
{
    PROFILE(EC_RigidBody_UpdatePosRotFromPlaceable);
    
    EC_Placeable* placeable = impl->placeable.lock().get();
    if (!placeable || !impl->body)
        return;
    
    float3 position = placeable->WorldPosition();
    Quat orientation = placeable->WorldOrientation();

    btTransform& worldTrans = impl->body->getWorldTransform();
    worldTrans.setOrigin(position);
    worldTrans.setRotation(orientation);
    
    // When we forcibly set the physics transform, also set the interpolation transform to prevent jerky motion
    btTransform interpTrans = impl->body->getInterpolationWorldTransform();
    interpTrans.setOrigin(worldTrans.getOrigin());
    interpTrans.setRotation(worldTrans.getRotation());
    impl->body->setInterpolationWorldTransform(interpTrans);
    
    KeepActive();
}

void EC_RigidBody::EmitPhysicsCollision(Entity* otherEntity, const float3& position, const float3& normal, float distance, float impulse, bool newCollision)
{
    PROFILE(EC_RigidBody_EmitPhysicsCollision);
    emit PhysicsCollision(otherEntity, position, normal, distance, impulse, newCollision);
}

