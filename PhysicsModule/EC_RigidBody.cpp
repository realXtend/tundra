// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_RigidBody.h"
#include "EC_Placeable.h"
#include "PhysicsModule.h"
#include "PhysicsUtils.h"
#include "PhysicsWorld.h"
#include "EventManager.h"
#include "RexTypes.h"
#include "Renderer.h"
#include "OgreMeshResource.h"
#include "ServiceManager.h"

#include "btBulletDynamicsCommon.h"

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_RigidBody");

using namespace Physics;
using namespace RexTypes;

EC_RigidBody::EC_RigidBody(IModule* module) :
    IComponent(module->GetFramework()),
    body_(0),
    world_(0),
    shape_(0),
    placeableDisconnected_(false),
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

void EC_RigidBody::UpdateSignals()
{
    Scene::Entity* parent = GetParentEntity();
    if (!parent)
        return;
    
    connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), this, SLOT(CheckForPlaceable()));
    
    Scene::SceneManager* scene = parent->GetScene();
    world_ = owner_->GetPhysicsWorldForScene(scene);
}

void EC_RigidBody::CheckForPlaceable()
{
    if (placeable_.lock().get())
        return;
    Scene::Entity* parent = GetParentEntity();
    if (!parent)
        return;
    boost::shared_ptr<EC_Placeable> placeable = parent->GetComponent<EC_Placeable>();
    if (placeable)
    {
        placeable_ = placeable;
        connect(placeable.get(), SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(PlaceableUpdated(IAttribute*)));
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
}

void EC_RigidBody::CreateBody()
{
    if ((!world_) || (body_))
        return;
    
    CheckForPlaceable();
    
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
    
    body_ = new btRigidBody(m, this, shape_, localInertia);
    body_->setUserPointer(this);
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
    
    // Important: disconnect our own response to the attribute change update to not create an endless loop!
    placeableDisconnected_ = true;
    
    Vector3df position = ToVector3(worldTrans.getOrigin());
    Quaternion orientation = ToQuaternion(worldTrans.getRotation());
    
    Transform newTrans = placeable->transform.Get();
    Vector3df euler;
    orientation.toEuler(euler);
    newTrans.SetPos(position.x, position.y, position.z);
    newTrans.SetRot(euler.x * RADTODEG, euler.y * RADTODEG, euler.z * RADTODEG);
    placeable->transform.Set(newTrans, AttributeChange::Default);
    
    placeableDisconnected_ = false;
}

void EC_RigidBody::AttributeUpdated(IAttribute* attribute)
{
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
            if (shapeType.Get() != Shape_TriMesh)
            {
                CreateCollisionShape();
                cachedShapeType_ = shapeType.Get();
                cachedSize_ = size.Get();
            }
            else
                requestMesh = true;
        }
    }
    
    if ((attribute == &collisionMeshId) && (shapeType.Get() == Shape_TriMesh))
        requestMesh = true;
    
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
    if ((placeableDisconnected_) || (!body_))
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
                        triangleMesh_ = owner_->GetTriangleMeshFromOgreMesh(mesh);
                        CreateCollisionShape();
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

