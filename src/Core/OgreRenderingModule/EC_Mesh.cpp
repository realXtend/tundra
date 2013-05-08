// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#define MATH_OGRE_INTEROP
#include "DebugOperatorNew.h"
#include "OgreRenderingModule.h"
#include "OgreWorld.h"
#include "Renderer.h"
#include "Entity.h"
#include "Scene/Scene.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "OgreSkeletonAsset.h"
#include "OgreMeshAsset.h"
#include "OgreMaterialAsset.h"
#include "IAssetTransfer.h"
#include "AssetAPI.h"
#include "AttributeMetadata.h"
#include "Profiler.h"
#include "Math/float2.h"
#include "Geometry/Ray.h"

#include <Ogre.h>
#include <OgreTagPoint.h>

#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

using namespace OgreRenderer;

EC_Mesh::EC_Mesh(Scene* scene) :
    IComponent(scene),
    nodeTransformation(this, "Transform", Transform(float3(0,0,0),float3(0,0,0),float3(1,1,1))),
    meshRef(this, "Mesh ref", AssetReference("", "OgreMesh")),
    skeletonRef(this, "Skeleton ref", AssetReference("", "OgreSkeleton")),
    meshMaterial(this, "Mesh materials", AssetReferenceList("OgreMaterial")),
    drawDistance(this, "Draw distance", 0.0f),
    castShadows(this, "Cast shadows", false),
    entity_(0),
    attached_(false)
{
    if (scene)
        world_ = scene->GetWorld<OgreWorld>();

    static AttributeMetadata drawDistanceData("", "0", "10000");
    drawDistance.SetMetadata(&drawDistanceData);

    static AttributeMetadata materialMetadata;
    materialMetadata.elementType = "assetreference";
    meshMaterial.SetMetadata(&materialMetadata);

    meshAsset = AssetRefListenerPtr(new AssetRefListener());
    skeletonAsset = AssetRefListenerPtr(new AssetRefListener());
    
    OgreWorldPtr world = world_.lock();
    if (world)
    {
        Ogre::SceneManager* sceneMgr = world->OgreSceneManager();
        adjustment_node_ = sceneMgr->createSceneNode(world->GetUniqueObjectName("EC_Mesh_adjustment_node"));

        connect(this, SIGNAL(ParentEntitySet()), SLOT(UpdateSignals()));
        connect(meshAsset.get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnMeshAssetLoaded(AssetPtr)), Qt::UniqueConnection);
        connect(skeletonAsset.get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnSkeletonAssetLoaded(AssetPtr)), Qt::UniqueConnection);
    }
}

EC_Mesh::~EC_Mesh()
{
    if (world_.expired())
    {
        // Log error only if there was an Ogre object to be destroyed
        if (entity_)
            LogError("EC_Mesh: World has expired, skipping uninitialization!");
        return;
    }
    OgreWorldPtr world = world_.lock();

    RemoveMesh();

    if (adjustment_node_)
    {
        Ogre::SceneManager* sceneMgr = world->OgreSceneManager();
        sceneMgr->destroySceneNode(adjustment_node_);
        adjustment_node_ = 0;
    }
}

void EC_Mesh::SetPlaceable(const ComponentPtr &placeable)
{
    if (placeable && !dynamic_pointer_cast<EC_Placeable>(placeable))
    {
        LogError("Attempted to set placeable which is not " + EC_Placeable::TypeNameStatic());
        return;
    }
    
    DetachEntity();
    placeable_ = placeable;
    AttachEntity();
}

void EC_Mesh::SetPlaceable(EC_Placeable* placeable)
{
    SetPlaceable(placeable->shared_from_this());
}

void EC_Mesh::AutoSetPlaceable()
{
    Entity* entity = ParentEntity();
    if (entity)
    {
        ComponentPtr placeable = entity->GetComponent(EC_Placeable::TypeNameStatic());
        if (placeable)
            SetPlaceable(placeable);
    }
}

void EC_Mesh::SetAdjustPosition(const float3& position)
{
    Transform transform = nodeTransformation.Get();
    transform.SetPos(position.x, position.y, position.z);
    nodeTransformation.Set(transform, AttributeChange::Default);
}

void EC_Mesh::SetAdjustOrientation(const Quat &orientation)
{
    Transform transform = nodeTransformation.Get();
    transform.SetOrientation(orientation);
    nodeTransformation.Set(transform, AttributeChange::Default);
}

void EC_Mesh::SetAdjustScale(const float3& scale)
{
    Transform transform = nodeTransformation.Get();
    transform.SetScale(scale);
    nodeTransformation.Set(transform, AttributeChange::Default);
}

void EC_Mesh::SetAttachmentPosition(uint index, const float3& position)
{
    if (index >= attachment_nodes_.size() || attachment_nodes_[index] == 0)
        return;
    
    attachment_nodes_[index]->setPosition(position);
}

void EC_Mesh::SetAttachmentOrientation(uint index, const Quat &orientation)
{
    if (index >= attachment_nodes_.size() || attachment_nodes_[index] == 0)
        return;
    
    attachment_nodes_[index]->setOrientation(orientation);
}

void EC_Mesh::SetAttachmentScale(uint index, const float3& scale)
{
    if (index >= attachment_nodes_.size() || attachment_nodes_[index] == 0)
        return;
    
    attachment_nodes_[index]->setScale(scale);
}

float3 EC_Mesh::AdjustPosition() const
{
    Transform transform = nodeTransformation.Get();
    return transform.pos;
}

Quat EC_Mesh::AdjustOrientation() const
{
    Transform transform = nodeTransformation.Get();
    return transform.Orientation();
}

float3 EC_Mesh::AdjustScale() const
{
    Transform transform = nodeTransformation.Get();
    return transform.scale;
}

float3 EC_Mesh::AttachmentPosition(uint index) const
{
    if (index >= attachment_nodes_.size() || attachment_nodes_[index] == 0)
        return float3::nan;

    return attachment_nodes_[index]->getPosition();
}

Quat EC_Mesh::AttachmentOrientation(uint index) const
{
    if (index >= attachment_nodes_.size() || attachment_nodes_[index] == 0)
        return Quat::nan;
        
    return attachment_nodes_[index]->getOrientation();
}

float3 EC_Mesh::AttachmentScale(uint index) const
{
    if (index >= attachment_nodes_.size() || attachment_nodes_[index] == 0)
        return float3::nan;

    return attachment_nodes_[index]->getScale();
}

