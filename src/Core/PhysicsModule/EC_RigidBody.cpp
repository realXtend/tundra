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
#include <BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.h>
#include <set>

#include <OgreSceneNode.h>

using namespace Physics;

static const float cForceThreshold = 0.0005f;
static const float cImpulseThreshold = 0.0005f;
static const float cTorqueThreshold = 0.0005f;

EC_RigidBody::EC_RigidBody(Scene* scene) :
    IComponent(scene),
    mass(this, "Mass", 0.0f),
    shapeType(this, "Shape type", (int)Shape_Box),
    size(this, "Size", float3(1,1,1)),
    collisionMeshRef(this, "Collision mesh ref", AssetReference("", "OgreMesh")),
    friction(this, "Friction", 0.5f),
    restitution(this, "Restitution", 0.0f),
    linearDamping(this, "Linear damping", 0.0f),
    angularDamping(this, "Angular damping", 0.0f),
    linearFactor(this, "Linear factor", float3(1,1,1)),
    angularFactor(this, "Angular factor", float3(1,1,1)),
    linearVelocity(this, "Linear velocity", float3(0,0,0)),
    angularVelocity(this, "Angular velocity", float3(0,0,0)),
    phantom(this, "Phantom", false),
    kinematic(this, "Kinematic", false),
    drawDebug(this, "Draw Debug", false),
    collisionLayer(this, "Collision Layer", -1),
    collisionMask(this, "Collision Mask", -1),
    body_(0),
    world_(0),
    shape_(0),
    childShape_(0),
    heightField_(0),
    disconnected_(false),
    cachedShapeType_(-1),
    cachedSize_(float3::zero),
    clientExtrapolating(false)
{
    owner_ = framework->GetModule<PhysicsModule>();
    
    static AttributeMetadata shapemetadata;
    static bool metadataInitialized = false;
    if(!metadataInitialized)
    {
        shapemetadata.enums[Shape_Box] = "Box";
        shapemetadata.enums[Shape_Sphere] = "Sphere";
        shapemetadata.enums[Shape_Cylinder] = "Cylinder";
        shapemetadata.enums[Shape_Capsule] = "Capsule";
        shapemetadata.enums[Shape_TriMesh] = "TriMesh";
        shapemetadata.enums[Shape_HeightField] = "HeightField";
        shapemetadata.enums[Shape_ConvexHull] = "ConvexHull";
        metadataInitialized = true;
    }
    shapeType.SetMetadata(&shapemetadata);

    connect(this, SIGNAL(ParentEntitySet()), SLOT(UpdateSignals()));
}

EC_RigidBody::~EC_RigidBody()
{
    RemoveBody();
    RemoveCollisionShape();
    if (world_)
        world_->debugRigidBodies_.erase(this);
}

bool EC_RigidBody::SetShapeFromVisibleMesh()
{
    Entity* parent = ParentEntity();
    if (!parent)
        return false;
    EC_Mesh* mesh = parent->GetComponent<EC_Mesh>().get();
    if (!mesh)
        return false;
    mass.Set(0.0f, AttributeChange::Default);
    shapeType.Set(Shape_TriMesh, AttributeChange::Default);
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
    if (force.Length() < cForceThreshold) ///\todo Use force.LengthSq() instead for optimization.
        return;
    
    if (!body_)
        CreateBody();
    if (body_)
    {
        Activate();
        if (position == float3::zero)
            body_->applyCentralForce(force);
        else
            body_->applyForce(force, position);
    }
}

void EC_RigidBody::ApplyTorque(const float3& torque)
{
    // Cannot modify server-authoritative physics object
    if (!HasAuthority())
        return;
    
    // If torque is very small, do not wake up the body and apply
    if (torque.Length() < cTorqueThreshold)  ///\todo Use torque.LengthSq() instead for optimization.
        return;
        
    if (!body_)
        CreateBody();
    if (body_)
    {
        Activate();
        body_->applyTorque(torque);
    }
}

