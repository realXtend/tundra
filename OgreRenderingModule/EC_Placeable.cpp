// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "Entity.h"
#include "SceneManager.h"
#include "RexNetworkUtils.h"
#include "LoggingFunctions.h"

#include <Ogre.h>
#include <OgreTagPoint.h>

DEFINE_POCO_LOGGING_FUNCTIONS("EC_Placeable")

using namespace OgreRenderer;

Scene::Entity* LookupParentEntity(Scene::SceneManager* scene, const QString& parent)
{
    // The parent can be either an entity ID or name. Try ID first
    bool ok = false;
    entity_id_t parentID = parent.toInt(&ok);
    if (ok)
    {
        Scene::Entity* parentEnt = scene->GetEntity(parentID).get();
        if (parentEnt)
            return parentEnt;
    }
    // Then try name if ID number was not valid or was not found
    return scene->GetEntityByName(parent).get();
}

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
    for (int i = 0; i < numChildren; ++i)
    {
        Ogre::SceneNode* childNode = dynamic_cast<Ogre::SceneNode*>(node->getChild(i));
        if (childNode)
            SetShowBoundingBoxRecursive(childNode, enable);
    }
}

EC_Placeable::EC_Placeable(IModule* module) :
    IComponent(module->GetFramework()),
    renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
    scene_node_(0),
    link_scene_node_(0),
    boneAttachmentNode_(0),
    parentBone_(0),
    parentPlaceable_(0),
    parentMesh_(0),
    attached_(false),
    select_priority_(0),
    transform(this, "Transform"),
    drawDebug(this, "Show bounding box", false),
    visible(this, "Visible", true),
    parentRef(this, "Parent entity ref", ""),
    parentBone(this, "Parent bone name", "")
{
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

    RendererPtr renderer = renderer_.lock();
    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    link_scene_node_ = scene_mgr->createSceneNode(renderer->GetUniqueObjectName("EC_Placeable_LinkSceneNode"));
    scene_node_ = scene_mgr->createSceneNode(renderer->GetUniqueObjectName("EC_Placeable_SceneNode"));
    link_scene_node_->addChild(scene_node_);
    
    // In case the placeable is used for camera control, set fixed yaw axis
    link_scene_node_->setFixedYawAxis(true, Ogre::Vector3::UNIT_Z);

    // Hook the transform attribute change
    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)),
        SLOT(HandleAttributeChanged(IAttribute*, AttributeChange::Type)));

    connect(this, SIGNAL(ParentEntitySet()), SLOT(RegisterActions()));
    
    AttachNode();
}

EC_Placeable::~EC_Placeable()
{
    if (renderer_.expired())
        return;

    emit AboutToBeDestroyed();

    RendererPtr renderer = renderer_.lock();
    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    
    if (scene_node_ && link_scene_node_)
    {
        link_scene_node_->removeChild(scene_node_);
    }
    
    if (scene_node_)
    {
        scene_mgr->destroySceneNode(scene_node_);
        scene_node_ = 0;
    }
    
    if (link_scene_node_)
    {
        DetachNode();
        
        scene_mgr->destroySceneNode(link_scene_node_);
        link_scene_node_ = 0;
    }
}