float3x4 EC_Mesh::LocalToParent() const
{
    if (!entity_)
    {
        LogError(QString("EC_Mesh::LocalToParent failed! No entity exists in mesh \"%1\" (entity: \"%2\")!").arg(meshRef.Get().ref).arg(ParentEntity() ? ParentEntity()->Name() : "(EC_Mesh with no parent entity)"));
        return float3x4::identity;
    }

    Ogre::SceneNode *node = entity_->getParentSceneNode();
    if (!node)
    {
        LogError(QString("EC_Mesh::LocalToParent failed! Ogre::Entity is not attached to a Ogre::SceneNode! Mesh \"%1\" (entity: \"%2\")!").arg(meshRef.Get().ref).arg(ParentEntity() ? ParentEntity()->Name() : "(EC_Mesh with no parent entity)"));
        return float3x4::identity;
    }

    return float3x4::FromTRS(node->getPosition(), node->getOrientation(), node->getScale());
}

float3x4 EC_Mesh::LocalToWorld() const
{
    if (!entity_)
    {
        LogError(QString("EC_Mesh::LocalToWorld failed! No entity exists in mesh \"%1\" (entity: \"%2\")!").arg(meshRef.Get().ref).arg(ParentEntity() ? ParentEntity()->Name() : "(EC_Mesh with no parent entity)"));
        return float3x4::identity;
    }

    Ogre::SceneNode *node = entity_->getParentSceneNode();
    if (!node)
    {
        LogError(QString("EC_Mesh::LocalToWorld failed! Ogre::Entity is not attached to a Ogre::SceneNode! Mesh \"%1\" (entity: \"%2\")!").arg(meshRef.Get().ref).arg(ParentEntity() ? ParentEntity()->Name() : "(EC_Mesh with no parent entity)"));
        return float3x4::identity;
    }

    assume(!float3(node->_getDerivedScale()).IsZero());
    float3x4 tm = float3x4::FromTRS(node->_getDerivedPosition(), node->_getDerivedOrientation(), node->_getDerivedScale());
    assume(tm.IsColOrthogonal());
    return tm;
}

bool EC_Mesh::SetMesh(QString meshResourceName, bool clone)
{
    if (!ViewEnabled())
        return false;
    
    OgreWorldPtr world = world_.lock();

    std::string mesh_name = meshResourceName.trimmed().toStdString();

    RemoveMesh();

    // If placeable is not set yet, set it manually by searching it from the parent entity
    if (!placeable_)
    {
        Entity* entity = ParentEntity();
        if (entity)
        {
            ComponentPtr placeable = entity->GetComponent(EC_Placeable::TypeNameStatic());
            if (placeable)
                placeable_ = placeable;
        }
    }
    
    Ogre::SceneManager* sceneMgr = world->OgreSceneManager();
    
    Ogre::Mesh* mesh = PrepareMesh(mesh_name, clone);
    if (!mesh)
        return false;
    
    try
    {
        entity_ = sceneMgr->createEntity(world->GetUniqueObjectName("EC_Mesh_entity"), mesh->getName());
        if (!entity_)
        {
            LogError("EC_Mesh::SetMesh: Could not set mesh " + mesh_name);
            return false;
        }
        
        entity_->setRenderingDistance(drawDistance.Get());
        entity_->setCastShadows(castShadows.Get());
        entity_->setUserAny(Ogre::Any(static_cast<IComponent *>(this)));
        // Set UserAny also on subentities
        for(uint i = 0; i < entity_->getNumSubEntities(); ++i)
            entity_->getSubEntity(i)->setUserAny(entity_->getUserAny());

        if (entity_->hasSkeleton())
        {
            Ogre::SkeletonInstance* skel = entity_->getSkeleton();
            // Enable cumulative mode on skeletal animations
            if (skel)
                skel->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);
        }
        
        // Make sure adjustment node is uptodate
        Transform newTransform = nodeTransformation.Get();
        adjustment_node_->setPosition(newTransform.pos);
        adjustment_node_->setOrientation(newTransform.Orientation());
        
        // Prevent Ogre exception from zero scale
        adjustment_node_->setScale(Max(newTransform.scale, float3::FromScalar(0.0000001f)));
            
        // Force a re-apply of all materials to this new mesh.
        ApplyMaterial();
    }
    catch(const Ogre::Exception& e)
    {
        LogError("EC_Mesh::SetMesh: Could not set mesh " + mesh_name + ": " + std::string(e.what()));
        return false;
    }
    
    AttachEntity();
    emit MeshChanged();
    
    return true;
}

bool EC_Mesh::SetMeshWithSkeleton(const std::string& mesh_name, const std::string& skeleton_name, bool clone)
{
    if (!ViewEnabled())
        return false;
    OgreWorldPtr world = world_.lock();

    Ogre::SkeletonPtr skel = Ogre::SkeletonManager::getSingleton().getByName(AssetAPI::SanitateAssetRef(skeleton_name));
    if (skel.isNull())
    {
        LogError("EC_Mesh::SetMeshWithSkeleton: Could not set skeleton " + skeleton_name + " to mesh " + mesh_name + ": not found");
        return false;
    }
    
    RemoveMesh();

    Ogre::SceneManager* sceneMgr = world->OgreSceneManager();
    
    Ogre::Mesh* mesh = PrepareMesh(mesh_name, clone);
    if (!mesh)
        return false;
    
    try
    {
        mesh->_notifySkeleton(skel);
//        LogDebug("Set skeleton " + skeleton_name + " to mesh " + mesh_name);
    }
    catch(const Ogre::Exception& e)
    {
        LogError("EC_Mesh::SetMeshWithSkeleton: Could not set skeleton " + skeleton_name + " to mesh " + mesh_name + ": " + std::string(e.what()));
        return false;
    }
    
    try
    {
        entity_ = sceneMgr->createEntity(world->GetUniqueObjectName("EC_Mesh_entwithskel"), mesh->getName());
        if (!entity_)
        {
            LogError("EC_Mesh::SetMeshWithSkeleton: Could not set mesh " + mesh_name);
            return false;
        }
        
        entity_->setRenderingDistance(drawDistance.Get());
        entity_->setCastShadows(castShadows.Get());
        entity_->setUserAny(Ogre::Any(static_cast<IComponent *>(this)));
        // Set UserAny also on subentities
        for(uint i = 0; i < entity_->getNumSubEntities(); ++i)
            entity_->getSubEntity(i)->setUserAny(entity_->getUserAny());
        
        if (entity_->hasSkeleton())
        {
            Ogre::SkeletonInstance* skel = entity_->getSkeleton();
            // Enable cumulative mode on skeletal animations
            if (skel)
                skel->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);
        }
    }
    catch(const Ogre::Exception& e)
    {
        LogError("EC_Mesh::SetMeshWithSkeleton: Could not set mesh " + mesh_name + ": " + std::string(e.what()));
        return false;
    }
    
    AttachEntity();
    
    emit MeshChanged();
    
    return true;
}