void EC_RigidBody::ApplyImpulse(const float3& impulse, const float3& position)
{
    // Cannot modify server-authoritative physics object
    if (!HasAuthority())
        return;
    
    // If impulse is very small, do not wake up the body and apply
    if (impulse.Length() < cImpulseThreshold)  ///\todo Use impulse.LengthSq() instead for optimization.
        return;
    
    if (!body_)
        CreateBody();
    if (body_)
    {
        Activate();
        if (position == float3::zero)
            body_->applyCentralImpulse(impulse);
        else
            body_->applyImpulse(impulse, position);
    }
}

void EC_RigidBody::ApplyTorqueImpulse(const float3& torqueImpulse)
{
    // Cannot modify server-authoritative physics object
    if (!HasAuthority())
        return;
    
    // If impulse is very small, do not wake up the body and apply
    if (torqueImpulse.Length() < cTorqueThreshold)  ///\todo Use torqueImpulse.LengthSq() instead for optimization.
        return;
        
    if (!body_)
        CreateBody();
    if (body_)
    {
        Activate();
        body_->applyTorqueImpulse(torqueImpulse);
    }
}

void EC_RigidBody::Activate()
{
    // Cannot modify server-authoritative physics object
    if (!HasAuthority())
        return;
    
    if (!body_)
        CreateBody();
    if (body_)
        body_->activate();
}

void EC_RigidBody::KeepActive()
{
    if (body_)
        body_->activate(true);
}

bool EC_RigidBody::IsActive()
{
    if (body_)
        return body_->isActive();
    else
        return false;
}

void EC_RigidBody::ResetForces()
{
    // Cannot modify server-authoritative physics object
    if (!HasAuthority())
        return;
    
    if (!body_)
        CreateBody();
    if (body_)
        body_->clearForces();
}

void EC_RigidBody::UpdateSignals()
{
    Entity* parent = ParentEntity();
    if (!parent)
        return;
    
    connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), this, SLOT(CheckForPlaceableAndTerrain()));
    
    Scene* scene = parent->ParentScene();
    world_ = scene->GetWorld<PhysicsWorld>().get();
    if (world_)
        connect(world_, SIGNAL(AboutToUpdate(float)), this, SLOT(OnAboutToUpdate()));
}

