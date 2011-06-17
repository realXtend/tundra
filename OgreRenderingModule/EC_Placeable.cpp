// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#define OGRE_INTEROP
#include "DebugOperatorNew.h"

#include "EC_Mesh.h"
#include "EC_Placeable.h"
#include "OgreRenderingModule.h"
#include "OgreWorld.h"
#include "Renderer.h"

#include "AttributeMetadata.h"
#include "Entity.h"
#include "Scene.h"
#include "Math/Quat.h"
#include "Math/float3x3.h"
#include "Math/float3x4.h"
#include "LoggingFunctions.h"

#include <Ogre.h>
#include <OgreTagPoint.h>

#include "MemoryLeakCheck.h"

using namespace OgreRenderer;

class CustomTagPoint : public Ogre::TagPoint
{
public:
    CustomTagPoint(Ogre::Entity* entity) :
        TagPoint(0, entity->getSkeleton())
    {
        setParentEntity(entity);
        setChildObject(0);
        setInheritOrientation(true);
        setInheritScale(true);
        setInheritParentEntityOrientation(true);
        setInheritParentEntityScale(true);
    }
    
    virtual void _update(bool updateChildren, bool parentHasChanged)
    {
        TagPoint::_update(updateChildren, parentHasChanged);
        
        // Update bone attached placeables manually, as Ogre does not support attaching scene nodes to bones
        for (unsigned i = 0; i < placeables_.size(); ++i)
        {
            EC_Placeable* placeable = placeables_[i];
            if (placeable->boneAttachmentNode_)
            {
                placeable->boneAttachmentNode_->setPosition(_getDerivedPosition());
                placeable->boneAttachmentNode_->setOrientation(_getDerivedOrientation());
                placeable->boneAttachmentNode_->setScale(_getDerivedScale());
            }
        }
    }

    std::vector<EC_Placeable*> placeables_;
};

struct BoneAttachment
{
    Ogre::Entity* entity_;
    Ogre::Bone* bone_;
    CustomTagPoint* tagPoint_;
};

class BoneAttachmentListener
{
public:
    void AddAttachment(Ogre::Entity* entity, Ogre::Bone* bone, EC_Placeable* placeable)
    {
        if (!entity || !bone || !placeable)
            return;
        
        // Check if attachment for this bone already exists
        std::map<Ogre::Bone*, BoneAttachment>::iterator i = attachments_.find(bone);
        if (i != attachments_.end())
        {
            i->second.tagPoint_->placeables_.push_back(placeable);
            return;
        }
        
        // Have to create a new entry
        BoneAttachment newEntry;
        newEntry.entity_ = entity;
        newEntry.bone_ = bone;
#include "DisableMemoryLeakCheck.h"
        CustomTagPoint* tagPoint = new CustomTagPoint(entity);
#include "EnableMemoryLeakCheck.h"
        newEntry.tagPoint_ = tagPoint;
        bone->addChild(tagPoint);
        
        if (placeable->boneAttachmentNode_)
        {
            placeable->boneAttachmentNode_->setPosition(tagPoint->_getDerivedPosition());
            placeable->boneAttachmentNode_->setOrientation(tagPoint->_getDerivedOrientation());
            placeable->boneAttachmentNode_->setScale(tagPoint->_getDerivedScale());
        }
        
        tagPoint->placeables_.push_back(placeable);
        
        attachments_[bone] = newEntry;
    }
    
    void RemoveAttachment(Ogre::Bone* bone, EC_Placeable* placeable)
    {
        std::map<Ogre::Bone*, BoneAttachment>::iterator i = attachments_.find(bone);
        if (i != attachments_.end())
        {
            std::vector<EC_Placeable*>& placeables = i->second.tagPoint_->placeables_;
            for (unsigned j = 0; j < placeables.size(); ++j)
            {
                if (placeables[j] == placeable)
                {
                    placeables.erase(placeables.begin() + j);
                    break;
                }
            }
            
            // If attachments for this bone became empty, remove the tagpoint and the whole entry
            if (placeables.empty())
            {
                bone->removeChild(i->second.tagPoint_);
#include "DisableMemoryLeakCheck.h"
                delete i->second.tagPoint_;
#include "EnableMemoryLeakCheck.h"
                attachments_.erase(i);
            }
        }
    }
    
    std::map<Ogre::Bone*, BoneAttachment> attachments_;
};

static BoneAttachmentListener attachmentListener;

void SetShowBoundingBoxRecursive(Ogre::SceneNode* node, bool enable)
{
    if (!node)
        return;
    node->showBoundingBox(enable);
    int numChildren = node->numChildren();
    for(int i = 0; i < numChildren; ++i)
    {
        Ogre::SceneNode* childNode = dynamic_cast<Ogre::SceneNode*>(node->getChild(i));
        if (childNode)
            SetShowBoundingBoxRecursive(childNode, enable);
    }
}