void EC_Mesh::RemoveMesh()
{
    OgreWorldPtr world = world_.lock();

    if (entity_)
    {
        emit MeshAboutToBeDestroyed();
        
        RemoveAllAttachments();
        DetachEntity();
        
        Ogre::SceneManager* sceneMgr = world->OgreSceneManager();
        sceneMgr->destroyEntity(entity_);
        
        entity_ = 0;
    }
    
    if (!cloned_mesh_name_.empty())
    {
        try
        {
            Ogre::MeshManager::getSingleton().remove(cloned_mesh_name_);
        }
        catch(const Ogre::Exception& e)
        {
            LogWarning("EC_Mesh::RemoveMesh: Could not remove cloned mesh:" + std::string(e.what()));
        }
        
        cloned_mesh_name_ = std::string();
    }
}

Ogre::Bone* EC_Mesh::OgreBone(const QString& boneName) const
{
    std::string boneNameStd = boneName.toStdString();
    Ogre::Skeleton* skel = (entity_ ? entity_->getSkeleton() : 0);
    return (skel && skel->hasBone(boneNameStd) ? skel->getBone(boneNameStd) : 0);
}

bool EC_Mesh::SetAttachmentMesh(uint index, const std::string& mesh_name, const std::string& attach_point, bool share_skeleton)
{
    if (!ViewEnabled())
        return false;
    OgreWorldPtr world = world_.lock();

    if (!entity_)
    {
        LogError("EC_Mesh::SetAttachmentMesh: No mesh entity created yet, can not create attachments!");
        return false;
    }
    
    Ogre::SceneManager* sceneMgr = world->OgreSceneManager();
    
    size_t oldsize = attachment_entities_.size();
    size_t newsize = index + 1;
    
    if (oldsize < newsize)
    {
        attachment_entities_.resize(newsize);
        attachment_nodes_.resize(newsize);
        for(size_t i = oldsize; i < newsize; ++i)
        {
            attachment_entities_[i] = 0;
            attachment_nodes_[i] = 0;
        }
    }
    
    RemoveAttachmentMesh(index);
    
    Ogre::Mesh* mesh = PrepareMesh(mesh_name, false);
    if (!mesh)
        return false;

    if (share_skeleton)
    {
        // If sharing a skeleton, force the attachment mesh to use the same skeleton
        // This is theoretically quite a scary operation, for there is possibility for things to go wrong
        Ogre::SkeletonPtr entity_skel = entity_->getMesh()->getSkeleton();
        if (entity_skel.isNull())
        {
            LogError("EC_Mesh::SetAttachmentMesh: Cannot share skeleton for attachment, not found");
            return false;
        }
        try
        {
            mesh->_notifySkeleton(entity_skel);
        }
        catch(const Ogre::Exception &/*e*/)
        {
            LogError("EC_Mesh::SetAttachmentMesh: Could not set shared skeleton for attachment");
            return false;
        }
    }

    try
    {
        QString entityName = QString("EC_Mesh_attach") + QString::number(index);
        attachment_entities_[index] = sceneMgr->createEntity(world->GetUniqueObjectName(entityName.toStdString()), mesh->getName());
        if (!attachment_entities_[index])
        {
            LogError("EC_Mesh::SetAttachmentMesh: Could not set attachment mesh " + mesh_name);
            return false;
        }

        attachment_entities_[index]->setRenderingDistance(drawDistance.Get());
        attachment_entities_[index]->setCastShadows(castShadows.Get());
        attachment_entities_[index]->setUserAny(entity_->getUserAny());
        // Set UserAny also on subentities
        for(uint i = 0; i < attachment_entities_[index]->getNumSubEntities(); ++i)
            attachment_entities_[index]->getSubEntity(i)->setUserAny(entity_->getUserAny());

        Ogre::Bone* attach_bone = 0;
        if (!attach_point.empty())
        {
            Ogre::Skeleton* skel = entity_->getSkeleton();
            if (skel && skel->hasBone(attach_point))
                attach_bone = skel->getBone(attach_point);
        }
        if (attach_bone)
        {
            Ogre::TagPoint* tag = entity_->attachObjectToBone(attach_point, attachment_entities_[index]);
            attachment_nodes_[index] = tag;
        }
        else
        {
            QString nodeName = QString("EC_Mesh_attachment_") + QString::number(index);
            Ogre::SceneNode* node = sceneMgr->createSceneNode(world->GetUniqueObjectName(nodeName.toStdString()));
            node->attachObject(attachment_entities_[index]);
            adjustment_node_->addChild(node);
            attachment_nodes_[index] = node;
        }
        
        if (share_skeleton && entity_->hasSkeleton() && attachment_entities_[index]->hasSkeleton())
        {
            attachment_entities_[index]->shareSkeletonInstanceWith(entity_);
        }
    }
    catch(const Ogre::Exception& e)
    {
        LogError("EC_Mesh::SetAttachmentMesh: Could not set attachment mesh " + mesh_name + ": " + std::string(e.what()));
        return false;
    }
    return true;
}

void EC_Mesh::RemoveAttachmentMesh(uint index)
{
    OgreWorldPtr world = world_.lock();
    
    if (!entity_)
        return;
        
    if (index >= attachment_entities_.size())
        return;
    
    Ogre::SceneManager* sceneMgr = world->OgreSceneManager();
    
    if (attachment_entities_[index] && attachment_nodes_[index])
    {
        // See if attached to a tagpoint or an ordinary node
        Ogre::TagPoint* tag = dynamic_cast<Ogre::TagPoint*>(attachment_nodes_[index]);
        if (tag)
        {
            entity_->detachObjectFromBone(attachment_entities_[index]);
        }
        else
        {
            Ogre::SceneNode* scenenode = dynamic_cast<Ogre::SceneNode*>(attachment_nodes_[index]);
            if (scenenode)
            {
                scenenode->detachObject(attachment_entities_[index]);
                sceneMgr->destroySceneNode(scenenode);
            }
        }
        
        attachment_nodes_[index] = 0;
    }
    if (attachment_entities_[index])
    {
        if (attachment_entities_[index]->sharesSkeletonInstance())
            attachment_entities_[index]->stopSharingSkeletonInstance();
        sceneMgr->destroyEntity(attachment_entities_[index]);
        attachment_entities_[index] = 0;
    }
}