void EC_RigidBody::CheckForPlaceableAndTerrain()
{
    Entity* parent = ParentEntity();
    if (!parent)
        return;
    
    if (!placeable_.lock())
    {
        shared_ptr<EC_Placeable> placeable = parent->GetComponent<EC_Placeable>();
        if (placeable)
        {
            placeable_ = placeable;
            connect(placeable.get(), SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(PlaceableUpdated(IAttribute*)));
        }
    }
    if (!terrain_.lock())
    {
        shared_ptr<EC_Terrain> terrain = parent->GetComponent<EC_Terrain>();
        if (terrain)
        {
            terrain_ = terrain;
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
    case Shape_Box:
        // Note: Bullet uses box halfsize
        shape_ = new btBoxShape(btVector3(sizeVec.x * 0.5f, sizeVec.y * 0.5f, sizeVec.z * 0.5f));
        break;
    case Shape_Sphere:
        shape_ = new btSphereShape(sizeVec.x * 0.5f);
        break;
    case Shape_Cylinder:
        shape_ = new btCylinderShape(btVector3(sizeVec.x * 0.5f, sizeVec.y * 0.5f, sizeVec.z * 0.5f));
        break;
    case Shape_Capsule:
        shape_ = new btCapsuleShape(sizeVec.x * 0.5f, sizeVec.y * 0.5f);
        break;
    case Shape_TriMesh:
        if (triangleMesh_)
        {
            // Need to first create a bvhTriangleMeshShape, then a scaled version of it to allow for individual scaling.
            childShape_ = new btBvhTriangleMeshShape(triangleMesh_.get(), true, true);
            shape_ = new btScaledBvhTriangleMeshShape(static_cast<btBvhTriangleMeshShape*>(childShape_), btVector3(1.0f, 1.0f, 1.0f));
        }
        break;
    case Shape_HeightField:
        CreateHeightFieldFromTerrain();
        break;
    case Shape_ConvexHull:
        CreateConvexHullSetShape();
        break;
    }
    
    UpdateScale();
    
    // If body already exists, set the new collision shape, and remove/readd the body to the physics world to make sure Bullet's internal representations are updated
    ReaddBody();
}

void EC_RigidBody::RemoveCollisionShape()
{
    if (shape_)
    {
        if (body_)
            body_->setCollisionShape(0);
        delete shape_;
        shape_ = 0;
    }
    if (childShape_)
    {
        delete childShape_;
        childShape_ = 0;
    }
    if (heightField_)
    {
        delete heightField_;
        heightField_ = 0;
    }
}

void EC_RigidBody::CreateBody()
{
    if ((!world_) || (!ParentEntity()) || (body_))
        return;
    
    CheckForPlaceableAndTerrain();
    
    CreateCollisionShape();
    
    btVector3 localInertia;
    float m;
    int collisionFlags;
    
    GetProperties(localInertia, m, collisionFlags);
    
    body_ = new btRigidBody(m, this, shape_, localInertia);
    // TEST: Adjust the threshold of when to sleep the object - for reducing network bandwidth.
//    body_->setSleepingThresholds(0.2f, 0.5f); // Bullet defaults are 0.8 and 1.0.
    body_->setUserPointer(this);
    body_->setCollisionFlags(collisionFlags);
    world_->BulletWorld()->addRigidBody(body_, collisionLayer.Get(), collisionMask.Get());
    body_->activate();
}

void EC_RigidBody::ReaddBody()
{
    if ((!world_) || (!ParentEntity()) || (!body_))
        return;

    btVector3 localInertia;
    float m;
    int collisionFlags;
    GetProperties(localInertia, m, collisionFlags);

    world_->BulletWorld()->removeRigidBody(body_);

    body_->setCollisionShape(shape_);
    body_->setMassProps(m, localInertia);
    body_->setCollisionFlags(collisionFlags);

    // We have changed the inertia tensor properties of the object, so recompute it.
    // http://www.bulletphysics.org/Bullet/phpBB3/viewtopic.php?f=9&t=5194&hilit=inertia+tensor#p18820
    body_->updateInertiaTensor();

    world_->BulletWorld()->addRigidBody(body_, collisionLayer.Get(), collisionMask.Get());
    body_->clearForces();
    body_->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
    body_->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
    body_->activate();
}

void EC_RigidBody::RemoveBody()
{
    if ((body_) && (world_))
    {
        world_->BulletWorld()->removeRigidBody(body_);
        delete body_;
        body_ = 0;
    }
}

void EC_RigidBody::getWorldTransform(btTransform &worldTrans) const
{
    EC_Placeable* placeable = placeable_.lock().get();
    if (!placeable)
        return;
        
    float3 position = placeable->WorldPosition();
    Quat orientation = placeable->WorldOrientation();
    
    worldTrans.setOrigin(position);
    worldTrans.setRotation(orientation);
}

void EC_RigidBody::setWorldTransform(const btTransform &worldTrans)
{
    /// \todo For a large scene, applying the changed transforms of rigid bodies is slow (slower than the physics simulation itself,
    /// or handling collisions) due to the large number of Qt signals being fired.
    
    PROFILE(EC_RigidBody_SetWorldTransform);
    
    // Cannot modify server-authoritative physics object, rather get the transform changes through placeable attributes
    const bool hasAuthority = HasAuthority();
    if (!hasAuthority && !clientExtrapolating)
        return;
    
    EC_Placeable* placeable = placeable_.lock().get();
    if (!placeable)
        return;
    
    // Important: disconnect our own response to attribute changes to not create an endless loop!
    disconnected_ = true;
    
    AttributeChange::Type changeType = hasAuthority ? AttributeChange::Default : AttributeChange::LocalOnly;

    // Set transform
    float3 position = worldTrans.getOrigin();
    Quat orientation = worldTrans.getRotation();
    
    // Non-parented case
    if (placeable->parentRef.Get().IsEmpty())
    {
        Transform newTrans = placeable->transform.Get();
        newTrans.SetPos(position.x, position.y, position.z);
        newTrans.SetOrientation(orientation);
        placeable->transform.Set(newTrans, changeType);
    }
    else
    // The placeable has a parent itself
    {
        if (placeable->IsAttached())
        {
            position = placeable->GetSceneNode()->convertWorldToLocalPosition(position);
            orientation = placeable->GetSceneNode()->convertWorldToLocalOrientation(orientation);
            
            Transform newTrans = placeable->transform.Get();
            newTrans.SetPos(position);
            newTrans.SetOrientation(orientation);
            placeable->transform.Set(newTrans, changeType);
        }
    }
    // Set linear & angular velocity
    if (body_)
    {
        // Performance optimization: because applying each attribute causes signals to be fired, which is slow in a large scene
        // (and furthermore, on a server, causes each connection's sync state to be accessed), do not set the linear/angular
        // velocities if they haven't changed
        float3 linearVel = body_->getLinearVelocity();
        float3 angularVel = RadToDeg(body_->getAngularVelocity());
        if (!linearVel.Equals(linearVelocity.Get()))
            linearVelocity.Set(linearVel, changeType);
        if (!angularVel.Equals(angularVelocity.Get()))
            angularVelocity.Set(angularVel, changeType);
    }
    
    disconnected_ = false;
}

void EC_RigidBody::SetClientExtrapolating(bool isClientExtrapolating)
{
    clientExtrapolating = isClientExtrapolating;
}

void EC_RigidBody::OnTerrainRegenerated()
{
    if (shapeType.Get() == Shape_HeightField)
        CreateCollisionShape();
}

void EC_RigidBody::OnCollisionMeshAssetLoaded(AssetPtr asset)
{
    OgreMeshAsset *meshAsset = dynamic_cast<OgreMeshAsset*>(asset.get());
    if (!meshAsset || !meshAsset->ogreMesh.get())
        LogError("EC_RigidBody::OnCollisionMeshAssetLoaded: Mesh asset load finished for asset \"" +
            asset->Name() + "\", but Ogre::Mesh pointer was null!");

    Ogre::Mesh *mesh = meshAsset->ogreMesh.get();

    if (mesh)
    {
        if (shapeType.Get() == Shape_TriMesh)
        {
            triangleMesh_ = owner_->GetTriangleMeshFromOgreMesh(mesh);
            CreateCollisionShape();
        }
        if (shapeType.Get() == Shape_ConvexHull)
        {
            convexHullSet_ = owner_->GetConvexHullSetFromOgreMesh(mesh);
            CreateCollisionShape();
        }

        cachedShapeType_ = shapeType.Get();
        cachedSize_ = size.Get();
    }
}

void EC_RigidBody::AttributesChanged()
{
    if (disconnected_)
        return;
    
    // Create body now if does not exist yet
    if (!body_)
        CreateBody();
    // If body was not created (we do not actually have a physics world), exit
    if (!body_)
        return;
    
    if (mass.ValueChanged() || collisionLayer.ValueChanged() || collisionMask.ValueChanged())
        // Readd body to the world in case static/dynamic classification changed, or if collision mask changed
        ReaddBody();
    
    if (friction.ValueChanged())
        body_->setFriction(friction.Get());
    
    if (restitution.ValueChanged())
        body_->setRestitution(friction.Get());
    
    if (linearDamping.ValueChanged() || angularDamping.ValueChanged())
         body_->setDamping(linearDamping.Get(), angularDamping.Get());
    
    if (linearFactor.ValueChanged())
        body_->setLinearFactor(linearFactor.Get());
    
    if (angularFactor.ValueChanged())
        body_->setAngularFactor(angularFactor.Get());
    
    if (shapeType.ValueChanged() || size.ValueChanged())
    {
        if ((shapeType.Get() != cachedShapeType_) || (size.Get() != cachedSize_))
        {
            // If shape does not involve mesh, can create it directly. Otherwise request the mesh
            if ((shapeType.Get() != Shape_TriMesh) && (shapeType.Get() != Shape_ConvexHull))
            {
                CreateCollisionShape();
                cachedShapeType_ = shapeType.Get();
                cachedSize_ = size.Get();
            }
            else
                RequestMesh();
        }
    }
    
    // Request mesh if its id changes
    if (collisionMeshRef.ValueChanged())
    {
        if (shapeType.Get() == Shape_TriMesh || shapeType.Get() == Shape_ConvexHull)
            RequestMesh();
    }
    
    if (phantom.ValueChanged() || kinematic.ValueChanged())
        // Readd body to the world in case phantom or kinematic classification changed
        ReaddBody();
    
    if (drawDebug.ValueChanged())
    {
        bool enable = drawDebug.Get();
        if (body_)
        {
            int collisionFlags = body_->getCollisionFlags();
            if (enable)
                collisionFlags &= ~btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT;
            else
                collisionFlags |= btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT;
            body_->setCollisionFlags(collisionFlags);
        }
        
        // Refresh PhysicsWorld's knowledge of debug-enabled rigidbodies
        if (world_)
        {
            if (enable)
                world_->debugRigidBodies_.insert(this);
            else
                world_->debugRigidBodies_.erase(this);
        }
    }
    
    if (linearVelocity.ValueChanged())
    {
        body_->setLinearVelocity(linearVelocity.Get());
        body_->activate();
    }
    
    if (angularVelocity.ValueChanged())
    {
        body_->setAngularVelocity(DegToRad(angularVelocity.Get()));
        body_->activate();
    }
}

void EC_RigidBody::PlaceableUpdated(IAttribute* attribute)
{
    // Do not respond to our own change
    if ((disconnected_) || (!body_))
        return;
    
    EC_Placeable* placeable = placeable_.lock().get();
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
        body_->updateInertiaTensor();
    }
}

