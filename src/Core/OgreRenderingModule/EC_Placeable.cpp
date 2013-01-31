// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#define MATH_OGRE_INTEROP
#include "DebugOperatorNew.h"

#include "EC_Mesh.h"
#include "EC_Placeable.h"
#include "OgreRenderingModule.h"
#include "OgreWorld.h"
#include "Renderer.h"

#include "AttributeMetadata.h"
#include "EC_Mesh.h"
#include "Entity.h"
#include "Scene/Scene.h"
#include "Math/Quat.h"
#include "Math/float3x3.h"
#include "Math/float3x4.h"
#include "LoggingFunctions.h"

#include <Ogre.h>
#include <OgreTagPoint.h>

#include "MemoryLeakCheck.h"

using namespace OgreRenderer;

/** @cond PRIVATE */
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
                placeable->boneAttachmentNode_->_update(true, true);
            }
        }
    }
    
    void FirstFrameUpdate()
    {
        // Forcibly update on first frame to get approximate position in case the parent is offscreen, otherwise we'll be at world origin
        _updateFromParent();
        for (unsigned i = 0; i < placeables_.size(); ++i)
        {
            EC_Placeable* placeable = placeables_[i];
            if (placeable->boneAttachmentNode_)
            {
                placeable->boneAttachmentNode_->setPosition(_getDerivedPosition());
                placeable->boneAttachmentNode_->setOrientation(_getDerivedOrientation());
                placeable->boneAttachmentNode_->setScale(_getDerivedScale());
                placeable->boneAttachmentNode_->_update(true, true);
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
            i->second.tagPoint_->FirstFrameUpdate();
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
        tagPoint->placeables_.push_back(placeable);
        tagPoint->FirstFrameUpdate();
        
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
/** @endcond */

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
        Ogre::SceneManager* sceneMgr = world->OgreSceneManager();
        sceneNode_ = sceneMgr->createSceneNode(world->GetUniqueObjectName("EC_Placeable_SceneNode"));
// Would like to do this for improved debugging in the Profiler window, but because we don't have the parent entity yet, we don't know the id or the name of this entity.
//        sceneNode_ = sceneMgr->createSceneNode(world->GetUniqueObjectName(("EC_Placeable_SceneNode_" + QString::number(ParentEntity()->Id()) + "_" + ParentEntity()->Name()).toStdString()));

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
    Ogre::SceneManager* sceneMgr = world->OgreSceneManager();
    
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
        
        Ogre::SceneManager* sceneMgr = world->OgreSceneManager();
        Ogre::SceneNode* root_node = sceneMgr->getRootSceneNode();
        
        // Three possible cases
        // 1) attach to scene root node
        // 2) attach to another EC_Placeable's scene node
        // 3) attach to a bone on a skeletal mesh
        // Disconnect from the EntityCreated & ComponentAdded signals, as responding to them might not be needed anymore.
        // We will reconnect signals as necessary
        Entity* ownEntity = ParentEntity();
        Scene* scene = ownEntity ? ownEntity->ParentScene() : 0;
        if (scene)
            scene->disconnect(this, SLOT(CheckParentEntityCreated(Entity*, AttributeChange::Type)));
        if (ownEntity)
            ownEntity->disconnect(this, SLOT(OnComponentAdded(IComponent*, AttributeChange::Type)));
        
        // Try to attach to another entity if the parent ref is non-empty
        // Make sure we're not trying to attach to ourselves as the parent
        const EntityReference& parent = parentRef.Get();
        if (!parent.IsEmpty())
        {
            if (!ownEntity || !scene)
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
                            parentMesh->ForceSkeletonUpdate();
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
                            if (ViewEnabled())
                                LogWarning("EC_Placeable::AttachNode: Could not find bone " + boneName + " to attach to, attaching to the parent scene node instead.");
                            connect(parentMesh, SIGNAL(MeshChanged()), this, SLOT(OnParentMeshChanged()), Qt::UniqueConnection);
                            // While we wait, fall through to attaching to the scene node instead
                        }
                    }
                    else
                    {
                        // If can't find the mesh component yet, wait for it to be created
                        connect(parentEntity, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), this, SLOT(OnComponentAdded(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
                        return;
                    }
                }
                
                EC_Placeable* parentPlaceable = parentEntity->GetComponent<EC_Placeable>().get();
                if (parentPlaceable)
                {
                    // If we have a cyclic parenting attempt, attach to the root instead
                    EC_Placeable* parentCheck = parentPlaceable;
                    while (parentCheck)
                    {
                        if (parentCheck == this)
                        {
                            LogWarning("EC_Placeable::AttachNode: Cyclic scene node parenting attempt detected! Parenting to the scene root node instead.");
                            root_node->addChild(sceneNode_);
                            attached_ = true;
                            return;
                        }
                        parentCheck = parentCheck->parentPlaceable_;
                    }
                    
                    parentPlaceable_ = parentPlaceable;
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
        Ogre::SceneManager* sceneMgr = world->OgreSceneManager();
        Ogre::SceneNode* root_node = sceneMgr->getRootSceneNode();
        
        // Three possible cases
        // 1) attached to scene root node
        // 2) attached to another scene node
        // 3) attached to a bone via manual tracking
        if (parentBone_)
        {
            disconnect(parentMesh_, SIGNAL(MeshAboutToBeDestroyed()), this, SLOT(OnParentMeshDestroyed()));
            attachmentListener.RemoveAttachment(parentBone_, this);
            boneAttachmentNode_->removeChild(sceneNode_);
            parentBone_ = 0;
            parentMesh_ = 0;
        }
        else if (parentPlaceable_)
        {
            disconnect(parentPlaceable_, SIGNAL(AboutToBeDestroyed()), this, SLOT(OnParentPlaceableDestroyed()));
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
        shared_ptr<EC_Placeable> parentPlaceable = parent->GetComponent<EC_Placeable>();
        if (!parentPlaceable)
        {
            LogError("EC_Placeable::SetParent: Parenting entity " + parentEntity->ToString() + " to entity " + parent->ToString() + ", but the target entity does not have an EC_Placeable component!");
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
        shared_ptr<EC_Placeable> parentPlaceable = parent->GetComponent<EC_Placeable>();
        if (!parentPlaceable)
        {
            LogError("EC_Placeable::SetParent: Parenting entity " + parentEntity->ToString() + " to entity " + parent->ToString() + ", but the target entity does not have an EC_Placeable component!");
            return;
        }

        shared_ptr<EC_Mesh> mesh = parent->GetComponent<EC_Mesh>();
        if (!mesh)
        {
            LogError("EC_Placeable::SetParent: Parenting entity " + parentEntity->ToString() + " to a bone \"" + boneName + "\" of entity " + parent->ToString() + ", but the target entity does not have an EC_Mesh component!");
            return;
        }

        Ogre::Bone *parentBone = mesh->GetBone(boneName);
        if (!parentBone)
        {
            LogError("EC_Placeable::SetParent: Parenting entity " + parentEntity->ToString() + " to a bone \"" + boneName + "\" of entity " + parent->ToString() + " with mesh ref \"" + mesh->meshRef.Get().ref + "\" and skeleton ref \"" + mesh->skeletonRef.Get().ref + "\", but the target entity does not have a bone with that name!");
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
        shared_ptr<EC_Placeable> placeable = child->GetComponent<EC_Placeable>();
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
        LogInfo(parents[i]->ParentEntity()->ToString().toStdString() + " at local->parent: " + 
            parents[i]->LocalToParent().ToString() + ", world space: " + parents[i]->LocalToWorld().ToString());

    DumpChildHierarchy(this, 2);
}

Entity *EC_Placeable::ParentPlaceableEntity() const
{
    return parentRef.Get().Lookup(ParentScene()).get();
}

EC_Placeable *EC_Placeable::ParentPlaceableComponent() const
{
    return parentPlaceable_;
}

bool EC_Placeable::IsGrandparentOf(Entity *entity) const
{
    if (!entity)
    {
        LogError("EC_Placeable::IsGrandParentOf: called with null pointer.");
        return false;
    }
    if (!ParentEntity())
        return false;
    if (entity == ParentEntity())
        return true;

    EntityList allChildren = Grandchildren(ParentEntity());
    EntityList::const_iterator iter = std::find(allChildren.begin(), allChildren.end(), entity->shared_from_this());
    return iter != allChildren.end();
}

bool EC_Placeable::IsGrandparentOf(EC_Placeable *placeable) const
{
    assert(placeable->ParentEntity());
    return IsGrandparentOf(placeable->ParentEntity());
}

bool EC_Placeable::IsGrandchildOf(Entity *entity) const
{
    if (!entity)
    {
        LogError("EC_Placeable::IsGrandChildOf: called with null pointer.");
        return false;
    }
    if (!ParentEntity())
        return false;
    Entity *parentPlaceableEntity = ParentPlaceableEntity();
    if (!parentPlaceableEntity)
        return false;
    if (entity == ParentEntity())
        return true;

    EntityList allChildren = Grandchildren(ParentEntity());
    EntityList::const_iterator iter = std::find(allChildren.begin(), allChildren.end(), ParentEntity()->shared_from_this());
    return iter != allChildren.end();
}

bool EC_Placeable::IsGrandchildOf(EC_Placeable *placeable) const
{
    assert(placeable->ParentEntity());
    return IsGrandchildOf(placeable->ParentEntity());
}

EntityList EC_Placeable::Grandchildren(Entity *entity) const
{
    EntityList ret;
    if (!entity)
        return ret;
    if (!entity->GetComponent<EC_Placeable>())
        return ret;
    foreach(const EntityPtr &e, entity->GetComponent<EC_Placeable>()->Children())
    {
        EntityList grandchildren = Grandchildren(e.get());
        ret.merge(grandchildren);
    }
    return ret;
}

EntityList EC_Placeable::Children() const
{
    EntityList children;

    Scene *scene = ParentEntity()->ParentScene();
    ///\todo Optimize this function! The current implementation is very slow since it iterates through the whole scene! Instead, keep a list of weak_ptrs to child EC_Placeables.
    for(Scene::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        shared_ptr<EC_Placeable> placeable = iter->second->GetComponent<EC_Placeable>();
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

void EC_Placeable::AttributesChanged()
{
    // If parent ref or parent bone changed, reattach node to scene hierarchy
    if (parentRef.ValueChanged() || parentBone.ValueChanged())
        AttachNode();
    
    if (transform.ValueChanged())
    {
        transform.ClearChangedFlag();
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
    if (drawDebug.ValueChanged())
    {
        SetShowBoundingBoxRecursive(sceneNode_, drawDebug.Get());
    }
    if (visible.ValueChanged() && sceneNode_)
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
    if (!attached_ || !parentBone.Get().trimmed().isEmpty())
        AttachNode();
}

void EC_Placeable::OnComponentAdded(IComponent* component, AttributeChange::Type change)
{
    if (!attached_)
        AttachNode();
}

void EC_Placeable::SetPosition(float x, float y, float z)
{
    assume(isfinite(x));
    assume(isfinite(y));
    assume(isfinite(z));
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
    assume(q.IsNormalized());
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
    assume(tm.IsColOrthogonal());
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
    assume(tm.IsColOrthogonal());
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
    assume(tm.IsColOrthogonal());
    assume(!tm.HasNegativeScale());
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
        if (parentEntity)
            LogError("Parent for entity " + parentEntity->ToString() + " has an invalid world transform!");
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
    assume(orientation.IsNormalized());
    SetWorldTransform(float3x4(orientation, pos));
}

void EC_Placeable::SetWorldTransform(const Quat &orientation, const float3 &pos, const float3 &scale)
{
    assume(orientation.IsNormalized());
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
    // If we are parented to an Ogre bone, we can't (yet) compute the local-to-world matrix ourselves,
    // so query Ogre for the world matrix.
    if (!parentBone.Get().isEmpty() && sceneNode_)
        return float4x4(sceneNode_->_getFullTransform()).Float3x4Part();

    // Otherwise, compute the world matrix using our Tundra scene structures (not the Ogre scene structures, which can be out-of-date!)
    EC_Placeable *parentPlaceable = ParentPlaceableComponent();
    assert(parentPlaceable != this);
    float3x4 localToWorld = parentPlaceable ? (parentPlaceable->LocalToWorld() * LocalToParent()) : LocalToParent();

#ifdef _DEBUG
    // But confirm to detect oddities when/if these two don't match.
    if (sceneNode_)
    {
        float3x4 ogresViewOfLocalToWorld = float4x4(sceneNode_->_getFullTransform()).Float3x4Part();
        float3 t, s, t2, s2;
        Quat r, r2;
        localToWorld.Decompose(t, r, s);
        ogresViewOfLocalToWorld.Decompose(t2, r2, s2);
        if (!t.Equals(t2) || !s.Equals(s2) || !r.Equals(r2))
            LogDebug("Warning: Ogre SceneNode transform does not agree with Tundra Scenenode transform in EC_Placeable::LocalToWorld!");
    }
#endif

    return localToWorld;
}

float3x4 EC_Placeable::WorldToLocal() const
{
    float3x4 tm = LocalToWorld();
    bool success = tm.Inverse();
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
    bool success = tm.Inverse();
    assume(success);
    return tm;
}
