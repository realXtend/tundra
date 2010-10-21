// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_Mesh.h"
#include "EC_RigidBody.h"
#include "EC_Placeable.h"
#include "EC_Terrain.h"
#include "ConvexHull.h"
#include "PhysicsModule.h"
#include "PhysicsUtils.h"
#include "PhysicsWorld.h"
#include "EventManager.h"
#include "RexTypes.h"
#include "Renderer.h"
#include "OgreMeshResource.h"
#include "ServiceManager.h"

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_RigidBody");

using namespace Physics;
using namespace RexTypes;

EC_RigidBody::EC_RigidBody(IModule* module) :
    IComponent(module->GetFramework()),
    body_(0),
    world_(0),
    shape_(0),
    heightField_(0),
    disconnected_(false),
    owner_(checked_static_cast<PhysicsModule*>(module)),
    mass(this, "Mass", 0.0f),
    shapeType(this, "Shape type", (int)Shape_Box),
    size(this, "Size", Vector3df(1,1,1)),
    collisionMeshId(this, "Collision mesh ref", ""),
    friction(this, "Friction", 0.5f),
    restitution(this, "Restitution", 0.0f),
    linearDamping(this, "Linear damping", 0.0f),
    angularDamping(this, "Angular damping", 0.0f),
    linearFactor(this, "Linear factor", Vector3df(1,1,1)),
    angularFactor(this, "Angular factor", Vector3df(1,1,1)),
    linearVelocity(this, "Linear velocity", Vector3df(0,0,0)),
    angularVelocity(this, "Angular velocity", Vector3df(0,0,0)),
    phantom(this, "Phantom", false),
    cachedShapeType_(-1),
    collision_mesh_tag_(0)
{
    static AttributeMetadata metadata;
    static bool metadataInitialized = false;
    if(!metadataInitialized)
    {
        metadata.enums[Shape_Box] = "Box";
        metadata.enums[Shape_Sphere] = "Sphere";
        metadata.enums[Shape_Cylinder] = "Cylinder";
        metadata.enums[Shape_Capsule] = "Capsule";
        metadata.enums[Shape_TriMesh] = "TriMesh";
        metadata.enums[Shape_HeightField] = "HeightField";
        metadata.enums[Shape_ConvexHull] = "ConvexHull";
        metadataInitialized = true;
    }
    shapeType.SetMetadata(&metadata);

    // Note: we cannot create the body yet because we are not in an entity/scene yet (and thus don't know what physics world we belong to)
    // We will create the body when the scene is known.
    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(UpdateSignals()));
    connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)),
        this, SLOT(AttributeUpdated(IAttribute*)));
    
    Foundation::EventManager *event_manager = framework_->GetEventManager().get();
    if(event_manager)
    {
        //event_manager->RegisterEventSubscriber(this, 99);
        resource_event_category_ = event_manager->QueryEventCategory("Resource");
        event_manager->RegisterEventSubscriber(this, resource_event_category_, Resource::Events::RESOURCE_READY);
    }
    else
    {
        LogWarning("Event manager was not valid.");
    }
}

EC_RigidBody::~EC_RigidBody()
{
    RemoveBody();
    RemoveCollisionShape();
}

bool EC_RigidBody::SetShapeFromVisibleMesh()
{
    Scene::Entity* parent = GetParentEntity();
    if (!parent)
        return false;
    EC_Mesh* mesh = parent->GetComponent<EC_Mesh>().get();
    if (!mesh)
        return false;
    mass.Set(0.0f, AttributeChange::Default);
    shapeType.Set(Shape_TriMesh, AttributeChange::Default);
    collisionMeshId.Set(mesh->meshResourceId.Get(), AttributeChange::Default);
    return true;
}

void EC_RigidBody::SetLinearVelocity(const Vector3df& velocity)
{
    linearVelocity.Set(velocity, AttributeChange::Default);
}

void EC_RigidBody::SetAngularVelocity(const Vector3df& velocity)
{
    angularVelocity.Set(velocity, AttributeChange::Default);
}

void EC_RigidBody::ApplyForce(const Vector3df& force, const Vector3df& position)
{
    if (!body_)
        CreateBody();
    if (body_)
    {
        if (position == Vector3df::ZERO)
            body_->applyCentralForce(ToBtVector3(force));
        else
            body_->applyForce(ToBtVector3(force), ToBtVector3(position));
    }
}