EC_Placeable::EC_Placeable(Scene* scene) :
    IComponent(scene),
    sceneNode_(0),
    boneAttachmentNode_(0),
    parentBone_(0),
    parentPlaceable_(0),
    parentMesh_(0),
    attached_(false),
    transform(this, "Transform"),
    drawDebug(this, "Show bounding box", false),
    visible(this, "Visible", true),
    selectionLayer(this, "Selection layer", 1),
    parentRef(this, "Parent entity ref", EntityReference()),
    parentBone(this, "Parent bone name", "")
{
    if (scene)
        world_ = scene->GetWorld<OgreWorld>();
    
    // Enable network interpolation for the transform
    static AttributeMetadata transAttrData;
    static AttributeMetadata nonDesignableAttrData;
    static bool metadataInitialized = false;
    if(!metadataInitialized)
    {
        transAttrData.interpolation = AttributeMetadata::Interpolate;
        nonDesignableAttrData.designable = false;
        metadataInitialized = true;
    }
    transform.SetMetadata(&transAttrData);

    OgreWorldPtr world = world_.lock();
    if (world)
    {
        Ogre::SceneManager* sceneMgr = world->GetSceneManager();
        sceneNode_ = sceneMgr->createSceneNode(world->GetUniqueObjectName("EC_Placeable_SceneNode"));
    
        // In case the placeable is used for camera control, set fixed yaw axis
        //sceneNode_->setFixedYawAxis(true, Ogre::Vector3::UNIT_Z);

        // Hook the transform attribute change
        connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)),
            SLOT(HandleAttributeChanged(IAttribute*, AttributeChange::Type)));

        connect(this, SIGNAL(ParentEntitySet()), SLOT(RegisterActions()));
    
        AttachNode();
    }
}

EC_Placeable::~EC_Placeable()
{
    if (world_.expired())
    {
        if (sceneNode_)
            LogError("EC_Placeable: World has expired, skipping uninitialization!");
        return;
    }
    
    emit AboutToBeDestroyed();
    
    OgreWorldPtr world = world_.lock();
    Ogre::SceneManager* sceneMgr = world->GetSceneManager();
    
    if (sceneNode_)
    {
        DetachNode();
        
        sceneMgr->destroySceneNode(sceneNode_);
        sceneNode_ = 0;
    }
    // Destroy the attachment node if it was created
    if (boneAttachmentNode_)
    {
        sceneMgr->getRootSceneNode()->removeChild(boneAttachmentNode_);
        sceneMgr->destroySceneNode(boneAttachmentNode_);
        boneAttachmentNode_ = 0;
    }
}
#if 0
float3 EC_Placeable::GetPosition() const
{
    const Ogre::Vector3& pos = sceneNode_->getPosition();
    return float3(pos.x, pos.y, pos.z);
}

Quat EC_Placeable::GetOrientation() const
{
    return (Quat)sceneNode_->getOrientation();
}

float3 EC_Placeable::GetScale() const
{
    const Ogre::Vector3& scale = sceneNode_->getScale();
    return float3(scale.x, scale.y, scale.z);
}

float3 EC_Placeable::GetLocalXAxis() const
{
    const Ogre::Vector3& xaxis = sceneNode_->getOrientation().xAxis();
    return float3(xaxis.x, xaxis.y, xaxis.z);
}

QVector3D EC_Placeable::GetQLocalXAxis() const
{
    float3 xaxis= GetLocalXAxis();
    return QVector3D(xaxis.x, xaxis.y, xaxis.z);
}

float3 EC_Placeable::GetLocalYAxis() const
{
    const Ogre::Vector3& yaxis = sceneNode_->getOrientation().yAxis();
    return float3(yaxis.x, yaxis.y, yaxis.z);
}

QVector3D EC_Placeable::GetQLocalYAxis() const
{
    float3 yaxis= GetLocalYAxis();
    return QVector3D(yaxis.x, yaxis.y, yaxis.z);
}

float3 EC_Placeable::GetLocalZAxis() const
{
    const Ogre::Vector3& zaxis = sceneNode_->getOrientation().zAxis();
    return float3(zaxis.x, zaxis.y, zaxis.z);
}

QVector3D EC_Placeable::GetQLocalZAxis() const
{
    float3 zaxis= GetLocalZAxis();
    return QVector3D(zaxis.x, zaxis.y, zaxis.z);
}