void EC_RigidBody::OnAboutToUpdate()
{
    PROFILE(EC_RigidBody_OnAboutToUpdate);
    
    // If the placeable is parented, we forcibly update world transform from it before each simulation step
    // However, we do not update scale, as that is expensive
    EC_Placeable* placeable = placeable_.lock().get();
    if (placeable && !placeable->parentRef.Get().IsEmpty() && placeable->IsAttached())
        UpdatePosRotFromPlaceable();
}

void EC_RigidBody::SetRotation(const float3& rotation)
{
    // Cannot modify server-authoritative physics object
    if (!HasAuthority())
        return;
    
    disconnected_ = true;
    
    EC_Placeable* placeable = placeable_.lock().get();
    if (placeable)
    {
        Transform trans = placeable->transform.Get();
        trans.rot = rotation;
        placeable->transform.Set(trans, AttributeChange::Default);
        
        if (body_)
        {
            btTransform& worldTrans = body_->getWorldTransform();
            btTransform interpTrans = body_->getInterpolationWorldTransform();
            worldTrans.setRotation(trans.Orientation());
            interpTrans.setRotation(worldTrans.getRotation());
            body_->setInterpolationWorldTransform(interpTrans);
        }
    }
    
    disconnected_ = false;
}

void EC_RigidBody::Rotate(const float3& rotation)
{
    // Cannot modify server-authoritative physics object
    if (!HasAuthority())
        return;
    
    disconnected_ = true;
    
    EC_Placeable* placeable = placeable_.lock().get();
    if (placeable)
    {
        Transform trans = placeable->transform.Get();
        trans.rot += rotation;
        placeable->transform.Set(trans, AttributeChange::Default);
        
        if (body_)
        {
            btTransform& worldTrans = body_->getWorldTransform();
            btTransform interpTrans = body_->getInterpolationWorldTransform();
            worldTrans.setRotation(trans.Orientation());
            interpTrans.setRotation(worldTrans.getRotation());
            body_->setInterpolationWorldTransform(interpTrans);
        }
    }
    
    disconnected_ = false;
}