void EC_RigidBody::ApplyTorque(const Vector3df& torque)
{
    if (!body_)
        CreateBody();
    if (body_)
        body_->applyTorque(ToBtVector3(torque));
}

void EC_RigidBody::ApplyImpulse(const Vector3df& impulse, const Vector3df& position)
{
    if (!body_)
        CreateBody();
    if (body_)
    {
        if (position == Vector3df::ZERO)
            body_->applyCentralImpulse(ToBtVector3(impulse));
        else
            body_->applyImpulse(ToBtVector3(impulse), ToBtVector3(position));
    }
}

void EC_RigidBody::ApplyTorqueImpulse(const Vector3df& torqueImpulse)
{
    if (!body_)
        CreateBody();
    if (body_)
        body_->applyTorqueImpulse(ToBtVector3(torqueImpulse));
}

void EC_RigidBody::Activate()
{
    if (!body_)
        CreateBody();
    if (body_)
        body_->activate();
}

void EC_RigidBody::ResetForces()
{
    if (!body_)
        CreateBody();
    if (body_)
        body_->clearForces();
}

void EC_RigidBody::UpdateSignals()
{
    Scene::Entity* parent = GetParentEntity();
    if (!parent)
        return;
    
    connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), this, SLOT(CheckForPlaceableAndTerrain()));
    
    Scene::SceneManager* scene = parent->GetScene();
    world_ = owner_->GetPhysicsWorldForScene(scene);
}

void EC_RigidBody::CheckForPlaceableAndTerrain()
{
    Scene::Entity* parent = GetParentEntity();
    if (!parent)
        return;
    
    if (!placeable_.lock().get())
    {
        boost::shared_ptr<EC_Placeable> placeable = parent->GetComponent<EC_Placeable>();
        if (placeable)
        {
            placeable_ = placeable;
            connect(placeable.get(), SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(PlaceableUpdated(IAttribute*)));
        }
    }
    if (!terrain_.lock().get())
    {
        boost::shared_ptr<Environment::EC_Terrain> terrain = parent->GetComponent<Environment::EC_Terrain>();
        if (terrain)
        {
            terrain_ = terrain;
            connect(terrain.get(), SIGNAL(TerrainRegenerated()), this, SLOT(OnTerrainRegenerated()));
            connect(terrain.get(), SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(TerrainUpdated(IAttribute*)));
        }
    }
}

void EC_RigidBody::CreateCollisionShape()
{
    RemoveCollisionShape();
    
    Vector3df sizeVec = size.Get();
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
        shape_ = new btCylinderShapeZ(btVector3(sizeVec.x * 0.5f, sizeVec.y * 0.5f, sizeVec.z * 0.5f));
        break;
    case Shape_Capsule:
        shape_ = new btCapsuleShapeZ(sizeVec.x * 0.5f, sizeVec.z * 0.5f);
        break;
    case Shape_TriMesh:
        if (triangleMesh_)
            shape_ = new btBvhTriangleMeshShape(triangleMesh_.get(), true, true);
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
    if (heightField_)
    {
        delete heightField_;
        heightField_ = 0;
    }
}

void EC_RigidBody::CreateBody()
{
    if ((!world_) || (body_))
        return;
    
    CheckForPlaceableAndTerrain();
    
    btVector3 localInertia(0.0f, 0.0f, 0.0f);
    
    CreateCollisionShape();
    
    float m = mass.Get();
    if (m < 0.0f)
        m = 0.0f;
    // Trimesh shape can not move
    if (shapeType.Get() == Shape_TriMesh)
        m = 0.0f;
    if ((shape_) && (m > 0.0f))
        shape_->calculateLocalInertia(m, localInertia);
    
    bool isDynamic = m > 0.0f;
    bool isPhantom = phantom.Get();
    int collisionFlags = 0;
    if (!isDynamic)
        collisionFlags |= btCollisionObject::CF_STATIC_OBJECT;
    if (isPhantom)
        collisionFlags |= btCollisionObject::CF_NO_CONTACT_RESPONSE;
    
    body_ = new btRigidBody(m, this, shape_, localInertia);
    body_->setUserPointer(this);
    body_->setCollisionFlags(collisionFlags);
    world_->GetWorld()->addRigidBody(body_);
    body_->activate();
}

