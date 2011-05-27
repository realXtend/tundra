// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_Mesh.h"
#include "EC_Placeable.h"
#include "OgreRenderingModule.h"
#include "OgreWorld.h"
#include "Renderer.h"

#include "AttributeMetadata.h"
#include "Entity.h"
#include "Scene.h"
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

Vector3df EC_Placeable::GetPosition() const
{
    const Ogre::Vector3& pos = sceneNode_->getPosition();
    return Vector3df(pos.x, pos.y, pos.z);
}

Quaternion EC_Placeable::GetOrientation() const
{
    const Ogre::Quaternion& orientation = sceneNode_->getOrientation();
    return Quaternion(orientation.x, orientation.y, orientation.z, orientation.w);
}

Vector3df EC_Placeable::GetScale() const
{
    const Ogre::Vector3& scale = sceneNode_->getScale();
    return Vector3df(scale.x, scale.y, scale.z);
}

Vector3df EC_Placeable::GetLocalXAxis() const
{
    const Ogre::Vector3& xaxis = sceneNode_->getOrientation().xAxis();
    return Vector3df(xaxis.x, xaxis.y, xaxis.z);
}

QVector3D EC_Placeable::GetQLocalXAxis() const
{
    Vector3df xaxis= GetLocalXAxis();
    return QVector3D(xaxis.x, xaxis.y, xaxis.z);
}

Vector3df EC_Placeable::GetLocalYAxis() const
{
    const Ogre::Vector3& yaxis = sceneNode_->getOrientation().yAxis();
    return Vector3df(yaxis.x, yaxis.y, yaxis.z);
}

QVector3D EC_Placeable::GetQLocalYAxis() const
{
    Vector3df yaxis= GetLocalYAxis();
    return QVector3D(yaxis.x, yaxis.y, yaxis.z);
}

Vector3df EC_Placeable::GetLocalZAxis() const
{
    const Ogre::Vector3& zaxis = sceneNode_->getOrientation().zAxis();
    return Vector3df(zaxis.x, zaxis.y, zaxis.z);
}

QVector3D EC_Placeable::GetQLocalZAxis() const
{
    Vector3df zaxis= GetLocalZAxis();
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
        newTransform.pos = Vector3df(resultPos.x, resultPos.y, resultPos.z);
        //newTransform.scale = Vector3df(resultScale.x, resultScale.y, resultScale.z);
        //Quaternion(resultRot.x, resultRot.y, resultRot.z, resultRot.w).toEuler(newTransform.rotation);
        transform.Set(newTransform, AttributeChange::Default);
        SetOrientation(Quaternion(resultRot.x, resultRot.y, resultRot.z, resultRot.w));
    }
}

void EC_Placeable::SetPosition(const Vector3df &pos)
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

void EC_Placeable::SetOrientation(const Quaternion& orientation)
{
    Transform newtrans = transform.Get();
    Vector3df result;
    orientation.toEuler(result);
    newtrans.SetRot(result.x * RADTODEG, result.y * RADTODEG, result.z * RADTODEG);
    transform.Set(newtrans, AttributeChange::Default);
}

void EC_Placeable::SetOrientation(const Vector3df& euler)
{
    SetOrientation(Quaternion(euler));
}

void EC_Placeable::LookAt(const Vector3df& look_at)
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

void EC_Placeable::SetScale(const Vector3df& newscale)
{
    sceneNode_->setScale(Ogre::Vector3(newscale.x, newscale.y, newscale.z));

    Transform newtrans = transform.Get();
    newtrans.SetScale(newscale.x, newscale.y, newscale.z);
    transform.Set(newtrans, AttributeChange::Default);
}

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
            Entity* ownEntity = GetParentEntity();
            if (!ownEntity)
                return;
            Scene* scene = ownEntity->GetScene();
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
    
    Vector3df eulers;
    q.toEuler(eulers);
    trans.rot.x = eulers.x * RADTODEG;
    trans.rot.y = eulers.y * RADTODEG;
    trans.rot.z = eulers.z * RADTODEG;
    transform.Set(trans, AttributeChange::Default);
    
    emit OrientationChanged(newort);
}

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