void EC_Mesh::RemoveAllAttachments()
{
    for(uint i = 0; i < attachment_entities_.size(); ++i)
        RemoveAttachmentMesh(i);
    attachment_entities_.clear();
    attachment_nodes_.clear();
}

bool EC_Mesh::SetMaterial(uint index, const QString& material_name, AttributeChange::Type change)
{
    if (!entity_)
        return false;
    
    if (index >= entity_->getNumSubEntities())
    {
        LogError("EC_Mesh::SetMaterial: Could not set material " + material_name + ": illegal submesh index " + QString::number(index) + 
            ". Mesh \"" + meshRef.Get().ref + "\" has only " + QString::number(entity_->getNumSubEntities()) + " submeshes!");
        return false;
    }
    
    try
    {
        entity_->getSubEntity(index)->setMaterialName(AssetAPI::SanitateAssetRef(material_name.toStdString()));

        if (pendingFailedMaterials_.contains(index))
            pendingFailedMaterials_.removeAll(index);

        // Update the EC_Mesh material attribute list so that users can call EC_Mesh::SetMaterial as a replacement for setting
        // meshMaterial attribute. Only apply the change if the value really changed.
        AssetReferenceList materials = meshMaterial.Get();
        while(materials.Size() <= (int)index)
            materials.Append(AssetReference());
        if (material_name.compare(materials[index].ref, Qt::CaseSensitive) != 0)
        {
            materials.Set(index, AssetReference(material_name));
            meshMaterial.Set(materials, change); // Potentially signal the change of attribute, if requested so.
        }

        // To retain compatibility with old behavior, always fire the EC_Mesh -specific change signal independent of the value of 'change'.
        emit MaterialChanged(index, material_name);
    }
    catch(const Ogre::Exception& e)
    {
        LogError("EC_Mesh::SetMaterial: Could not set material " + material_name + ": " + e.what());
        return false;
    }
    
    return true;
}

bool EC_Mesh::SetAttachmentMaterial(uint index, uint submesh_index, const std::string& material_name)
{
    if (index >= attachment_entities_.size() || attachment_entities_[index] == 0)
    {
        LogError("EC_Mesh::SetAttachmentMaterial: Could not set material " + material_name + " on attachment: no mesh");
        return false;
    }
    
    if (submesh_index >= attachment_entities_[index]->getNumSubEntities())
    {
        LogError("EC_Mesh::SetAttachmentMaterial: Could not set material " + material_name + " on attachment: illegal submesh index " + QString::number(submesh_index).toStdString());
        return false;
    }
    
    try
    {
        attachment_entities_[index]->getSubEntity(submesh_index)->setMaterialName(AssetAPI::SanitateAssetRef(material_name));
    }
    catch(const Ogre::Exception& e)
    {
        LogError("EC_Mesh::SetAttachmentMaterial: Could not set material " + material_name + " on attachment: " + std::string(e.what()));
        return false;
    }
    
    return true;
}

uint EC_Mesh::NumMaterials() const
{
    if (!entity_)
        return 0;
        
    return entity_->getNumSubEntities();
}

uint EC_Mesh::NumAttachmentMaterials(uint index) const
{
    if (index >= attachment_entities_.size() || attachment_entities_[index] == 0)
        return 0;
        
    return attachment_entities_[index]->getNumSubEntities();
}

const std::string& EC_Mesh::MaterialName(uint index) const
{
    const static std::string empty;
    
    if (!entity_)
        return empty;
    
    if (index >= entity_->getNumSubEntities())
        return empty;
    
    return entity_->getSubEntity(index)->getMaterialName();
}

const std::string& EC_Mesh::AttachmentMaterialName(uint index, uint submesh_index) const
{
    const static std::string empty;
    
    if (index >= attachment_entities_.size() || attachment_entities_[index] == 0)
        return empty;
    if (submesh_index >= attachment_entities_[index]->getNumSubEntities())
        return empty;
    
    return attachment_entities_[index]->getSubEntity(submesh_index)->getMaterialName();
}

bool EC_Mesh::HasAttachmentMesh(uint index) const
{
    if (index >= attachment_entities_.size() || attachment_entities_[index] == 0)
        return false;
        
    return true;
}

Ogre::Entity* EC_Mesh::AttachmentOgreEntity(uint index) const
{
    if (index >= attachment_entities_.size())
        return 0;
    return attachment_entities_[index];
}

uint EC_Mesh::NumSubMeshes() const
{
    uint count = 0;
    if (HasMesh())
        if (entity_->getMesh().get())
            count = entity_->getMesh()->getNumSubMeshes();
    return count;
}

const std::string& EC_Mesh::MeshName() const
{
    static std::string empty_name;
    
    if (!entity_)
        return empty_name;
    else
        return entity_->getMesh()->getName();
}

const std::string& EC_Mesh::SkeletonName() const
{
    static std::string empty_name;
    
    if (!entity_)
        return empty_name;
    else
    {
        Ogre::MeshPtr mesh = entity_->getMesh();
        if (!mesh->hasSkeleton())
            return empty_name;
        Ogre::SkeletonPtr skel = mesh->getSkeleton();
        if (skel.isNull())
            return empty_name;
        return skel->getName();
    }
}

void EC_Mesh::DetachEntity()
{
    if (!attached_ || !entity_ || !placeable_)
        return;
    
    EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(placeable_.get());
    Ogre::SceneNode* node = placeable->GetSceneNode();
    adjustment_node_->detachObject(entity_);
    node->removeChild(adjustment_node_);
    attached_ = false;
}

void EC_Mesh::AttachEntity()
{
    if (attached_ || !entity_ || !placeable_)
        return;
    
    EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(placeable_.get());
    Ogre::SceneNode* node = placeable->GetSceneNode();
    node->addChild(adjustment_node_);
    adjustment_node_->attachObject(entity_);

    // Honor the EC_Placeable's isVisible attribute by enforcing its values on this mesh.
    adjustment_node_->setVisible(placeable->visible.Get());

    attached_ = true;
}