void EC_RigidBody::ReaddBody()
{
    if ((!body_) || (!world_))
        return;
    
    btVector3 localInertia(0.0f, 0.0f, 0.0f);
    float m = mass.Get();
    if (m < 0.0f)
        m = 0.0f;
    // Trimesh shape can not move
    if (shapeType.Get() == Shape_TriMesh)
        m = 0.0f;
    if ((shape_) && (m > 0.0f))
        shape_->calculateLocalInertia(m, localInertia);
    body_->setCollisionShape(shape_);
    body_->setMassProps(m, localInertia);
    
    bool isDynamic = m > 0.0f;
    bool isPhantom = phantom.Get();
    int collisionFlags = 0;
    if (!isDynamic)
        collisionFlags |= btCollisionObject::CF_STATIC_OBJECT;
    if (isPhantom)
        collisionFlags |= btCollisionObject::CF_NO_CONTACT_RESPONSE;
    body_->setCollisionFlags(collisionFlags);
    
    world_->GetWorld()->removeRigidBody(body_);
    world_->GetWorld()->addRigidBody(body_);
    body_->clearForces();
    body_->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
    body_->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
    body_->activate();
}

void EC_RigidBody::RemoveBody()
{
    if ((body_) && (world_))
    {
        world_->GetWorld()->removeRigidBody(body_);
        delete body_;
        body_ = 0;
    }
}

void EC_RigidBody::getWorldTransform(btTransform &worldTrans) const
{
    EC_Placeable* placeable = placeable_.lock().get();
    if (!placeable)
        return;
        
    const Transform& trans = placeable->transform.Get();
    const Vector3df& position = trans.position;
    Quaternion orientation(DEGTORAD * trans.rotation.x, DEGTORAD * trans.rotation.y, DEGTORAD * trans.rotation.z);
    
    worldTrans.setOrigin(ToBtVector3(position));
    worldTrans.setRotation(ToBtQuaternion(orientation));
}

void EC_RigidBody::setWorldTransform(const btTransform &worldTrans)
{
    EC_Placeable* placeable = placeable_.lock().get();
    if (!placeable)
        return;
    
    // Important: disconnect our own response to attribute changes to not create an endless loop!
    disconnected_ = true;
    
    // Set transform
    Vector3df position = ToVector3(worldTrans.getOrigin());
    Quaternion orientation = ToQuaternion(worldTrans.getRotation());
    Transform newTrans = placeable->transform.Get();
    Vector3df euler;
    orientation.toEuler(euler);
    newTrans.SetPos(position.x, position.y, position.z);
    newTrans.SetRot(euler.x * RADTODEG, euler.y * RADTODEG, euler.z * RADTODEG);
    placeable->transform.Set(newTrans, AttributeChange::Default);
    
    // Set linear & angular velocity
    if (body_)
    {
        linearVelocity.Set(ToVector3(body_->getLinearVelocity()), AttributeChange::Default);
        const btVector3& angular = body_->getAngularVelocity();
        angularVelocity.Set(Vector3df(angular.x() * RADTODEG, angular.y() * RADTODEG, angular.z() * RADTODEG), AttributeChange::Default);
    }
    
    disconnected_ = false;
}

void EC_RigidBody::OnTerrainRegenerated()
{
    if (shapeType.Get() == Shape_HeightField)
        CreateCollisionShape();
}