void EC_Placeable::ConvertToObjectSpace(IComponent *comp)
{
    if (comp && comp != this)
    {
        EC_Placeable *parentPlaceable = dynamic_cast<EC_Placeable*>(comp);
        if (!parentPlaceable)
        {
            LogError("ConvertToObjectSpace(): failed to dynamic cast given component into EC_Placeable.");
            return;
        }

        Ogre::SceneNode *parentNode = parentPlaceable->GetSceneNode();//ent->GetComponent("EC_Placeable").get())->GetSceneNode();
        Ogre::Vector3 resultPos = parentNode->convertWorldToLocalPosition(GetSceneNode()->getPosition());
        Ogre::Quaternion resultRot = parentNode->convertWorldToLocalOrientation(GetSceneNode()->getOrientation());
        //Ogre::Vector3 invScale = Ogre::Vector3(1.0,1.0,1.0) / parentNode->getScale();
        //Ogre::Vector3 resultScale = invScale * GetSceneNode()->getScale();

        Transform newTransform = transform.Get();
        newTransform.pos = float3(resultPos.x, resultPos.y, resultPos.z);
        //newTransform.scale = float3(resultScale.x, resultScale.y, resultScale.z);
        //Quaternion(resultRot.x, resultRot.y, resultRot.z, resultRot.w).toEuler(newTransform.rotation);
        transform.Set(newTransform, AttributeChange::Default);
        SetOrientation(Quat(resultRot.x, resultRot.y, resultRot.z, resultRot.w));
    }
}

void EC_Placeable::SetPosition(const float3 &pos)
{
   if (!pos.IsFinite())
   {
        LogError("EC_Placeable::SetPosition called with a vector that is not finite!");
        return;
   }
    Transform newtrans = transform.Get();
    newtrans.SetPos(pos.x, pos.y, pos.z);
    transform.Set(newtrans, AttributeChange::Default);
}

void EC_Placeable::SetOrientation(const Quat &orientation)
{
    Transform newtrans = transform.Get();
    float3 euler = orientation.ToEulerZYX() * RADTODEG; 
    newtrans.SetRot(euler.z, euler.y, euler.x);
    transform.Set(newtrans, AttributeChange::Default);
}

void EC_Placeable::SetOrientation(const float3& euler)
{
    SetOrientation(Quat::FromEulerZYX(euler.z, euler.y, euler.x));
}

void EC_Placeable::LookAt(const float3& look_at)
{
    // Don't rely on the stability of the lookat (since it uses previous orientation), 
    // so start in identity transform
    sceneNode_->setOrientation(Ogre::Quaternion::IDENTITY);
    sceneNode_->lookAt(Ogre::Vector3(look_at.x, look_at.y, look_at.z), Ogre::Node::TS_WORLD);
}

void EC_Placeable::SetYaw(float radians)
{
    sceneNode_->yaw(Ogre::Radian(radians), Ogre::Node::TS_WORLD);
}

void EC_Placeable::SetPitch(float radians)
{
    sceneNode_->pitch(Ogre::Radian(radians));
}

void EC_Placeable::SetRoll(float radians)
{
    sceneNode_->roll(Ogre::Radian(radians));
} 

float EC_Placeable::GetYaw() const
{
    const Ogre::Quaternion& orientation = sceneNode_->getOrientation();
    return orientation.getYaw().valueRadians();
}
float EC_Placeable::GetPitch() const
{
    const Ogre::Quaternion& orientation = sceneNode_->getOrientation();
    return orientation.getPitch().valueRadians();
}
float EC_Placeable::GetRoll() const
{
    const Ogre::Quaternion& orientation = sceneNode_->getOrientation();
    return orientation.getRoll().valueRadians();
}