Ogre::Mesh* EC_Mesh::PrepareMesh(const std::string& mesh_name, bool clone)
{
    if (!ViewEnabled())
        return 0;
    OgreWorldPtr world = world_.lock();
    
    Ogre::MeshManager& mesh_mgr = Ogre::MeshManager::getSingleton();
    Ogre::MeshPtr mesh = mesh_mgr.getByName(AssetAPI::SanitateAssetRef(mesh_name));
    
    // For local meshes, mesh will not get automatically loaded until used in an entity. Load now if necessary
    if (mesh.isNull())
    {
        try
        {
            mesh_mgr.load(mesh_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            mesh = mesh_mgr.getByName(mesh_name);
        }
        catch(const Ogre::Exception& e)
        {
            LogError("EC_Mesh::PrepareMesh: Could not load mesh " + mesh_name + ": " + std::string(e.what()));
            return 0;
        }
    }
    
    // If mesh is still null, must abort
    if (mesh.isNull())
    {
        LogError("EC_Mesh::PrepareMesh: Mesh " + mesh_name + " does not exist");
        return 0;
    }
    
    if (clone)
    {
        try
        {
            mesh = mesh->clone(world->GetUniqueObjectName("EC_Mesh_clone"));
            mesh->setAutoBuildEdgeLists(false);
            cloned_mesh_name_ = mesh->getName();
        }
        catch(const Ogre::Exception& e)
        {
            LogError("EC_Mesh::PrepareMesh: Could not clone mesh " + mesh_name + ":" + std::string(e.what()));
            return 0;
        }
    }
    
    if (mesh->hasSkeleton())
    {
        Ogre::SkeletonPtr skeleton = Ogre::SkeletonManager::getSingleton().getByName(mesh->getSkeletonName());
        if (skeleton.isNull() || skeleton->getNumBones() == 0)
        {
            LogDebug("EC_Mesh::PrepareMesh: Mesh " + mesh_name + " has a skeleton with 0 bones. Disabling the skeleton.");
            mesh->setSkeletonName("");
        }
    }
    
    return mesh.get();
}

void EC_Mesh::UpdateSignals()
{
    Entity* parent = ParentEntity();
    if (parent)
    {
        // Connect to ComponentRemoved signal of the parent entity, so we can check if the placeable gets removed
        connect(parent, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), SLOT(OnComponentRemoved(IComponent*, AttributeChange::Type)));
    }
}

void EC_Mesh::AttributesChanged()
{
    if (drawDistance.ValueChanged())
    {
        if(entity_)
            entity_->setRenderingDistance(drawDistance.Get());
    }
    if (castShadows.ValueChanged())
    {
        if(entity_)
        {
            if (entity_)
                entity_->setCastShadows(castShadows.Get());
            /// \todo might want to disable shadows for some attachments
            for(uint i = 0; i < attachment_entities_.size(); ++i)
            {
                if (attachment_entities_[i])
                    attachment_entities_[i]->setCastShadows(castShadows.Get());
            }
        }
    }
    if (nodeTransformation.ValueChanged())
    {
        Transform newTransform = nodeTransformation.Get();
        adjustment_node_->setPosition(newTransform.pos);
        adjustment_node_->setOrientation(newTransform.Orientation());
        
        // Prevent Ogre exception from zero scale
        newTransform.scale = Max(newTransform.scale, float3::FromScalar(0.0000001f));
        
        adjustment_node_->setScale(newTransform.scale);
    }
    if (meshRef.ValueChanged())
    {
        if (!ViewEnabled())
            return;

        if (meshRef.Get().ref.trimmed().isEmpty())
            LogDebug("Warning: Mesh \"" + this->parentEntity->Name() + "\" mesh ref was set to an empty reference!");
        meshAsset->HandleAssetRefChange(&meshRef);
    }
    if (meshMaterial.ValueChanged())
    {
        if (!ViewEnabled())
            return;

        AssetReferenceList materials = meshMaterial.Get();

        // Reset all the materials from the submeshes which now have an empty material asset reference set.
        for(uint i = 0; i < GetNumMaterials(); ++i)
        {
            if ((int)i >= materials.Size() || materials[i].ref.trimmed().isEmpty())
            {
                if (entity_ && entity_->getSubEntity(i))
                    entity_->getSubEntity(i)->setMaterialName("");
            }
        }

        // Reallocate the number of material asset ref listeners.
        while(materialAssets.size() > (size_t)materials.Size())
            materialAssets.pop_back();
        while(materialAssets.size() < (size_t)materials.Size())
            materialAssets.push_back(shared_ptr<AssetRefListener>(new AssetRefListener));

        for(int i = 0; i < materials.Size(); ++i)
        {
            // Don't request empty refs. HandleAssetRefChange will just do unnecessary work and the below connections are for nothing.
            if (!materials[i].ref.trimmed().isEmpty())
            {
                connect(materialAssets[i].get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnMaterialAssetLoaded(AssetPtr)), Qt::UniqueConnection);
                connect(materialAssets[i].get(), SIGNAL(TransferFailed(IAssetTransfer*, QString)), this, SLOT(OnMaterialAssetFailed(IAssetTransfer*, QString)), Qt::UniqueConnection);
                materialAssets[i]->HandleAssetRefChange(framework->Asset(), materials[i].ref);
            }
        }
    }
    if (skeletonRef.ValueChanged())
    {
        if (!ViewEnabled())
            return;

        if (!skeletonRef.Get().ref.isEmpty())
            skeletonAsset->HandleAssetRefChange(&skeletonRef);
    }
}

void EC_Mesh::OnComponentRemoved(IComponent* component, AttributeChange::Type change)
{
    if (component == placeable_.get())
        SetPlaceable(ComponentPtr());
}

void EC_Mesh::OnMeshAssetLoaded(AssetPtr asset)
{
    OgreMeshAsset *mesh = dynamic_cast<OgreMeshAsset*>(asset.get());
    if (!mesh)
    {
        LogError("EC_Mesh::OnMeshAssetLoaded: Mesh asset load finished for asset \"" +
            asset->Name() + "\", but downloaded asset was not of type OgreMeshAsset!");
        return;
    }

    QString ogreMeshName = mesh->Name();
    if (mesh)
    {
        if (mesh->ogreMesh.get())
            ogreMeshName = QString::fromStdString(mesh->ogreMesh->getName()).trimmed();
        else
            LogError("EC_Mesh::OnMeshAssetLoaded: Mesh asset load finished for asset \"" +
                asset->Name() + "\", but Ogre::Mesh pointer was null!");
    }

    SetMesh(ogreMeshName);

    // Force a re-application of the skeleton on this mesh. ///\todo This path should be re-evaluated to see if we have potential performance issues here. -jj.
    if (skeletonAsset->Asset())
        OnSkeletonAssetLoaded(skeletonAsset->Asset());

    // Apply any failed materials that failed before the mesh was loaded. We want the visual error material to be there.
    foreach(uint failedIndex, pendingFailedMaterials_)
    {
        if (!entity_ || !entity_->getSubEntity(failedIndex))
            continue;
        try
        {
            entity_->getSubEntity(failedIndex)->setMaterialName("AssetLoadError");
        }
        catch(const Ogre::Exception& e)
        {
            LogError(QString("EC_Mesh::OnMeshAssetLoaded: Could not set error material AssetLoadError: ") + e.what());
        }
    }
    pendingFailedMaterials_.clear();
}