float3 EC_RigidBody::GetLinearVelocity()
{
    if (body_)
        return body_->getLinearVelocity();
    else 
        return linearVelocity.Get();
}

float3 EC_RigidBody::GetAngularVelocity()
{
    if (body_)
        return RadToDeg(body_->getAngularVelocity());
    else
        return angularVelocity.Get();
}

void EC_RigidBody::GetAabbox(float3 &outAabbMin, float3 &outAabbMax)
{
    btVector3 aabbMin, aabbMax;
    body_->getAabb(aabbMin, aabbMax);
    outAabbMin.Set(aabbMin.x(), aabbMin.y(), aabbMin.z());
    outAabbMax.Set(aabbMax.x(), aabbMax.y(), aabbMax.z());
}

bool EC_RigidBody::HasAuthority() const
{
    if ((!world_) || ((world_->IsClient()) && (!ParentEntity()->IsLocal())))
        return false;
    
    return true;
}

AABB EC_RigidBody::ShapeAABB() const
{
    btVector3 aabbMin, aabbMax;
    body_->getAabb(aabbMin, aabbMax);
    return AABB(aabbMin, aabbMax);
}

void EC_RigidBody::TerrainUpdated(IAttribute* attribute)
{
    EC_Terrain* terrain = terrain_.lock().get();
    if (!terrain)
        return;
    /// \todo It is suboptimal to regenerate the whole heightfield when just the terrain's transform changes
    if ((attribute == &terrain->nodeTransformation) && (shapeType.Get() == Shape_HeightField))
        CreateCollisionShape();
}