void EC_RigidBody::AttributeUpdated(IAttribute* attribute)
{
    if (disconnected_)
        return;
    
    bool requestMesh = false;
    
    if (attribute == &mass)
    {
        if (!body_)
            CreateBody();
        else
            // Readd body to the world in case static/dynamic classification changed
            ReaddBody();
    }
    
    if (attribute == &friction)
    {
        if (!body_)
            CreateBody();
        if (body_)
            body_->setFriction(friction.Get());
    }
    
    if (attribute == &restitution)
    {
        if (!body_)
            CreateBody();
        if (body_)
            body_->setRestitution(friction.Get());
    }
    
    if ((attribute == &linearDamping) || (attribute == &angularDamping))
    {
        if (!body_)
            CreateBody();
        if (body_)
            body_->setDamping(linearDamping.Get(), angularDamping.Get());
    }
    
    if (attribute == &linearFactor)
    {
        if (!body_)
            CreateBody();
        if (body_)
            body_->setLinearFactor(ToBtVector3(linearFactor.Get()));
    }
    
    if (attribute == &angularFactor)
    {
        if (!body_)
            CreateBody();
        if (body_)
            body_->setAngularFactor(ToBtVector3(angularFactor.Get()));
    }
    
    if ((attribute == &shapeType) || (attribute == &size))
    {
        if ((shapeType.Get() != cachedShapeType_) || (size.Get() != cachedSize_))
        {
            if ((shapeType.Get() != Shape_TriMesh) && (shapeType.Get() != Shape_ConvexHull))
            {
                CreateCollisionShape();
                cachedShapeType_ = shapeType.Get();
                cachedSize_ = size.Get();
            }
            else
                requestMesh = true;
        }
        if (!body_)
            CreateBody();
    }
    
    if ((attribute == &collisionMeshId) && ((shapeType.Get() == Shape_TriMesh) || (shapeType.Get() == Shape_ConvexHull)))
        requestMesh = true;
    
    if (attribute == &phantom)
    {
        if (!body_)
            CreateBody();
        else
            // Readd body to the world in case phantom classification changed
            ReaddBody();
    }
    
    if (attribute == &linearVelocity)
    {
        if (!body_)
            CreateBody();
        if (body_)
        {
            body_->setLinearVelocity(ToBtVector3(linearVelocity.Get()));
            body_->activate();
        }
    }
    
    if (attribute == &angularVelocity)
    {
        if (!body_)
            CreateBody();
        if (body_)
        {
            const Vector3df& angular = angularVelocity.Get();
            body_->setAngularVelocity(btVector3(angular.x * DEGTORAD, angular.y * DEGTORAD, angular.z * DEGTORAD));
            body_->activate();
        }
    }
    
    if (requestMesh)
    {
        std::string id = collisionMeshId.Get().toStdString();
        if (!id.empty())
        {
            // Do not create shape right now, but request the mesh resource
            boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
            if (renderer)
                collision_mesh_tag_ = renderer->RequestResource(id, OgreRenderer::OgreMeshResource::GetTypeStatic());
        }
    }
}

void EC_RigidBody::PlaceableUpdated(IAttribute* attribute)
{
    // Do not respond to our own change
    if ((disconnected_) || (!body_))
        return;
    
    EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(sender());
    if (attribute == &placeable->transform)
    {
        const Transform& trans = placeable->transform.Get();
        const Vector3df& position = trans.position;
        Quaternion orientation(DEGTORAD * trans.rotation.x, DEGTORAD * trans.rotation.y, DEGTORAD * trans.rotation.z);
        
        btTransform& worldTrans = body_->getWorldTransform();
        worldTrans.setOrigin(ToBtVector3(position));
        worldTrans.setRotation(ToBtQuaternion(orientation));
        
        // When we forcibly set the physics transform, also set the interpolation transform to prevent jerky motion
        btTransform interpTrans = body_->getInterpolationWorldTransform();
        interpTrans.setOrigin(worldTrans.getOrigin());
        interpTrans.setRotation(worldTrans.getRotation());
        body_->setInterpolationWorldTransform(interpTrans);
        
        body_->activate();
        
        UpdateScale();
    }
}

void EC_RigidBody::TerrainUpdated(IAttribute* attribute)
{
    Environment::EC_Terrain* terrain = terrain_.lock().get();
    if (!terrain)
        return;
    //! \todo It is suboptimal to regenerate the whole heightfield when just the terrain's transform changes
    if ((attribute == &terrain->nodeTransformation) && (shapeType.Get() == Shape_HeightField))
        CreateCollisionShape();
}