void EC_Mesh::OnSkeletonAssetLoaded(AssetPtr asset)
{
    OgreSkeletonAsset *skeletonAsset = dynamic_cast<OgreSkeletonAsset*>(asset.get());
    if (!skeletonAsset)
    {
        LogError("EC_Mesh::OnSkeletonAssetLoaded: Skeleton asset load finished for asset \"" +
            asset->Name() + "\", but downloaded asset was not of type OgreSkeletonAsset!");
        return;
    }

    Ogre::SkeletonPtr skeleton = skeletonAsset->ogreSkeleton;
    if (skeleton.isNull())
    {
        LogError("EC_Mesh::OnSkeletonAssetLoaded: Skeleton asset load finished for asset \"" +
            asset->Name() + "\", but Ogre::Skeleton pointer was null!");
        return;
    }

    if(!entity_)
    {
//        LogDebug("Could not set skeleton yet because entity is not yet created");
        return;
    }

    try
    {
        // If old skeleton is same as a new one no need to replace it.
        if (entity_->getSkeleton() && entity_->getSkeleton()->getName() == skeleton->getName())
            return;
        
        entity_->getMesh()->_notifySkeleton(skeleton);
        
//        LogDebug("Set skeleton " + skeleton->getName() + " to mesh " + entity_->getName());
        emit SkeletonChanged(QString::fromStdString(skeleton->getName()));
    }
    catch(...)
    {
        LogError("Exception while setting skeleton to mesh" + entity_->getName());
    }

    // Now we have to recreate the entity to get proper animations etc.
    SetMesh(entity_->getMesh()->getName().c_str(), false);
}

void EC_Mesh::OnMaterialAssetLoaded(AssetPtr asset)
{
    OgreMaterialAsset *ogreMaterial = dynamic_cast<OgreMaterialAsset*>(asset.get());
    if (!ogreMaterial)
    {
        LogError("OnMaterialAssetLoaded: Material asset load finished for asset \"" +
            asset->Name() + "\", but downloaded asset was not of type OgreMaterialAsset!");
        return;
    }

    Ogre::MaterialPtr material = ogreMaterial->ogreMaterial;
    bool assetUsed = false;

    AssetReferenceList materialList = meshMaterial.Get();
    for(int i = 0; i < materialList.Size(); ++i)
        if (materialList[i].ref.compare(ogreMaterial->Name(), Qt::CaseInsensitive) == 0 ||
            framework->Asset()->ResolveAssetRef("", materialList[i].ref).compare(ogreMaterial->Name(), Qt::CaseInsensitive) == 0) ///<///\todo The design of whether the ResolveAssetRef should occur here, or internal to Asset API needs to be revisited.
        {
            SetMaterial(i, ogreMaterial->Name(), AttributeChange::Disconnected);
            assetUsed = true;
        }

    // This check & debug print is now in Debug mode only. Rapid changes in materials and the delay-loaded nature of assets makes it unavoidable in some cases.
    #ifdef _DEBUG
    if (!assetUsed)
    {
        LogDebug("OnMaterialAssetLoaded: Trying to apply material \"" + ogreMaterial->Name() + "\" to mesh " +
            meshRef.Get().ref + ", but no submesh refers to the given material! The references are: ");
        for(int i = 0; i < materialList.Size(); ++i)
            LogDebug(QString::number(i) + ": " + materialList[i].ref);
    }
    #endif
}

void EC_Mesh::OnMaterialAssetFailed(IAssetTransfer* transfer, QString reason)
{
    // Check which of the material(s) match the failed ref
    AssetReferenceList materialList = meshMaterial.Get();
    for(int i = 0; i < materialList.Size(); ++i)
    {
        QString absoluteRef = framework->Asset()->ResolveAssetRef("", materialList[i].ref);
        if (absoluteRef == transfer->source.ref)
        {
            // Do not use SetMaterial here as it will modify meshMaterials into "AssetLoadError"
            // which is not what we want to do here. If the asset it later created/loaded the 
            // string compare in OnMaterialAssetLoaded breaks. This is a valid scenario in some 
            // components like EC_Material.
            if (entity_)
            {
                if ((uint)i < entity_->getNumSubEntities() && entity_->getSubEntity(i))
                {
                    try
                    {
                        entity_->getSubEntity(i)->setMaterialName("AssetLoadError");
                    }
                    catch(const Ogre::Exception& e)
                    {
                        LogError(QString("EC_Mesh::OnMaterialAssetFailed: Could not set error material AssetLoadError: ") + e.what());
                    }
                }
            }
            // Store the index so we can apply the visual error material later once the mesh is loaded.
            else if (!pendingFailedMaterials_.contains((uint)i))
                pendingFailedMaterials_ << (uint)i;
        }
    }
}

void EC_Mesh::ApplyMaterial()
{
    AssetReferenceList materialList = meshMaterial.Get();
    AssetAPI *assetAPI = framework->Asset();
    for(int i = 0; i < materialList.Size(); ++i)
    {
        if (!materialList[i].ref.isEmpty())
        {
            // Only apply the material if it is loaded and has no dependencies
            QString assetFullName = assetAPI->ResolveAssetRef("", materialList[i].ref);
            AssetPtr asset = assetAPI->GetAsset(assetFullName);
            if (asset && !assetAPI->HasPendingDependencies(asset))
                SetMaterial(i, assetFullName, AttributeChange::Disconnected);
        }
    }
}

QStringList EC_Mesh::AvailableBones() const
{
    QStringList ret;
    
    if (!entity_)
        return ret;
    Ogre::Skeleton* skel = entity_->getSkeleton();
    if (!skel)
        return ret;
    Ogre::Skeleton::BoneIterator it = skel->getBoneIterator();
    while (it.hasMoreElements())
    {
        Ogre::Bone* bone = it.getNext();
        ret.push_back(QString::fromStdString(bone->getName()));
    }
    
    return ret;
}

void EC_Mesh::ForceSkeletonUpdate()
{
    if (!entity_)
        return;
    Ogre::Skeleton* skel = entity_->getSkeleton();
    if (!skel)
        return;
    if (entity_->getAllAnimationStates())
        skel->setAnimationState(*entity_->getAllAnimationStates());
}