void EC_Placeable::AttachNode()
{
    RendererPtr renderer = renderer_.lock();
    if (!renderer)
        return;
    Ogre::SceneManager* sceneMgr = renderer->GetSceneManager();
    
    try
    {
        // If already attached, detach first
        if (attached_)
            DetachNode();
        
        Ogre::SceneNode* root_node = sceneMgr->getRootSceneNode();
        
        // Three possible cases
        // 1) attach to scene root node
        // 2) attach to another EC_Placeable's scene node
        // 3) attach to a bone on a skeletal mesh
        // Disconnect from the EntityCreated & ParentMeshChanged signals, as responding to them might not be needed anymore.
        // We will reconnect signals as necessary
        disconnect(this, SLOT(CheckParentEntityCreated(Scene::Entity*, AttributeChange::Type)));
        disconnect(this, SLOT(OnParentMeshChanged()));
        disconnect(this, SLOT(OnComponentAdded(IComponent*, AttributeChange::Type)));
        
        // Try to attach to another entity if the parent ref is non-empty
        // Make sure we're not trying to attach to ourselves as the parent
        QString parent = parentRef.Get().trimmed();
        if (!parent.isEmpty())
        {
            Scene::Entity* ownEntity = GetParentEntity();
            if (!ownEntity)
                return;
            Scene::SceneManager* scene = ownEntity->GetScene();
            if (!scene)
                return;

            Scene::Entity* parentEntity = LookupParentEntity(scene, parent);
            
            if (parentEntity == ownEntity)
            {
                // If we refer to self, attach to the root
                root_node->addChild(link_scene_node_);
                attached_ = true;
                return;
            }
            
            if (parentEntity)
            {
                // Note: if we don't find the correct bone, we attach to the root
                QString boneName = parentBone.Get().trimmed();
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
                                boneAttachmentNode_ = sceneMgr->createSceneNode(renderer->GetUniqueObjectName("EC_Placeable_BoneAttachmentNode"));
                                root_node->addChild(boneAttachmentNode_);
                            }
                            
                            // Setup manual bone tracking, as Ogre does not allow to attach scene nodes to bones
                            attachmentListener.AddAttachment(parentMesh->GetEntity(), bone, this);
                            
                            boneAttachmentNode_->addChild(link_scene_node_);
                            
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
                    parentPlaceable_->GetSceneNode()->addChild(link_scene_node_);
                    
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
                connect(scene, SIGNAL(EntityCreated(Entity*, AttributeChange::Type)), this, SLOT(CheckParentEntityCreated(Scene::Entity*, AttributeChange::Type)), Qt::UniqueConnection);
                return;
            }
        }
        
        root_node->addChild(link_scene_node_);
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
    RendererPtr renderer = renderer_.lock();
    if (!renderer)
        return;
    Ogre::SceneManager* sceneMgr = renderer->GetSceneManager();
    
    if (!attached_)
        return;
    
    try
    {
        Ogre::SceneNode* root_node = sceneMgr->getRootSceneNode();
        
        // Three possible cases
        // 1) attached to scene root node
        // 2) attached to another scene node
        // 3) attached to a bone via manual tracking
        if (parentBone_)
        {
            disconnect(parentMesh_, SIGNAL(MeshAboutToBeDestroyed()));
            attachmentListener.RemoveAttachment(parentBone_, this);
            boneAttachmentNode_->removeChild(link_scene_node_);
            parentBone_ = 0;
            parentMesh_ = 0;
        }
        else if (parentPlaceable_)
        {
            disconnect(parentPlaceable_, SIGNAL(AboutToBeDestroyed()));
            parentPlaceable_->GetSceneNode()->removeChild(link_scene_node_);
            parentPlaceable_ = 0;
        }
        else
            root_node->removeChild(link_scene_node_);
        
        attached_ = false;
    }
    catch (Ogre::Exception& e)
    {
        LogError("EC_Placeable::DetachNode: Ogre exception " + std::string(e.what()));
    }
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

void EC_Placeable::CheckParentEntityCreated(Scene::Entity* entity, AttributeChange::Type change)
{
    if ((!attached_) && (entity))
    {
        // Check if the entity is the one we should use as parent
        if (entity == LookupParentEntity(entity->GetScene(), parentRef.Get().trimmed()))
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

void EC_Placeable::SetParent(ComponentPtr placeable)
{
    // Deprecated no-op
    LogError("SetParent is deprecated and no longer supported");
}

Vector3df EC_Placeable::GetPosition() const
{
    const Ogre::Vector3& pos = link_scene_node_->getPosition();
    return Vector3df(pos.x, pos.y, pos.z);
}

Quaternion EC_Placeable::GetOrientation() const
{
    const Ogre::Quaternion& orientation = link_scene_node_->getOrientation();
    return Quaternion(orientation.x, orientation.y, orientation.z, orientation.w);
}

Vector3df EC_Placeable::GetScale() const
{
    const Ogre::Vector3& scale = scene_node_->getScale();
    return Vector3df(scale.x, scale.y, scale.z);
}

Vector3df EC_Placeable::GetLocalXAxis() const
{
    const Ogre::Vector3& xaxis = link_scene_node_->getOrientation().xAxis();
    return Vector3df(xaxis.x, xaxis.y, xaxis.z);
}

QVector3D EC_Placeable::GetQLocalXAxis() const
{
    Vector3df xaxis= GetLocalXAxis();
    return QVector3D(xaxis.x, xaxis.y, xaxis.z);
}

Vector3df EC_Placeable::GetLocalYAxis() const
{
    const Ogre::Vector3& yaxis = link_scene_node_->getOrientation().yAxis();
    return Vector3df(yaxis.x, yaxis.y, yaxis.z);
}

QVector3D EC_Placeable::GetQLocalYAxis() const
{
    Vector3df yaxis= GetLocalYAxis();
    return QVector3D(yaxis.x, yaxis.y, yaxis.z);
}

Vector3df EC_Placeable::GetLocalZAxis() const
{
    const Ogre::Vector3& zaxis = link_scene_node_->getOrientation().zAxis();
    return Vector3df(zaxis.x, zaxis.y, zaxis.z);
}

QVector3D EC_Placeable::GetQLocalZAxis() const
{
    Vector3df zaxis= GetLocalZAxis();
    return QVector3D(zaxis.x, zaxis.y, zaxis.z);
}

void EC_Placeable::SetPosition(const Vector3df &pos)
{
   if (!pos.IsFinite())
   {
        LogError("EC_Placeable::SetPosition called with a vector that is not finite!");
        return;
   }
    // link_scene_node_->setPosition(Ogre::Vector3(pos.x, pos.y, pos.z));
    Transform newtrans = transform.Get();
    newtrans.SetPos(pos.x, pos.y, pos.z);
    transform.Set(newtrans, AttributeChange::Default);
}

void EC_Placeable::SetOrientation(const Quaternion& orientation)
{
    // link_scene_node_->setOrientation(Ogre::Quaternion(orientation.w, orientation.x, orientation.y, orientation.z));
    Transform newtrans = transform.Get();
    Vector3df result;
    orientation.toEuler(result);
    newtrans.SetRot(result.x * RADTODEG, result.y * RADTODEG, result.z * RADTODEG);
    transform.Set(newtrans, AttributeChange::Default);
}

void EC_Placeable::LookAt(const Vector3df& look_at)
{
    // Don't rely on the stability of the lookat (since it uses previous orientation), 
    // so start in identity transform
    link_scene_node_->setOrientation(Ogre::Quaternion::IDENTITY);
    link_scene_node_->lookAt(Ogre::Vector3(look_at.x, look_at.y, look_at.z), Ogre::Node::TS_WORLD);
}

void EC_Placeable::SetYaw(float radians)
{
    link_scene_node_->yaw(Ogre::Radian(radians), Ogre::Node::TS_WORLD);
}

void EC_Placeable::SetPitch(float radians)
{
    link_scene_node_->pitch(Ogre::Radian(radians));
}

void EC_Placeable::SetRoll(float radians)
{
    link_scene_node_->roll(Ogre::Radian(radians));
} 

float EC_Placeable::GetYaw() const
{
    const Ogre::Quaternion& orientation = link_scene_node_->getOrientation();
    return orientation.getYaw().valueRadians();
}
float EC_Placeable::GetPitch() const
{
    const Ogre::Quaternion& orientation = link_scene_node_->getOrientation();
    return orientation.getPitch().valueRadians();
}
float EC_Placeable::GetRoll() const
{
    const Ogre::Quaternion& orientation = link_scene_node_->getOrientation();
    return orientation.getRoll().valueRadians();
}

void EC_Placeable::SetScale(const Vector3df& newscale)
{
    scene_node_->setScale(Ogre::Vector3(newscale.x, newscale.y, newscale.z));
    // AttachNode(); // Nodes become visible only after having their position set at least once

    Transform newtrans = transform.Get();
    newtrans.SetScale(newscale.x, newscale.y, newscale.z);
    transform.Set(newtrans, AttributeChange::Default);
}

//experimental QVector3D acessors
QVector3D EC_Placeable::GetQPosition() const
{
    const Transform& trans = transform.Get();
    return QVector3D(trans.position.x, trans.position.y, trans.position.z);
}

void EC_Placeable::SetQPosition(QVector3D newpos)
{
    Transform trans = transform.Get();
    trans.position.x = newpos.x();
    trans.position.y = newpos.y();
    trans.position.z = newpos.z();
    transform.Set(trans, AttributeChange::Default);
    emit PositionChanged(newpos);
}


QQuaternion EC_Placeable::GetQOrientation() const 
{
    const Transform& trans = transform.Get();
    Quaternion orientation(DEGTORAD * trans.rotation.x,
                      DEGTORAD * trans.rotation.y,
                      DEGTORAD * trans.rotation.z);
    return QQuaternion(orientation.w, orientation.x, orientation.y, orientation.z);
}

void EC_Placeable::SetQOrientation(QQuaternion newort)
{
    Transform trans = transform.Get();
    
    Quaternion q(newort.x(), newort.y(), newort.z(), newort.scalar());
    
    Vector3df eulers;
    q.toEuler(eulers);
    trans.rotation.x = eulers.x * RADTODEG;
    trans.rotation.y = eulers.y * RADTODEG;
    trans.rotation.z = eulers.z * RADTODEG;
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
    trans.rotation.x = newrot.x();
    trans.rotation.y = newrot.y();
    trans.rotation.z = newrot.z();
    transform.Set(trans, AttributeChange::Default);
    
    Quaternion orientation(DEGTORAD * newrot.x(),
                      DEGTORAD * newrot.y(),
                      DEGTORAD * newrot.z());
    emit OrientationChanged(QQuaternion(orientation.w, orientation.x, orientation.y, orientation.z));
}

QVector3D EC_Placeable::GetQOrientationEuler() const
{
    const Transform& trans = transform.Get();
    return QVector3D(trans.rotation.x, trans.rotation.y, trans.rotation.z);
}

QVector3D EC_Placeable::translate(int axis, float amount)
{
    Ogre::Matrix3 m;
    Ogre::Vector3 v;
    float x, y, z;
    x = y = z = 0.0;
    m.SetColumn(0,  link_scene_node_->getOrientation().xAxis());
    m.SetColumn(1,  link_scene_node_->getOrientation().yAxis());
    m.SetColumn(2,  link_scene_node_->getOrientation().zAxis());
    switch(axis) {
        case 0:
            x = amount;
            break;
        case 1:
            y = amount;
            break;
        case 2:
            z = amount;
            break;
        default:
            // nothing, don't translate
            break;

    }
    link_scene_node_->translate(m, Ogre::Vector3(x, y, z), Ogre::Node::TS_LOCAL);
    const Ogre::Vector3 newpos = link_scene_node_->getPosition();
    return QVector3D(newpos.x, newpos.y, newpos.z);
}

void EC_Placeable::HandleAttributeChanged(IAttribute* attribute, AttributeChange::Type change)
{
    // If parent ref or parent bone changed, reattach node to scene hierarchy
    if ((attribute == &parentRef) || (attribute == &parentBone))
        AttachNode();

    if (attribute == &transform)
    {
        if (!link_scene_node_ || !scene_node_)
            return;
        
        const Transform& trans = transform.Get();
        if (trans.position.IsFinite())
        {
            link_scene_node_->setPosition(trans.position.x, trans.position.y, trans.position.z);
        }
        
        Quaternion orientation(DEGTORAD * trans.rotation.x,
                          DEGTORAD * trans.rotation.y,
                          DEGTORAD * trans.rotation.z);

        if (orientation.IsFinite())
            link_scene_node_->setOrientation(Ogre::Quaternion(orientation.w, orientation.x, orientation.y, orientation.z));
        else
            OgreRenderingModule::LogError("EC_Placeable: transform attribute changed, but orientation not valid!");

        // Prevent Ogre exception from zero scale
        Vector3df scale(trans.scale.x, trans.scale.y, trans.scale.z);
        if (scale.x < 0.0000001f)
            scale.x = 0.0000001f;
        if (scale.y < 0.0000001f)
            scale.y = 0.0000001f;
        if (scale.z < 0.0000001f)
            scale.z = 0.0000001f;

        scene_node_->setScale(scale.x, scale.y, scale.z);
    }
    else if (attribute == &drawDebug)
    {
        SetShowBoundingBoxRecursive(link_scene_node_, drawDebug.Get());
    }
    else if (attribute == &visible && link_scene_node_)
        link_scene_node_->setVisible(visible.Get());
    /*
    else if(attribute == &position)
    {
        if (!link_scene_node_)
            return;
        QVector3D newPosition = position.Get();
        link_scene_node_->setPosition(newPosition.x(), newPosition.y(), newPosition.z());
        AttachNode();
    }
    else if(attribute == &scale)
    {
        if (!link_scene_node_)
            return;
        QVector3D newScale = scale.Get();
        scene_node_->setScale(newScale.x(), newScale.y(), newScale.z());
        AttachNode();
    }
    */
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

void EC_Placeable::Show()
{
    if (!link_scene_node_)
        return;

    link_scene_node_->setVisible(true);
}

void EC_Placeable::Hide()
{
    if (!link_scene_node_)
        return;	

    link_scene_node_->setVisible(false);
}

void EC_Placeable::ToggleVisibility()
{
    if (!link_scene_node_)
        return;

    link_scene_node_->flipVisibility();
}

void EC_Placeable::RegisterActions()
{
    Scene::Entity *entity = GetParentEntity();
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
    newTrans.position += translation;
    transform.Set(newTrans, AttributeChange::Default);
}

void EC_Placeable::TranslateRelative(const Vector3df& translation)
{
    Transform newTrans = transform.Get();
    newTrans.position += GetOrientation() * translation;
    transform.Set(newTrans, AttributeChange::Default);
}

Vector3df EC_Placeable::GetRelativeVector(const Vector3df& vec)
{
    return GetOrientation() * vec;
}