bool EC_RigidBody::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData *data)
{
    if(category_id == resource_event_category_)
    {
        if(event_id == Resource::Events::RESOURCE_READY)
        {
            Resource::Events::ResourceReady* event_data = checked_static_cast<Resource::Events::ResourceReady*>(data);
            if (event_data->tag_ == collision_mesh_tag_)
            {
                OgreRenderer::OgreMeshResource* res = checked_static_cast<OgreRenderer::OgreMeshResource*>(event_data->resource_.get());
                if (res)
                {
                    Ogre::Mesh* mesh = res->GetMesh().getPointer();
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
                return true;
            }
        }
    }
    return false;
}

void EC_RigidBody::UpdateScale()
{
   Vector3df sizeVec = size.Get();
    // Sanitize the size
    if (sizeVec.x < 0)
        sizeVec.x = 0;
    if (sizeVec.y < 0)
        sizeVec.y = 0;
    if (sizeVec.z < 0)
        sizeVec.z = 0;
    
    // If placeable exists, set local scaling from its scale
    /*! \todo Evil hack: we currently have an adjustment node for Ogre->OpenSim coordinate space conversion, but Ogre scaling of child nodes disregards the rotation,
     * so have to swap y/z axes here to have meaningful controls. Hopefully removed in the future.
     */
    EC_Placeable* placeable = placeable_.lock().get();
    if ((placeable) && (shape_))
    {
        const Transform& trans = placeable->transform.Get();
        // Trianglemesh does not have scaling of its own, so use the size
        if ((!triangleMesh_) || (shapeType.Get() != Shape_TriMesh))
            shape_->setLocalScaling(btVector3(trans.scale.x, trans.scale.z, trans.scale.y));
        else
            shape_->setLocalScaling(btVector3(sizeVec.x * trans.scale.x, sizeVec.y * trans.scale.z, sizeVec.z * trans.scale.y));
    }
}

void EC_RigidBody::CreateHeightFieldFromTerrain()
{
    CheckForPlaceableAndTerrain();
    
    Environment::EC_Terrain* terrain = terrain_.lock().get();
    if (!terrain)
        return;
    
    int width = terrain->PatchWidth() * Environment::EC_Terrain::cPatchSize;
    int height = terrain->PatchHeight() * Environment::EC_Terrain::cPatchSize;
    
    if ((!width) || (!height))
        return;
    
    heightValues_.resize(width * height);
    
    float xySpacing = 1.0f;
    float zSpacing = 1.0f;
    float minZ = 1000000000;
    float maxZ = -1000000000;
    for (uint y = 0; y < height; ++y)
    {
        for (uint x = 0; x < width; ++x)
        {
            float value = terrain->GetPoint(x, y);
            if (value < minZ)
                minZ = value;
            if (value > maxZ)
                maxZ = value;
            heightValues_[y * width + x] = value;
        }
    }
    
    Vector3df scale = terrain->nodeTransformation.Get().scale;
    Vector3df bbMin(0, 0, minZ);
    Vector3df bbMax(xySpacing * (width - 1), xySpacing * (height - 1), maxZ);
    Vector3df bbCenter = scale * (bbMin + bbMax) * 0.5f;
    
    heightField_ = new btHeightfieldTerrainShape(width, height, &heightValues_[0], zSpacing, minZ, maxZ, 2, PHY_FLOAT, false);
    
    /*! \todo EC_Terrain uses its own transform that is independent of the placeable. It is not nice to support, since rest of EC_RigidBody assumes
        the transform is in the placeable. Right now, we only support position & scaling. Here, we also counteract Bullet's nasty habit to center 
        the heightfield on its own. Also, Bullet's collisionshapes generally do not support arbitrary transforms, so we must construct a "compound shape"
        and add the heightfield as its child, to be able to specify the transform.
     */
    heightField_->setLocalScaling(ToBtVector3(scale));
    
    Vector3df positionAdjust = terrain->nodeTransformation.Get().position;
    positionAdjust += bbCenter;
    
    btCompoundShape* compound = new btCompoundShape();
    shape_ = compound;
    compound->addChildShape(btTransform(btQuaternion(0,0,0,1), ToBtVector3(positionAdjust)), heightField_);
}

void EC_RigidBody::CreateConvexHullSetShape()
{
    if (!convexHullSet_)
        return;
    btCompoundShape* compound = new btCompoundShape();
    shape_ = compound;
    for (uint i = 0; i < convexHullSet_->hulls_.size(); ++i)
        compound->addChildShape(btTransform(btQuaternion(0,0,0,1), ToBtVector3(convexHullSet_->hulls_[i].position_)), convexHullSet_->hulls_[i].hull_.get());
}