void EC_Placeable::SetScale(const float3& newscale)
{
    sceneNode_->setScale(Ogre::Vector3(newscale.x, newscale.y, newscale.z));

    Transform newtrans = transform.Get();
    newtrans.SetScale(newscale.x, newscale.y, newscale.z);
    transform.Set(newtrans, AttributeChange::Default);
}
#endif
void EC_Placeable::AttachNode()
{
    if (world_.expired())
    {
        LogError("EC_Placeable::AttachNode: No OgreWorld available to call this function!");
        return;
    }
    OgreWorldPtr world = world_.lock();
    
    try
    {
        // If already attached, detach first
        if (attached_)
            DetachNode();
        
        Ogre::SceneManager* sceneMgr = world->GetSceneManager();
        Ogre::SceneNode* root_node = sceneMgr->getRootSceneNode();
        
        // Three possible cases
        // 1) attach to scene root node
        // 2) attach to another EC_Placeable's scene node
        // 3) attach to a bone on a skeletal mesh
        // Disconnect from the EntityCreated & ParentMeshChanged signals, as responding to them might not be needed anymore.
        // We will reconnect signals as necessary
        disconnect(this, SLOT(CheckParentEntityCreated(Entity*, AttributeChange::Type)));
        disconnect(this, SLOT(OnParentMeshChanged()));
        disconnect(this, SLOT(OnComponentAdded(IComponent*, AttributeChange::Type)));
        
        // Try to attach to another entity if the parent ref is non-empty
        // Make sure we're not trying to attach to ourselves as the parent
        const EntityReference& parent = parentRef.Get();
        if (!parent.IsEmpty())
        {
            Entity* ownEntity = ParentEntity();
            if (!ownEntity)
                return;
            Scene* scene = ownEntity->ParentScene();
            if (!scene)
                return;

            Entity* parentEntity = parent.Lookup(scene).get();
            if (parentEntity == ownEntity)
            {
                // If we refer to self, attach to the root
                root_node->addChild(sceneNode_);
                attached_ = true;
                return;
            }
            
            if (parentEntity)
            {
                // Note: if we don't find the correct bone, we attach to the root
                QString boneName = parentBone.Get();
                if (!boneName.isEmpty())
                {
                    EC_Mesh* parentMesh = parentEntity->GetComponent<EC_Mesh>().get();
                    if (parentMesh)
                    {
                        Ogre::Bone* bone = parentMesh->GetBone(boneName);
                        if (bone)
                        {
                            // Create the node for bone attachment if it did not exist already
                            if (!boneAttachmentNode_)
                            {
                                boneAttachmentNode_ = sceneMgr->createSceneNode(world->GetUniqueObjectName("EC_Placeable_BoneAttachmentNode"));
                                root_node->addChild(boneAttachmentNode_);
                            }
                            
                            // Setup manual bone tracking, as Ogre does not allow to attach scene nodes to bones
                            attachmentListener.AddAttachment(parentMesh->GetEntity(), bone, this);
                            
                            boneAttachmentNode_->addChild(sceneNode_);
                            
                            parentBone_ = bone;
                            parentMesh_ = parentMesh;
                            connect(parentMesh, SIGNAL(MeshAboutToBeDestroyed()), this, SLOT(OnParentMeshDestroyed()), Qt::UniqueConnection);
                            attached_ = true;
                            return;
                        }
                        else
                        {
                            // Could not find the bone. Connect to the parent mesh MeshChanged signal to wait for the proper mesh to be assigned.
                            connect(parentMesh, SIGNAL(MeshChanged()), this, SLOT(OnParentMeshChanged()), Qt::UniqueConnection);
                            return;
                        }
                    }
                    else
                    {
                        // If can't find the mesh component yet, wait for it to be created
                        connect(parentEntity, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), this, SLOT(OnComponentAdded(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
                        return;
                    }
                }
                
                parentPlaceable_ = parentEntity->GetComponent<EC_Placeable>().get();
                if (parentPlaceable_)
                {
                    parentPlaceable_->GetSceneNode()->addChild(sceneNode_);
                    
                    // Connect to destruction of the placeable to be able to detach gracefully
                    connect(parentPlaceable_, SIGNAL(AboutToBeDestroyed()), this, SLOT(OnParentPlaceableDestroyed()), Qt::UniqueConnection);
                    attached_ = true;
                    return;
                }
                else
                {
                    // If can't find the placeable component yet, wait for it to be created
                    connect(parentEntity, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), this, SLOT(OnComponentAdded(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
                    return;
                }
            }
            else
            {
                // Could not find parent entity. Check for it later, when new entities are created into the scene
                connect(scene, SIGNAL(EntityCreated(Entity*, AttributeChange::Type)), this, SLOT(CheckParentEntityCreated(Entity*, AttributeChange::Type)), Qt::UniqueConnection);
                return;
            }
        }
        
        root_node->addChild(sceneNode_);
        attached_ = true;
    }
    catch (Ogre::Exception& e)
    {
        LogError("EC_Placeable::AttachNode: Ogre exception " + std::string(e.what()));
        return;
    }
}

void EC_Placeable::DetachNode()
{
    if (world_.expired())
    {
        LogError("EC_Placeable::DetachNode: No OgreWorld available to call this function!");
        return;
    }
    OgreWorldPtr world = world_.lock();
    
    if (!attached_)
        return;
    
    try
    {
        Ogre::SceneManager* sceneMgr = world->GetSceneManager();
        Ogre::SceneNode* root_node = sceneMgr->getRootSceneNode();
        
        // Three possible cases
        // 1) attached to scene root node
        // 2) attached to another scene node
        // 3) attached to a bone via manual tracking
        if (parentBone_)
        {
            disconnect(parentMesh_, SIGNAL(MeshAboutToBeDestroyed()));
            attachmentListener.RemoveAttachment(parentBone_, this);
            boneAttachmentNode_->removeChild(sceneNode_);
            parentBone_ = 0;
            parentMesh_ = 0;
        }
        else if (parentPlaceable_)
        {
            disconnect(parentPlaceable_, SIGNAL(AboutToBeDestroyed()));
            parentPlaceable_->GetSceneNode()->removeChild(sceneNode_);
            parentPlaceable_ = 0;
        }
        else
            root_node->removeChild(sceneNode_);
        
        attached_ = false;
    }
    catch (Ogre::Exception& e)
    {
        LogError("EC_Placeable::DetachNode: Ogre exception " + std::string(e.what()));
    }
}
#if 0
//experimental QVector3D acessors
QVector3D EC_Placeable::GetQPosition() const
{
    const Transform& trans = transform.Get();
    return QVector3D(trans.pos.x, trans.pos.y, trans.pos.z);
}

void EC_Placeable::SetQPosition(QVector3D newpos)
{
    Transform trans = transform.Get();
    trans.pos.x = newpos.x();
    trans.pos.y = newpos.y();
    trans.pos.z = newpos.z();
    transform.Set(trans, AttributeChange::Default);
    emit PositionChanged(newpos);
}

/*
QQuaternion EC_Placeable::GetQOrientation() const 
{
    const Transform& trans = transform.Get();
    Quaternion orientation(DEGTORAD * trans.rot.x,
                      DEGTORAD * trans.rot.y,
                      DEGTORAD * trans.rot.z);
    return QQuaternion(orientation.w, orientation.x, orientation.y, orientation.z);
}

void EC_Placeable::SetQOrientation(QQuaternion newort)
{
    Transform trans = transform.Get();
    
    Quaternion q(newort.x(), newort.y(), newort.z(), newort.scalar());
    
    float3 eulers;
    q.toEuler(eulers);
    trans.rot.x = eulers.x * RADTODEG;
    trans.rot.y = eulers.y * RADTODEG;
    trans.rot.z = eulers.z * RADTODEG;
    transform.Set(trans, AttributeChange::Default);
    
    emit OrientationChanged(newort);
}
*/
QVector3D EC_Placeable::GetQScale() const
{
    const Transform& trans = transform.Get();
    return QVector3D(trans.scale.x, trans.scale.y, trans.scale.z);
}

void EC_Placeable::SetQScale(QVector3D newscale)
{
    Transform trans = transform.Get();
    trans.scale.x = newscale.x();
    trans.scale.y = newscale.y();
    trans.scale.z = newscale.z();
    transform.Set(trans, AttributeChange::Default);
    emit ScaleChanged(newscale);
}
/*
void EC_Placeable::SetQOrientationEuler(QVector3D newrot)
{
    Transform trans = transform.Get();
    trans.rot.x = newrot.x();
    trans.rot.y = newrot.y();
    trans.rot.z = newrot.z();
    transform.Set(trans, AttributeChange::Default);
    
    Quaternion orientation(DEGTORAD * newrot.x(),
                      DEGTORAD * newrot.y(),
                      DEGTORAD * newrot.z());
    emit OrientationChanged(QQuaternion(orientation.w, orientation.x, orientation.y, orientation.z));
}

QVector3D EC_Placeable::GetQOrientationEuler() const
{
    const Transform& trans = transform.Get();
    return QVector3D(trans.rot.x, trans.rot.y, trans.rot.z);
}
*/

/*
float3 EC_Placeable::GetRotationFromTo(const float3& from, const float3& to)
{
    Quat orientation;
    orientation.rotationFromTo(from,to);
    float3 result;
    orientation.toEuler(result);
    result *= RADTODEG;
    return result;
}
*/

float3 EC_Placeable::GetWorldPosition() const
{
    const Ogre::Vector3& pos = sceneNode_->_getDerivedPosition();
    return float3(pos.x, pos.y, pos.z);
}

Quaternion EC_Placeable::GetWorldOrientation() const
{
    const Ogre::Quaternion& orientation = sceneNode_->_getDerivedOrientation();
    return Quaternion(orientation.x, orientation.y, orientation.z, orientation.w);
}

float3 EC_Placeable::GetWorldOrientationEuler() const
{
    const Ogre::Quaternion& orientation = sceneNode_->_getDerivedOrientation();
    Quaternion q(orientation.x, orientation.y, orientation.z, orientation.w);
    float3 eulers;
    q.toEuler(eulers);
    eulers *= RADTODEG;
    return eulers;
}

float3 EC_Placeable::GetWorldScale() const
{
    const Ogre::Vector3& pos = sceneNode_->_getDerivedScale();
    return float3(pos.x, pos.y, pos.z);
}
#endif
void EC_Placeable::Show()
{
    if (!sceneNode_)
        return;

    sceneNode_->setVisible(true);
}

void EC_Placeable::Hide()
{
    if (!sceneNode_)
        return;

    sceneNode_->setVisible(false);
}

void EC_Placeable::ToggleVisibility()
{
    if (!sceneNode_)
        return;

    sceneNode_->flipVisibility();
}

void EC_Placeable::SetParent(Entity *parent, bool preserveWorldTransform)
{
    float3x4 desiredTransform = (preserveWorldTransform ? LocalToWorld() : transform.Get().ToFloat3x4());

    if (!parent)
    {
        EntityReference r;
        r.Set(0);
        parentRef.Set(r, AttributeChange::Default);
    }
    else
    {
        boost::shared_ptr<EC_Placeable> parentPlaceable = parent->GetComponent<EC_Placeable>();
        if (!parentPlaceable)
        {
            LogError("EC_Placeable::SetParent: Parenting entity " + parentEntity_->ToString() + " to entity " + parent->ToString() + ", but the target entity does not have an EC_Placeable component!");
            return;
        }
        if (preserveWorldTransform)
            desiredTransform = parentPlaceable->WorldToLocal() * desiredTransform;
        parentRef.Set(EntityReference(parent->Id()), AttributeChange::Default);
    }
    // Not attaching to a bone, clear any previous bone ref.
    parentBone.Set("", AttributeChange::Default);

    if (preserveWorldTransform)
        transform.Set(Transform(desiredTransform), AttributeChange::Default);
}

void EC_Placeable::SetParent(Entity *parent, QString boneName, bool preserveWorldTransform)
{
    float3x4 desiredTransform = (preserveWorldTransform ? LocalToWorld() : transform.Get().ToFloat3x4());

    if (!parent)
    {
        EntityReference r;
        r.Set(0);
        parentRef.Set(r, AttributeChange::Default);
    }
    else
    {
        boost::shared_ptr<EC_Placeable> parentPlaceable = parent->GetComponent<EC_Placeable>();
        if (!parentPlaceable)
        {
            LogError("EC_Placeable::SetParent: Parenting entity " + parentEntity_->ToString() + " to entity " + parent->ToString() + ", but the target entity does not have an EC_Placeable component!");
            return;
        }

        boost::shared_ptr<EC_Mesh> mesh = parent->GetComponent<EC_Mesh>();
        if (!mesh)
        {
            LogError("EC_Placeable::SetParent: Parenting entity " + parentEntity_->ToString() + " to a bone \"" + boneName + "\" of entity " + parent->ToString() + ", but the target entity does not have an EC_Mesh component!");
            return;
        }

        Ogre::Bone *parentBone = mesh->GetBone(boneName);
        if (!parentBone)
        {
            LogError("EC_Placeable::SetParent: Parenting entity " + parentEntity_->ToString() + " to a bone \"" + boneName + "\" of entity " + parent->ToString() + " with mesh ref \"" + mesh->meshRef.Get().ref + "\" and skeleton ref \"" + mesh->skeletonRef.Get().ref + "\", but the target entity does not have a bone with that name!");
            return;
        }
        if (preserveWorldTransform)
            desiredTransform = float4x4(parentBone->_getFullTransform()).Float3x4Part() * desiredTransform;
        parentRef.Set(EntityReference(parent->Id()), AttributeChange::Default);
    }
    // Not attaching to a bone, clear any previous bone ref.
    parentBone.Set(boneName, AttributeChange::Default);

    if (preserveWorldTransform)
        transform.Set(Transform(desiredTransform), AttributeChange::Default);
}

void DumpChildHierarchy(EC_Placeable *placeable, int indent)
{
    EntityList children = placeable->Children();

    std::string indentStr = "";
    for(int i = 0; i < indent; ++i)
        indentStr = indentStr + " ";

    foreach(EntityPtr child, children)
    {
        boost::shared_ptr<EC_Placeable> placeable = child->GetComponent<EC_Placeable>();
        LogInfo(indentStr + child->ToString().toStdString() + " at local->parent: " + placeable->LocalToParent().ToString() + ", world space: " + placeable->LocalToWorld().ToString());
        DumpChildHierarchy(placeable.get(), indent + 2);
    }
}

void EC_Placeable::DumpNodeHierarhy()
{
    std::vector<EC_Placeable*> parents;
    parents.push_back(this);
    // Print all parents
    EC_Placeable *p = this;
    while(p->parentPlaceable_)
    {
        parents.push_back(p->parentPlaceable_);
        p = p->parentPlaceable_;
    }

    for(int i = parents.size()-1; i >= 0; --i)
        LogInfo(parents[i]->ParentEntity()->ToString().toStdString() + " at local->parent: " + parents[i]->LocalToParent().ToString() + ", world space: " + parents[i]->LocalToWorld().ToString());

    DumpChildHierarchy(this, 2);
}

EntityList EC_Placeable::Children()
{
    EntityList children;

    Scene *scene = ParentEntity()->ParentScene();
    ///\todo Optimize this function! The current implementation is very slow since it iterates through the whole scene! Instead, keep a list of weak_ptrs to child EC_Placeables.
    for(Scene::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        boost::shared_ptr<EC_Placeable> placeable = iter->second->GetComponent<EC_Placeable>();
        if (placeable)
        {
            EntityPtr parent = placeable->parentRef.Get().Lookup(scene);
            if (parent.get() == this->ParentEntity())
                children.push_back(iter->second);
        }
    }
    return children;
}

void EC_Placeable::RegisterActions()
{
    Entity *entity = ParentEntity();
    assert(entity);
    if (entity)
    {
        // Generic actions
        entity->ConnectAction("ShowEntity", this, SLOT(Show()));
        entity->ConnectAction("HideEntity", this, SLOT(Hide()));
        entity->ConnectAction("ToggleEntity", this, SLOT(ToggleVisibility()));
    }
}
#if 0
void EC_Placeable::Translate(const float3& translation)
{
    Transform newTrans = transform.Get();
    newTrans.pos += translation;
    transform.Set(newTrans, AttributeChange::Default);
}

void EC_Placeable::TranslateRelative(const float3& translation)
{
    Transform newTrans = transform.Get();
    newTrans.pos += GetOrientation() * translation;
    transform.Set(newTrans, AttributeChange::Default);
}

void EC_Placeable::TranslateWorldRelative(const float3& translation)
{
    Transform newTrans = transform.Get();
    newTrans.pos += GetWorldOrientation() * translation;
    transform.Set(newTrans, AttributeChange::Default);
}

float3 EC_Placeable::GetRelativeVector(const float3& vec)
{
    return GetOrientation() * vec;
}

float3 EC_Placeable::GetWorldRelativeVector(const float3& vec)
{
    return GetWorldOrientation() * vec;
}
#endif
void EC_Placeable::HandleAttributeChanged(IAttribute* attribute, AttributeChange::Type change)
{
    // If parent ref or parent bone changed, reattach node to scene hierarchy
    if ((attribute == &parentRef) || (attribute == &parentBone))
        AttachNode();
    
    if (attribute == &transform)
    {
        const Transform& trans = transform.Get();
        if (trans.pos.IsFinite())
            sceneNode_->setPosition(trans.pos);
        
        Quat orientation = trans.Orientation();

        if (orientation.IsFinite())
            sceneNode_->setOrientation(orientation);
        else
            ::LogError("EC_Placeable: transform attribute changed, but orientation not valid!");

        // Prevent Ogre exception from zero scale
        float3 scale = trans.scale;
        if (scale.x < 0.0000001f)
            scale.x = 0.0000001f;
        if (scale.y < 0.0000001f)
            scale.y = 0.0000001f;
        if (scale.z < 0.0000001f)
            scale.z = 0.0000001f;

        sceneNode_->setScale(scale);
    }
    else if (attribute == &drawDebug)
    {
        SetShowBoundingBoxRecursive(sceneNode_, drawDebug.Get());
    }
    else if (attribute == &visible && sceneNode_)
        sceneNode_->setVisible(visible.Get());
}

void EC_Placeable::OnParentMeshDestroyed()
{
    DetachNode();
    // Connect to the mesh component setting a new mesh; we might (re)find the proper bone then
    connect(sender(), SIGNAL(MeshChanged()), this, SLOT(OnParentMeshChanged()), Qt::UniqueConnection);
}

void EC_Placeable::OnParentPlaceableDestroyed()
{
    DetachNode();
}

void EC_Placeable::CheckParentEntityCreated(Entity* entity, AttributeChange::Type change)
{
    if ((!attached_) && (entity))
    {
        // Check if the entity is the one we should use as parent
        if (entity == parentRef.Get().Lookup(entity->ParentScene()).get())
            AttachNode();
    }
}

void EC_Placeable::OnParentMeshChanged()
{
    if (!attached_)
        AttachNode();
}

void EC_Placeable::OnComponentAdded(IComponent* component, AttributeChange::Type change)
{
    if (!attached_)
        AttachNode();
}

void EC_Placeable::SetPosition(float x, float y, float z)
{
    Transform newtrans = transform.Get();
    newtrans.SetPos(x, y, z);
    transform.Set(newtrans, AttributeChange::Default);
}

void EC_Placeable::SetPosition(const float3 &pos)
{
    SetPosition(pos.x, pos.y, pos.z);
}

void EC_Placeable::SetOrientation(const Quat &q)
{
    Transform newtrans = transform.Get();
    newtrans.SetOrientation(q);
    transform.Set(newtrans, AttributeChange::Default);
}

void EC_Placeable::SetScale(float x, float y, float z)
{
    SetScale(float3(x,y,z));
}

void EC_Placeable::SetScale(const float3 &scale)
{
    Transform newtrans = transform.Get();
    newtrans.SetScale(scale);
    transform.Set(newtrans, AttributeChange::Default);
}

void EC_Placeable::SetOrientationAndScale(const float3x3 &tm)
{
    assume(tm.IsOrthogonal());
    assume(!tm.HasNegativeScale());
    Transform newtrans = transform.Get();
    newtrans.SetRotationAndScale(tm);
    transform.Set(newtrans, AttributeChange::Default);
}

void EC_Placeable::SetOrientationAndScale(const Quat &q, const float3 &scale)
{
    SetOrientationAndScale(q.ToFloat3x3() * float3x3::Scale(scale));
}

void EC_Placeable::SetTransform(const float3x3 &tm, const float3 &pos)
{
    SetTransform(float3x4(tm, pos));
}

void EC_Placeable::SetTransform(const float3x4 &tm)
{
    assume(tm.IsOrthogonal());
    assume(!tm.HasNegativeScale());
    float3 translate;
    Quat rotate;
    float3 scale;
    tm.Decompose(translate, rotate, scale);
    SetPosition(translate);
    SetOrientation(rotate);
    SetScale(scale);
}

void EC_Placeable::SetTransform(const Quat &orientation, const float3 &pos)
{
    SetTransform(float3x4(orientation, pos));
}

void EC_Placeable::SetTransform(const Quat &orientation, const float3 &pos, const float3 &scale)
{
    SetTransform(float3x4::FromTRS(pos, orientation, scale));
}

void EC_Placeable::SetTransform(const float4x4 &tm)
{
    assume(tm.Row(3).Equals(float4(0,0,0,1)));
    SetTransform(tm.Float3x4Part());
}

void EC_Placeable::SetWorldTransform(const float3x3 &tm, const float3 &pos)
{
    SetWorldTransform(float3x4(tm, pos));
}

void EC_Placeable::SetWorldTransform(const float3x4 &tm)
{
    if (!parentPlaceable_) // No parent, the local->parent transform equals the local->world transform.
    {
        SetTransform(tm);
        return;
    }

    float3x4 parentWorldTransform = float3x4::identity;

    if (parentBone_)
        parentWorldTransform = float4x4(parentBone_->_getFullTransform()).Float3x4Part();
    else
        parentWorldTransform = parentPlaceable_->LocalToWorld();

    bool success = parentWorldTransform.Inverse();
    if (!success)
    {
        if (parentEntity_)
            LogError("Parent for entity " + parentEntity_->ToString() + " has an invalid world transform!");
        else
            LogError("Parent for a detached entity has an invalid world transform!");
        return;
    }
        
    SetTransform(parentWorldTransform * tm);
}

void EC_Placeable::SetWorldTransform(const float4x4 &tm)
{
    assume(tm.Row(3).Equals(float4(0,0,0,1)));
    SetWorldTransform(tm.Float3x4Part());
}

void EC_Placeable::SetWorldTransform(const Quat &orientation, const float3 &pos)
{
    SetWorldTransform(float3x4(orientation, pos));
}

void EC_Placeable::SetWorldTransform(const Quat &orientation, const float3 &pos, const float3 &scale)
{
    SetWorldTransform(float3x4::FromTRS(pos, orientation, scale));
}

float3 EC_Placeable::WorldPosition() const
{
    return LocalToWorld().TranslatePart();
}

Quat EC_Placeable::WorldOrientation() const
{
    float3 translate;
    Quat rotate;
    float3 scale;
    LocalToWorld().Decompose(translate, rotate, scale);
    return rotate;
}

float3 EC_Placeable::WorldScale() const
{
    float3 translate;
    Quat rotate;
    float3 scale;
    LocalToWorld().Decompose(translate, rotate, scale);
    return scale;
}

float3 EC_Placeable::Position() const
{
    return transform.Get().pos;
}

Quat EC_Placeable::Orientation() const
{
    return transform.Get().Orientation();
}

float3 EC_Placeable::Scale() const
{
    return transform.Get().scale;
}

float3x4 EC_Placeable::LocalToWorld() const
{
    if (sceneNode_)
        return ((float4x4)sceneNode_->_getFullTransform()).Float3x4Part();
    else
        return float3x4::identity;
}

float3x4 EC_Placeable::WorldToLocal() const
{
    float3x4 tm = LocalToWorld();
    bool success = tm.InverseOrthogonal();
    assume(success);
    return tm;
}

float3x4 EC_Placeable::LocalToParent() const
{
    return transform.Get().ToFloat3x4();
}

float3x4 EC_Placeable::ParentToLocal() const
{
    float3x4 tm = transform.Get().ToFloat3x4();
    bool success = tm.InverseOrthogonal();
    assume(success);
    return tm;
}