float3 EC_Mesh::BonePosition(const QString& bone_name)
{
    Ogre::Bone* bone = GetBone(bone_name);
    if (bone)
        return bone->getPosition();
    else
        return float3::zero;
}

float3 EC_Mesh::BoneDerivedPosition(const QString& bone_name)
{
    Ogre::Bone* bone = GetBone(bone_name);
    if (bone)
        return bone->_getDerivedPosition();
    else
        return float3::zero;
}

Quat EC_Mesh::BoneOrientation(const QString& bone_name)
{
    Ogre::Bone* bone = GetBone(bone_name);
    if (bone)
        return bone->getOrientation();
    else
        return Quat::identity;
}

Quat EC_Mesh::BoneDerivedOrientation(const QString& bone_name)
{
    Ogre::Bone* bone = GetBone(bone_name);
    if (bone)
        return bone->_getDerivedOrientation();
    else
        return Quat::identity;
}
/*
float3 EC_Mesh::BoneOrientationEuler(const QString& bone_name)
{
    Ogre::Bone* bone = GetBone(bone_name);
    if (bone)
    {
        const Ogre::Quaternion& quat = bone->getOrientation();
        Quaternion q(quat.x, quat.y, quat.z, quat.w);
        float3 euler;
        q.toEuler(euler);
        return euler * RADTODEG;
    }
    else
        return float3::zero;
}

float3 EC_Mesh::BoneDerivedOrientationEuler(const QString& bone_name)
{
    Ogre::Bone* bone = GetBone(bone_name);
    if (bone)
    {
        const Ogre::Quaternion& quat = bone->_getDerivedOrientation();
        Quaternion q(quat.x, quat.y, quat.z, quat.w);
        float3 euler;
        q.toEuler(euler);
        return euler * RADTODEG;
    }
    else
        return float3::zero;
}
*/

void EC_Mesh::SetMorphWeight(const QString& morphName, float weight)
{
    if (!entity_)
        return;
    Ogre::AnimationStateSet* anims = entity_->getAllAnimationStates();
    if (!anims)
        return;
    
    std::string morphNameStd = morphName.toStdString();
    
    if (anims->hasAnimationState(morphNameStd))
    {
        if (weight < 0.0f)
            weight = 0.0f;
        // Clamp very close to 1.0, but do not actually go to 1.0 or the morph animation will wrap
        if (weight > 0.99995f)
            weight = 0.99995f;
        
        Ogre::AnimationState* anim = anims->getAnimationState(morphNameStd);
        anim->setTimePosition(weight);
        anim->setEnabled(weight > 0.0f);
    }
}

float EC_Mesh::MorphWeight(const QString& morphName) const
{
    if (!entity_)
        return 0.0f;
    Ogre::AnimationStateSet* anims = entity_->getAllAnimationStates();
    if (!anims)
        return 0.0f;
    
    std::string morphNameStd = morphName.toStdString();
    if (anims->hasAnimationState(morphNameStd))
    {
        Ogre::AnimationState* anim = anims->getAnimationState(morphNameStd);
        return anim->getTimePosition();
    }
    else
        return 0.0f;
}

void EC_Mesh::SetAttachmentMorphWeight(unsigned index, const QString& morphName, float weight)
{
    Ogre::Entity* entity = GetAttachmentEntity(index);
    Ogre::AnimationStateSet* anims = entity->getAllAnimationStates();
    if (!anims)
        return;
    
    std::string morphNameStd = morphName.toStdString();
    
    if (anims->hasAnimationState(morphNameStd))
    {
        if (weight < 0.0f)
            weight = 0.0f;
        // Clamp very close to 1.0, but do not actually go to 1.0 or the morph animation will wrap
        if (weight > 0.99995f)
            weight = 0.99995f;
        
        Ogre::AnimationState* anim = anims->getAnimationState(morphNameStd);
        anim->setTimePosition(weight);
        anim->setEnabled(weight > 0.0f);
    }
}

float EC_Mesh::AttachmentMorphWeight(unsigned index, const QString& morphName) const
{
    Ogre::Entity* entity = GetAttachmentEntity(index);
    if (!entity)
        return 0.0f;
    Ogre::AnimationStateSet* anims = entity->getAllAnimationStates();
    if (!anims)
        return 0.0f;
    
    std::string morphNameStd = morphName.toStdString();
    if (anims->hasAnimationState(morphNameStd))
    {
        Ogre::AnimationState* anim = anims->getAnimationState(morphNameStd);
        return anim->getTimePosition();
    }
    else
        return 0.0f;
}

OBB EC_Mesh::WorldOBB() const
{
    OBB obb = LocalOBB();
    obb.Transform(LocalToWorld());
    return obb;
}

OBB EC_Mesh::LocalOBB() const
{
    return OBB(LocalAABB());
}

AABB EC_Mesh::WorldAABB() const
{
    AABB aabb = LocalAABB();
    aabb.Transform(LocalToWorld());
    return aabb;
}

AABB EC_Mesh::LocalAABB() const
{
    if (!entity_)
        return AABB();

    Ogre::MeshPtr mesh = entity_->getMesh();
    if (mesh.isNull())
        return AABB();

    return AABB(mesh->getBounds());
}

OgreMeshAssetPtr EC_Mesh::MeshAsset() const
{
    if (!meshAsset)
        return OgreMeshAssetPtr();
    return dynamic_pointer_cast<OgreMeshAsset>(meshAsset->Asset());
}

OgreMaterialAssetPtr EC_Mesh::MaterialAsset(int materialIndex) const
{
    if (materialIndex < 0 || materialIndex >= (int)materialAssets.size())
        return OgreMaterialAssetPtr();
    return dynamic_pointer_cast<OgreMaterialAsset>(materialAssets[materialIndex]->Asset());
}

OgreSkeletonAssetPtr EC_Mesh::SkeletonAsset() const
{
    if (!skeletonAsset)
        return OgreSkeletonAssetPtr();
    return dynamic_pointer_cast<OgreSkeletonAsset>(skeletonAsset->Asset());
}