void EC_RigidBody::RequestMesh()
{    
    Entity *parent = ParentEntity();

    QString collisionMesh = collisionMeshRef.Get().ref.trimmed();
    if (collisionMesh.isEmpty() && parent) // We use the mesh ref in EC_Mesh as the collision mesh ref if no collision mesh is set in EC_RigidBody.
    {
        shared_ptr<EC_Mesh> mesh = parent->GetComponent<EC_Mesh>();
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
    EC_Placeable* placeable = placeable_.lock().get();
    if ((placeable) && (shape_))
    {
        // Note: for now, world scale is purposefully NOT used, because it would be problematic to change the scale when a parenting change occurs
        const float3& scale = placeable->transform.Get().scale;
        // Trianglemesh or convexhull does not have scaling of its own in the shape, so multiply with the size
        if ((shapeType.Get() != Shape_TriMesh) && (shapeType.Get() != Shape_ConvexHull))
            shape_->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
        else
            shape_->setLocalScaling(btVector3(sizeVec.x * scale.x, sizeVec.y * scale.y, sizeVec.z * scale.z));
    }
}

void EC_RigidBody::CreateHeightFieldFromTerrain()
{
    CheckForPlaceableAndTerrain();
    
    EC_Terrain* terrain = terrain_.lock().get();
    if (!terrain)
        return;
    
    int width = terrain->PatchWidth() * EC_Terrain::cPatchSize;
    int height = terrain->PatchHeight() * EC_Terrain::cPatchSize;
    
    if ((!width) || (!height))
        return;
    
    heightValues_.resize(width * height);
    
    float xzSpacing = 1.0f;
    float ySpacing = 1.0f;
    float minY = 1000000000;
    float maxY = -1000000000;
    for(uint z = 0; z < (uint)height; ++z)
        for(uint x = 0; x < (uint)width; ++x)
        {
            float value = terrain->GetPoint(x, z);
            if (value < minY)
                minY = value;
            if (value > maxY)
                maxY = value;
            heightValues_[z * width + x] = value;
        }

    float3 scale = terrain->nodeTransformation.Get().scale;
    float3 bbMin(0, minY, 0);
    float3 bbMax(xzSpacing * (width - 1), maxY, xzSpacing * (height - 1));
    float3 bbCenter = scale.Mul((bbMin + bbMax) * 0.5f);
    
    heightField_ = new btHeightfieldTerrainShape(width, height, &heightValues_[0], ySpacing, minY, maxY, 1, PHY_FLOAT, false);
    
    /** \todo EC_Terrain uses its own transform that is independent of the placeable. It is not nice to support, since rest of EC_RigidBody assumes
        the transform is in the placeable. Right now, we only support position & scaling. Here, we also counteract Bullet's nasty habit to center 
        the heightfield on its own. Also, Bullet's collisionshapes generally do not support arbitrary transforms, so we must construct a "compound shape"
        and add the heightfield as its child, to be able to specify the transform.
     */
    heightField_->setLocalScaling(scale);
    
    float3 positionAdjust = terrain->nodeTransformation.Get().pos;
    positionAdjust += bbCenter;
    
    btCompoundShape* compound = new btCompoundShape();
    shape_ = compound;
    compound->addChildShape(btTransform(btQuaternion(0,0,0,1), positionAdjust), heightField_);
}

void EC_RigidBody::CreateConvexHullSetShape()
{
    if (!convexHullSet_)
        return;
    
    // Avoid creating a compound shape if only 1 hull in the set
    if (convexHullSet_->hulls_.size() > 1)
    {
        btCompoundShape* compound = new btCompoundShape();
        shape_ = compound;
        for (uint i = 0; i < convexHullSet_->hulls_.size(); ++i)
            compound->addChildShape(btTransform(btQuaternion(0,0,0,1), convexHullSet_->hulls_[i].position_), convexHullSet_->hulls_[i].hull_.get());
    }
    else if (convexHullSet_->hulls_.size() == 1)
    {
        btConvexHullShape* original = convexHullSet_->hulls_[0].hull_.get();
        btConvexHullShape* convex = new btConvexHullShape(reinterpret_cast<const btScalar*>(original->getUnscaledPoints()), original->getNumVertices());
        shape_ = convex;
    }
}

void EC_RigidBody::GetProperties(btVector3& localInertia, float& m, int& collisionFlags)
{
    localInertia = btVector3(0.0f, 0.0f, 0.0f);
    m = mass.Get();
    if (m < 0.0f)
        m = 0.0f;
    // Trimesh shape can not move
    if (shapeType.Get() == Shape_TriMesh)
        m = 0.0f;
    // On client, all server-side entities become static to not desync or try to send updates we should not
    //if (!HasAuthority())
    //   m = 0.0f;
    
    if ((shape_) && (m > 0.0f))
        shape_->calculateLocalInertia(m, localInertia);
    
    bool isDynamic = m > 0.0f;
    bool isPhantom = phantom.Get();
    bool isKinematic = kinematic.Get();
    collisionFlags = 0;
    if (!isDynamic)
        collisionFlags |= btCollisionObject::CF_STATIC_OBJECT;
    if (isKinematic)
        collisionFlags |= btCollisionObject::CF_KINEMATIC_OBJECT;
    if (isPhantom)
        collisionFlags |= btCollisionObject::CF_NO_CONTACT_RESPONSE;
    if (!drawDebug.Get())
        collisionFlags |= btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT;
}

void EC_RigidBody::UpdatePosRotFromPlaceable()
{
    PROFILE(EC_RigidBody_UpdatePosRotFromPlaceable);
    
    EC_Placeable* placeable = placeable_.lock().get();
    if (!placeable || !body_)
        return;
    
    float3 position = placeable->WorldPosition();
    Quat orientation = placeable->WorldOrientation();

    btTransform& worldTrans = body_->getWorldTransform();
    worldTrans.setOrigin(position);
    worldTrans.setRotation(orientation);
    
    // When we forcibly set the physics transform, also set the interpolation transform to prevent jerky motion
    btTransform interpTrans = body_->getInterpolationWorldTransform();
    interpTrans.setOrigin(worldTrans.getOrigin());
    interpTrans.setRotation(worldTrans.getRotation());
    body_->setInterpolationWorldTransform(interpTrans);
    
    KeepActive();
}

void EC_RigidBody::EmitPhysicsCollision(Entity* otherEntity, const float3& position, const float3& normal, float distance, float impulse, bool newCollision)
{
    PROFILE(EC_RigidBody_EmitPhysicsCollision);
    emit PhysicsCollision(otherEntity, position, normal, distance, impulse, newCollision);
}