Vector3df EC_Placeable::GetRotationFromTo(const Vector3df& from, const Vector3df& to)
{
    Quaternion orientation;
    orientation.rotationFromTo(from,to);
    Vector3df result;
    orientation.toEuler(result);
    result *= RADTODEG;
    return result;
}

Vector3df EC_Placeable::GetWorldPosition() const
{
    const Ogre::Vector3& pos = sceneNode_->_getDerivedPosition();
    return Vector3df(pos.x, pos.y, pos.z);
}

Quaternion EC_Placeable::GetWorldOrientation() const
{
    const Ogre::Quaternion& orientation = sceneNode_->_getDerivedOrientation();
    return Quaternion(orientation.x, orientation.y, orientation.z, orientation.w);
}

Vector3df EC_Placeable::GetWorldOrientationEuler() const
{
    const Ogre::Quaternion& orientation = sceneNode_->_getDerivedOrientation();
    Quaternion q(orientation.x, orientation.y, orientation.z, orientation.w);
    Vector3df eulers;
    q.toEuler(eulers);
    eulers *= RADTODEG;
    return eulers;
}

Vector3df EC_Placeable::GetWorldScale() const
{
    const Ogre::Vector3& pos = sceneNode_->_getDerivedScale();
    return Vector3df(pos.x, pos.y, pos.z);
}

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

void EC_Placeable::RegisterActions()
{
    Entity *entity = GetParentEntity();
    assert(entity);
    if (entity)
    {
        // Generic actions
        entity->ConnectAction("ShowEntity", this, SLOT(Show()));
        entity->ConnectAction("HideEntity", this, SLOT(Hide()));
        entity->ConnectAction("ToggleEntity", this, SLOT(ToggleVisibility()));
    }
}

void EC_Placeable::Translate(const Vector3df& translation)
{
    Transform newTrans = transform.Get();
    newTrans.pos += translation;
    transform.Set(newTrans, AttributeChange::Default);
}

void EC_Placeable::TranslateRelative(const Vector3df& translation)
{
    Transform newTrans = transform.Get();
    newTrans.pos += GetOrientation() * translation;
    transform.Set(newTrans, AttributeChange::Default);
}

void EC_Placeable::TranslateWorldRelative(const Vector3df& translation)
{
    Transform newTrans = transform.Get();
    newTrans.pos += GetWorldOrientation() * translation;
    transform.Set(newTrans, AttributeChange::Default);
}

Vector3df EC_Placeable::GetRelativeVector(const Vector3df& vec)
{
    return GetOrientation() * vec;
}

Vector3df EC_Placeable::GetWorldRelativeVector(const Vector3df& vec)
{
    return GetWorldOrientation() * vec;
}

void EC_Placeable::HandleAttributeChanged(IAttribute* attribute, AttributeChange::Type change)
{
    // If parent ref or parent bone changed, reattach node to scene hierarchy
    if ((attribute == &parentRef) || (attribute == &parentBone))
        AttachNode();
    
    if (attribute == &transform)
    {
        const Transform& trans = transform.Get();
        if (trans.pos.IsFinite())
            sceneNode_->setPosition(trans.pos.x, trans.pos.y, trans.pos.z);
        
        Quaternion orientation(DEGTORAD * trans.rot.x,
                          DEGTORAD * trans.rot.y,
                          DEGTORAD * trans.rot.z);

        if (orientation.IsFinite())
            sceneNode_->setOrientation(Ogre::Quaternion(orientation.w, orientation.x, orientation.y, orientation.z));
        else
            ::LogError("EC_Placeable: transform attribute changed, but orientation not valid!");

        // Prevent Ogre exception from zero scale
        Vector3df scale(trans.scale.x, trans.scale.y, trans.scale.z);
        if (scale.x < 0.0000001f)
            scale.x = 0.0000001f;
        if (scale.y < 0.0000001f)
            scale.y = 0.0000001f;
        if (scale.z < 0.0000001f)
            scale.z = 0.0000001f;

        sceneNode_->setScale(scale.x, scale.y, scale.z);
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
        if (entity == parentRef.Get().Lookup(entity->GetScene()).get())
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