Ogre::Vector2 FindUVs(const Ogre::Vector3& hitPoint, const Ogre::Vector3& t1, const Ogre::Vector3& t2, const Ogre::Vector3& t3, const Ogre::Vector2& tex1, const Ogre::Vector2& tex2, const Ogre::Vector2& tex3)
{
    Ogre::Vector3 v1 = hitPoint - t1;
    Ogre::Vector3 v2 = hitPoint - t2;
    Ogre::Vector3 v3 = hitPoint - t3;
    
    float area1 = (v2.crossProduct(v3)).length() / 2.0f;
    float area2 = (v1.crossProduct(v3)).length() / 2.0f;
    float area3 = (v1.crossProduct(v2)).length() / 2.0f;
    float sum_area = area1 + area2 + area3;
    if (sum_area <= 0.0)
        return Ogre::Vector2(0.0f, 0.0f);
    
    Ogre::Vector3 bary(area1 / sum_area, area2 / sum_area, area3 / sum_area);
    Ogre::Vector2 t = tex1 * bary.x + tex2 * bary.y + tex3 * bary.z;
    
    return t;
}

bool EC_Mesh::Raycast(Ogre::Entity* meshEntity, const Ray& ray, float* distance, unsigned* subMeshIndex, unsigned* triangleIndex, float3* hitPosition, float3* normal, float2* uv)
{
    PROFILE(EC_Mesh_Raycast);
    
    if (!meshEntity)
    {
        LogError("EC_Mesh::Raycast called with null mesh entity. Returning no result.");
        return false;
    }
    Ogre::SceneNode *node = meshEntity->getParentSceneNode();
    if (!node)
    {
        LogError("EC_Mesh::Raycast called for a mesh entity that is not attached to a scene node. Returning no result.");
        return false;
    }

    assume(!float3(node->_getDerivedScale()).IsZero());
    float3x4 localToWorld = float3x4::FromTRS(node->_getDerivedPosition(), node->_getDerivedOrientation(), node->_getDerivedScale());
    assume(localToWorld.IsColOrthogonal());
    
    float3x4 worldToLocal = localToWorld.Inverted();
    Ray localRay = ray;
    localRay.Transform(worldToLocal);
    Ogre::Ray ogreLocalRay = localRay;

    Ogre::MeshPtr mesh = meshEntity->getMesh();
    bool useSoftwareBlendingVertices = meshEntity->hasSkeleton();
    
    float closestDistance = -1.0f; // In objectspace
    
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
        
        Ogre::VertexData* vertexData;
        if (useSoftwareBlendingVertices)
            vertexData = submesh->useSharedVertices ? meshEntity->_getSkelAnimVertexData() : meshEntity->getSubEntity(i)->_getSkelAnimVertexData();
        else
            vertexData = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;
        
        const Ogre::VertexElement* posElem =
            vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
        if (!posElem)
            continue; // No position element, can not raycast
        
        Ogre::HardwareVertexBufferSharedPtr vbufPos =
            vertexData->vertexBufferBinding->getBuffer(posElem->getSource());

        unsigned char* pos;
        {
            PROFILE(EC_Mesh_Raycast_PosBuf_Lock);
            pos = static_cast<unsigned char*>(vbufPos->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        }

        size_t posOffset = posElem->getOffset();
        size_t posSize = vbufPos->getVertexSize();
        
        // Texcoord element is not mandatory
        unsigned char* texCoord = 0;
        size_t texOffset = 0;
        size_t texSize = 0;
        Ogre::HardwareVertexBufferSharedPtr vbufTex;
        const Ogre::VertexElement *texElem = vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES);
        if (texElem)
        {
            vbufTex = vertexData->vertexBufferBinding->getBuffer(texElem->getSource());
            // Check if the texcoord buffer is different than the position buffer, in that case lock it separately
            if (vbufTex != vbufPos)
            {
                PROFILE(EC_Mesh_Raycast_TexBuf_Lock);
                texCoord = static_cast<unsigned char*>(vbufTex->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
            }
            else
                texCoord = pos;
            texOffset = texElem->getOffset();
            texSize = vbufTex->getVertexSize();
        }
        
        Ogre::IndexData* indexData = submesh->indexData;
        Ogre::HardwareIndexBufferSharedPtr ibuf = indexData->indexBuffer;

        u32*  pLong;
        {
            PROFILE(EC_Mesh_Raycast_IndexBuf_Lock);
            pLong = static_cast<u32*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        }
        u16* pShort = reinterpret_cast<u16*>(pLong);
        bool use32BitIndices = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
        
        for (unsigned j = 0; j < indexData->indexCount - 2; j += 3)
        {
            unsigned i0, i1, i2;
            if (use32BitIndices)
            {
                i0 = pLong[j];
                i1 = pLong[j+1];
                i2 = pLong[j+2];
            }
            else
            {
                i0 = pShort[j];
                i1 = pShort[j+1];
                i2 = pShort[j+2];
            }
            
            const Ogre::Vector3& v0 = *((Ogre::Vector3*)(pos + posOffset + i0 * posSize));
            const Ogre::Vector3& v1 = *((Ogre::Vector3*)(pos + posOffset + i1 * posSize));
            const Ogre::Vector3& v2 = *((Ogre::Vector3*)(pos + posOffset + i2 * posSize));
            
            std::pair<bool, Ogre::Real> hit = Ogre::Math::intersects(ogreLocalRay, v0, v1, v2, true, false);
            if (hit.first && hit.second >= 0.0f && (closestDistance < 0.0f || hit.second < closestDistance))
            {
                closestDistance = hit.second;
                
                float3 localHitPoint = ogreLocalRay.getPoint(hit.second);
                float3 worldHitPoint = localToWorld.TransformPos(localHitPoint);

                if (subMeshIndex)
                    *subMeshIndex = i;
                if (triangleIndex)
                    *triangleIndex = j / 3;
                if (distance)
                    *distance = (worldHitPoint - ray.pos).Length();
                if (hitPosition)
                    *hitPosition = worldHitPoint;
                if (uv && texElem)
                {
                    const Ogre::Vector2& t0 = *((Ogre::Vector2*)(texCoord + texOffset + i0 * texSize));
                    const Ogre::Vector2& t1 = *((Ogre::Vector2*)(texCoord + texOffset + i1 * texSize));
                    const Ogre::Vector2& t2 = *((Ogre::Vector2*)(texCoord + texOffset + i2 * texSize));
                    
                    *uv = FindUVs(localHitPoint, v0, v1, v2, t0, t1, t2);
                }
                if (normal)
                {
                    float3 edge1 = v1 - v0;
                    float3 edge2 = v2 - v0;
                    *normal = localToWorld.TransformDir(edge1.Cross(edge2));
                    normal->Normalize();
                }
            }
        }
        
        vbufPos->unlock();
        if (!vbufTex.isNull() && vbufTex != vbufPos)
            vbufTex->unlock();
        ibuf->unlock();
    }
    
    return closestDistance >= 0.0f;
}
