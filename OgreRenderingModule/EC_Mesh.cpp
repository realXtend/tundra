// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "OgreRenderingModule.h"
#include "OgreWorld.h"
#include "Renderer.h"
#include "Entity.h"
#include "Scene.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "OgreConversionUtils.h"
#include "OgreSkeletonAsset.h"
#include "OgreMeshAsset.h"
#include "OgreMaterialAsset.h"
#include "IAssetTransfer.h"
#include "AssetAPI.h"
#include "AttributeMetadata.h"

#include <Ogre.h>
#include <OgreTagPoint.h>

#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

using namespace OgreRenderer;

EC_Mesh::EC_Mesh(Scene* scene) :
    IComponent(scene),
    nodeTransformation(this, "Transform", Transform(Vector3df(0,0,0),Vector3df(0,0,0),Vector3df(1,1,1))),
    meshRef(this, "Mesh ref"),
    skeletonRef(this, "Skeleton ref"),
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

    static AttributeMetadata meshRefMetadata;
    AttributeMetadata::ButtonInfoList meshRefButtons;
    meshRefButtons.push_back(AttributeMetadata::ButtonInfo(meshRef.Name(), "V", "View"));
    meshRefMetadata.buttons = meshRefButtons;
    meshRef.SetMetadata(&meshRefMetadata);

    meshAsset = AssetRefListenerPtr(new AssetRefListener());
    skeletonAsset = AssetRefListenerPtr(new AssetRefListener());
    
    OgreWorldPtr world = world_.lock();
    if (world)
    {
        Ogre::SceneManager* sceneMgr = world->GetSceneManager();
        adjustment_node_ = sceneMgr->createSceneNode(world->GetUniqueObjectName("EC_Mesh_adjustment_node"));

        connect(this, SIGNAL(ParentEntitySet()), SLOT(UpdateSignals()));
        connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(OnAttributeUpdated(IAttribute*)));
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
        Ogre::SceneManager* sceneMgr = world->GetSceneManager();
        sceneMgr->destroySceneNode(adjustment_node_);
        adjustment_node_ = 0;
    }
}

void EC_Mesh::SetPlaceable(ComponentPtr placeable)
{
    if (!dynamic_cast<EC_Placeable*>(placeable.get()))
    {
        LogError("Attempted to set placeable which is not " + EC_Placeable::TypeNameStatic().toStdString());
        return;
    }
    
    DetachEntity();
    placeable_ = placeable;
    AttachEntity();
}

void EC_Mesh::SetPlaceable(EC_Placeable* placeable)
{
     ComponentPtr ptr = placeable->GetParentEntity()->GetComponent(placeable->TypeName(), placeable->Name()); //hack to get the shared_ptr to this component
     SetPlaceable(ptr);
}

void EC_Mesh::View(const QString &attributeName)
{
    // todo add implementation.
}

void EC_Mesh::AutoSetPlaceable()
{
    Entity* entity = GetParentEntity();
    if (entity)
    {
        ComponentPtr placeable = entity->GetComponent(EC_Placeable::TypeNameStatic());
        if (placeable)
            SetPlaceable(placeable);
    }
}

void EC_Mesh::SetAdjustPosition(const Vector3df& position)
{
    Transform transform = nodeTransformation.Get();
    transform.SetPos(position.x, position.y, position.z);
    nodeTransformation.Set(transform, AttributeChange::LocalOnly);
}

void EC_Mesh::SetAdjustOrientation(const Quaternion& orientation)
{
    Transform transform = nodeTransformation.Get();
    Vector3df euler;
    orientation.toEuler(euler);
    transform.SetRot(euler.x * RADTODEG, euler.y * RADTODEG, euler.z * RADTODEG);
    nodeTransformation.Set(transform, AttributeChange::LocalOnly);
}

void EC_Mesh::SetAdjustScale(const Vector3df& scale)
{
    Transform transform = nodeTransformation.Get();
    transform.SetScale(scale.x, scale.y, scale.z);
    nodeTransformation.Set(transform, AttributeChange::LocalOnly);
}

void EC_Mesh::SetAttachmentPosition(uint index, const Vector3df& position)
{
    if (index >= attachment_nodes_.size() || attachment_nodes_[index] == 0)
        return;
    
    attachment_nodes_[index]->setPosition(Ogre::Vector3(position.x, position.y, position.z));
}

void EC_Mesh::SetAttachmentOrientation(uint index, const Quaternion& orientation)
{
    if (index >= attachment_nodes_.size() || attachment_nodes_[index] == 0)
        return;
    
    attachment_nodes_[index]->setOrientation(Ogre::Quaternion(orientation.w, orientation.x, orientation.y, orientation.z));
}

void EC_Mesh::SetAttachmentScale(uint index, const Vector3df& scale)
{
    if (index >= attachment_nodes_.size() || attachment_nodes_[index] == 0)
        return;
    
    attachment_nodes_[index]->setScale(Ogre::Vector3(scale.x, scale.y, scale.z));
}

Vector3df EC_Mesh::GetAdjustPosition() const
{
    Transform transform = nodeTransformation.Get();
    return transform.pos;
}

Quaternion EC_Mesh::GetAdjustOrientation() const
{
    Transform transform = nodeTransformation.Get();
    Quaternion orientation(DEGTORAD * transform.rot.x,
                      DEGTORAD * transform.rot.y,
                      DEGTORAD * transform.rot.z);
    return orientation;
}

Vector3df EC_Mesh::GetAdjustScale() const
{
    Transform transform = nodeTransformation.Get();
    return transform.scale;
}

Vector3df EC_Mesh::GetAttachmentPosition(uint index) const
{
    if (index >= attachment_nodes_.size() || attachment_nodes_[index] == 0)
        return Vector3df(0.0f, 0.0f, 0.0f);
        
    const Ogre::Vector3& pos = attachment_nodes_[index]->getPosition();
    return Vector3df(pos.x, pos.y, pos.z);
}

Quaternion EC_Mesh::GetAttachmentOrientation(uint index) const
{
    if (index >= attachment_nodes_.size() || attachment_nodes_[index] == 0)
        return Quaternion();
        
    const Ogre::Quaternion& orientation = attachment_nodes_[index]->getOrientation();
    return Quaternion(orientation.x, orientation.y, orientation.z, orientation.w);
}

Vector3df EC_Mesh::GetAttachmentScale(uint index) const
{
    if (index >= attachment_nodes_.size() || attachment_nodes_[index] == 0)
        return Vector3df(1.0f, 1.0f, 1.0f);
        
    const Ogre::Vector3& scale = attachment_nodes_[index]->getScale();
    return Vector3df(scale.x, scale.y, scale.z);
}

void EC_Mesh::SetDrawDistance(float draw_distance)
{
    drawDistance.Set(draw_distance, AttributeChange::LocalOnly);
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
        Entity* entity = GetParentEntity();
        if (entity)
        {
            ComponentPtr placeable = entity->GetComponent(EC_Placeable::TypeNameStatic());
            if (placeable)
                placeable_ = placeable;
        }
    }
    
    Ogre::SceneManager* sceneMgr = world->GetSceneManager();
    
    Ogre::Mesh* mesh = PrepareMesh(mesh_name, clone);
    if (!mesh)
        return false;
    
    try
    {
        entity_ = sceneMgr->createEntity(world->GetUniqueObjectName("EC_Mesh_entity"), mesh->getName());
        if (!entity_)
        {
            LogError("Could not set mesh " + mesh_name);
            return false;
        }
        
        entity_->setRenderingDistance(drawDistance.Get());
        entity_->setCastShadows(castShadows.Get());
        entity_->setUserAny(Ogre::Any(GetParentEntity()));
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
        adjustment_node_->setPosition(newTransform.pos.x, newTransform.pos.y, newTransform.pos.z);
        Quaternion adjust(DEGTORAD * newTransform.rot.x,
                        DEGTORAD * newTransform.rot.y,
                        DEGTORAD * newTransform.rot.z);
        adjustment_node_->setOrientation(Ogre::Quaternion(adjust.w, adjust.x, adjust.y, adjust.z));
        
        // Prevent Ogre exception from zero scale
        if (newTransform.scale.x < 0.0000001f)
            newTransform.scale.x = 0.0000001f;
        if (newTransform.scale.y < 0.0000001f)
            newTransform.scale.y = 0.0000001f;
        if (newTransform.scale.z < 0.0000001f)
            newTransform.scale.z = 0.0000001f;

        adjustment_node_->setScale(newTransform.scale.x, newTransform.scale.y, newTransform.scale.z);
            
        // Force a re-apply of all materials to this new mesh.
        ApplyMaterial();
    }
    catch(Ogre::Exception& e)
    {
        LogError("Could not set mesh " + mesh_name + ": " + std::string(e.what()));
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

    Ogre::SkeletonPtr skel = Ogre::SkeletonManager::getSingleton().getByName(SanitateAssetIdForOgre(skeleton_name));
    if (skel.isNull())
    {
        LogError("Could not set skeleton " + skeleton_name + " to mesh " + mesh_name + ": not found");
        return false;
    }
    
    RemoveMesh();

    Ogre::SceneManager* sceneMgr = world->GetSceneManager();
    
    Ogre::Mesh* mesh = PrepareMesh(mesh_name, clone);
    if (!mesh)
        return false;
    
    try
    {
        mesh->_notifySkeleton(skel);
//        LogDebug("Set skeleton " + skeleton_name + " to mesh " + mesh_name);
    }
    catch(Ogre::Exception& e)
    {
        LogError("Could not set skeleton " + skeleton_name + " to mesh " + mesh_name + ": " + std::string(e.what()));
        return false;
    }
    
    try
    {
        entity_ = sceneMgr->createEntity(world->GetUniqueObjectName("EC_Mesh_entwithskel"), mesh->getName());
        if (!entity_)
        {
            LogError("Could not set mesh " + mesh_name);
            return false;
        }
        
        entity_->setRenderingDistance(drawDistance.Get());
        entity_->setCastShadows(castShadows.Get());
        entity_->setUserAny(Ogre::Any(GetParentEntity()));
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
    catch(Ogre::Exception& e)
    {
        LogError("Could not set mesh " + mesh_name + ": " + std::string(e.what()));
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
        
        Ogre::SceneManager* sceneMgr = world->GetSceneManager();
        sceneMgr->destroyEntity(entity_);
        
        entity_ = 0;
    }
    
    if (!cloned_mesh_name_.empty())
    {
        try
        {
            Ogre::MeshManager::getSingleton().remove(cloned_mesh_name_);
        }
        catch(Ogre::Exception& e)
        {
            LogWarning("Could not remove cloned mesh:" + std::string(e.what()));
        }
        
        cloned_mesh_name_ = std::string();
    }
}

bool EC_Mesh::SetAttachmentMesh(uint index, const std::string& mesh_name, const std::string& attach_point, bool share_skeleton)
{
    if (!ViewEnabled())
        return false;
    OgreWorldPtr world = world_.lock();

    if (!entity_)
    {
        LogError("No mesh entity created yet, can not create attachments");
        return false;
    }
    
    Ogre::SceneManager* sceneMgr = world->GetSceneManager();
    
    size_t oldsize = attachment_entities_.size();
    size_t newsize = index + 1;
    
    if (oldsize < newsize)
    {
        attachment_entities_.resize(newsize);
        attachment_nodes_.resize(newsize);
        for(uint i = oldsize; i < newsize; ++i)
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
            LogError("Cannot share skeleton for attachment, not found");
            return false;
        }
        try
        {
            mesh->_notifySkeleton(entity_skel);
        }
        catch(Ogre::Exception e)
        {
            LogError("Could not set shared skeleton for attachment");
            return false;
        }
    }

    try
    {
        QString entityName = QString("EC_Mesh_attach") + QString::number(index);
        attachment_entities_[index] = sceneMgr->createEntity(world->GetUniqueObjectName(entityName.toStdString()), mesh->getName());
        if (!attachment_entities_[index])
        {
            LogError("Could not set attachment mesh " + mesh_name);
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
    catch(Ogre::Exception& e)
    {
        LogError("Could not set attachment mesh " + mesh_name + ": " + std::string(e.what()));
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
    
    Ogre::SceneManager* sceneMgr = world->GetSceneManager();
    
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

bool EC_Mesh::SetMaterial(uint index, const std::string& material_name)
{
    if (!entity_)
    {
        // The mesh is not ready yet, track bending applies 
        // so we can apply it once OnMeshAssetLoaded() is called
        pendingMaterialApplies[index] = QString::fromStdString(material_name);
        return false;
    }
    
    if (index >= entity_->getNumSubEntities())
    {
        LogError("Could not set material " + material_name + ": illegal submesh index " + ToString<uint>(index));
        return false;
    }
    
    try
    {
        entity_->getSubEntity(index)->setMaterialName(SanitateAssetIdForOgre(material_name));
        emit MaterialChanged(index, QString(material_name.c_str()));
    }
    catch(Ogre::Exception& e)
    {
        LogError("Could not set material " + material_name + ": " + std::string(e.what()));
        return false;
    }
    
    return true;
}

bool EC_Mesh::SetMaterial(uint index, const QString& material_name) 
{
    return SetMaterial(index, material_name.toStdString());
}

bool EC_Mesh::SetAttachmentMaterial(uint index, uint submesh_index, const std::string& material_name)
{
    if (index >= attachment_entities_.size() || attachment_entities_[index] == 0)
    {
        LogError("Could not set material " + material_name + " on attachment: no mesh");
        return false;
    }
    
    if (submesh_index >= attachment_entities_[index]->getNumSubEntities())
    {
        LogError("Could not set material " + material_name + " on attachment: illegal submesh index " + ToString<uint>(submesh_index));
        return false;
    }
    
    try
    {
        attachment_entities_[index]->getSubEntity(submesh_index)->setMaterialName(SanitateAssetIdForOgre(material_name));
    }
    catch(Ogre::Exception& e)
    {
        LogError("Could not set material " + material_name + " on attachment: " + std::string(e.what()));
        return false;
    }
    
    return true;
}

void EC_Mesh::SetCastShadows(bool enabled)
{
    castShadows.Set(enabled, AttributeChange::LocalOnly);
}

uint EC_Mesh::GetNumMaterials() const
{
    if (!entity_)
        return 0;
        
    return entity_->getNumSubEntities();
}

uint EC_Mesh::GetAttachmentNumMaterials(uint index) const
{
    if (index >= attachment_entities_.size() || attachment_entities_[index] == 0)
        return 0;
        
    return attachment_entities_[index]->getNumSubEntities();
}

const std::string& EC_Mesh::GetMaterialName(uint index) const
{
    const static std::string empty;
    
    if (!entity_)
        return empty;
    
    if (index >= entity_->getNumSubEntities())
        return empty;
    
    return entity_->getSubEntity(index)->getMaterialName();
}

QString EC_Mesh::GetMatName(uint index) const
{
    return GetMaterialName(index).c_str();
}

const std::string& EC_Mesh::GetAttachmentMaterialName(uint index, uint submesh_index) const
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

Ogre::Entity* EC_Mesh::GetAttachmentEntity(uint index) const
{
    if (index >= attachment_entities_.size())
        return 0;
    return attachment_entities_[index];
}

const uint EC_Mesh::GetNumSubMeshes() const
{
    uint count = 0;
    if (HasMesh())
        if (entity_->getMesh().get())
            count = entity_->getMesh()->getNumSubMeshes();
    return count;
}

const std::string& EC_Mesh::GetMeshName() const
{
    static std::string empty_name;
    
    if (!entity_)
        return empty_name;
    else
        return entity_->getMesh()->getName();
}

const std::string& EC_Mesh::GetSkeletonName() const
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
    
void EC_Mesh::GetBoundingBox(Vector3df& min, Vector3df& max) const
{
    if (!entity_)
    {
        min = Vector3df(0.0, 0.0, 0.0);
        max = Vector3df(0.0, 0.0, 0.0);
        return;
    }
 
    const Ogre::AxisAlignedBox& bbox = entity_->getMesh()->getBounds();
    const Ogre::Vector3& bboxmin = bbox.getMinimum();
    const Ogre::Vector3& bboxmax = bbox.getMaximum();
    
    min = Vector3df(bboxmin.x, bboxmin.y, bboxmin.z);
    max = Vector3df(bboxmax.x, bboxmax.y, bboxmax.z);
}

QVector3D EC_Mesh::GetWorldSize() const
{
    QVector3D size(0,0,0);
    if (!entity_ || !adjustment_node_ || !placeable_)
        return size;

    // Get mesh bounds and scale it to the scene node
    EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(placeable_.get());
    Ogre::AxisAlignedBox bbox = entity_->getMesh()->getBounds();
    ///\bug Rewrite this code to properly take the world transform into account. -jj.
    bbox.scale(adjustment_node_->getScale());

    // Get size and take placeable scale into consideration to get real in-world size
    const Ogre::Vector3& bbsize = bbox.getSize();
    const Vector3df &placeable_scale = placeable->GetScale();
    // Swap y and z to make it align with other vectors
    size = QVector3D(bbsize.x*placeable_scale.x, bbsize.y*placeable_scale.y, bbsize.z*placeable_scale.z);
    return size;
}

void EC_Mesh::DetachEntity()
{
    if ((!attached_) || (!entity_) || (!placeable_))
        return;
    
    EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(placeable_.get());
    Ogre::SceneNode* node = placeable->GetSceneNode();
    adjustment_node_->detachObject(entity_);
    node->removeChild(adjustment_node_);
    attached_ = false;
}

void EC_Mesh::AttachEntity()
{
    if ((attached_) || (!entity_) || (!placeable_))
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
    Ogre::MeshPtr mesh = mesh_mgr.getByName(SanitateAssetIdForOgre(mesh_name));
    
    // For local meshes, mesh will not get automatically loaded until used in an entity. Load now if necessary
    if (mesh.isNull())
    {
        try
        {
            mesh_mgr.load(mesh_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            mesh = mesh_mgr.getByName(mesh_name);
        }
        catch(Ogre::Exception& e)
        {
            LogError("Could not load mesh " + mesh_name + ": " + std::string(e.what()));
            return 0;
        }
    }
    
    // If mesh is still null, must abort
    if (mesh.isNull())
    {
        LogError("Mesh " + mesh_name + " does not exist");
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
        catch(Ogre::Exception& e)
        {
            LogError("Could not clone mesh " + mesh_name + ":" + std::string(e.what()));
            return 0;
        }
    }
    
    if (mesh->hasSkeleton())
    {
        Ogre::SkeletonPtr skeleton = Ogre::SkeletonManager::getSingleton().getByName(mesh->getSkeletonName());
        if (skeleton.isNull() || skeleton->getNumBones() == 0)
        {
            LogDebug("Mesh " + mesh_name + " has a skeleton with 0 bones. Disabling the skeleton.");
            mesh->setSkeletonName("");
        }
    }
    
    return mesh.get();
}

void EC_Mesh::UpdateSignals()
{
    Entity* parent = GetParentEntity();
    if (parent)
    {
        // Connect to ComponentRemoved signal of the parent entity, so we can check if the placeable gets removed
        connect(parent, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), SLOT(OnComponentRemoved(IComponent*, AttributeChange::Type)));
    }
}

void EC_Mesh::OnAttributeUpdated(IAttribute *attribute)
{
    if (attribute == &drawDistance)
    {
        if(entity_)
            entity_->setRenderingDistance(drawDistance.Get());
    }
    else if (attribute == &castShadows)
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
    else if (attribute == &nodeTransformation)
    {
        Transform newTransform = nodeTransformation.Get();
        adjustment_node_->setPosition(newTransform.pos.x, newTransform.pos.y, newTransform.pos.z);
        Quaternion adjust(DEGTORAD * newTransform.rot.x,
                          DEGTORAD * newTransform.rot.y,
                          DEGTORAD * newTransform.rot.z);
        adjustment_node_->setOrientation(Ogre::Quaternion(adjust.w, adjust.x, adjust.y, adjust.z));
        
        // Prevent Ogre exception from zero scale
        if (newTransform.scale.x < 0.0000001f)
            newTransform.scale.x = 0.0000001f;
        if (newTransform.scale.y < 0.0000001f)
            newTransform.scale.y = 0.0000001f;
        if (newTransform.scale.z < 0.0000001f)
            newTransform.scale.z = 0.0000001f;
        
        adjustment_node_->setScale(newTransform.scale.x, newTransform.scale.y, newTransform.scale.z);
    }
    else if (attribute == &meshRef)
    {
        if (!ViewEnabled())
            return;
            
        //Ensure that mesh is requested only when it's has actually changed.
//        if(entity_)
 //           if(QString::fromStdString(entity_->getMesh()->getName()) == meshRef.Get().ref/*meshResourceId.Get()*/)
  //              return;
/*
        AssetTransferPtr transfer = GetFramework()->Asset()->RequestAsset(meshRef.Get());
        if (transfer)
        {
            connect(transfer.get(), SIGNAL(Succeeded(AssetPtr)), SLOT(OnMeshAssetLoaded()), Qt::UniqueConnection);
        }
        else
        {
            RemoveMesh();
        }
        */
        if (meshRef.Get().ref.trimmed().isEmpty())
            LogDebug("Warning: Mesh \"" + this->parentEntity_->GetName().toStdString() + "\" mesh ref was set to an empty reference!");
        meshAsset->HandleAssetRefChange(&meshRef);
    }
    else if (attribute == &meshMaterial)
    {
        if (!ViewEnabled())
            return;
        
        // We won't request materials until we are sure that mesh has been loaded and it's safe to apply materials into it.
        // This logic shouldn't be necessary anymore. -jj.
//        if(!HasMaterialsChanged())
//            return;

        AssetReferenceList materials = meshMaterial.Get();
        // Make sure that the asset ref list type stays intact.
        materials.type = "OgreMaterial";
        meshMaterial.Set(materials, AttributeChange::Disconnected);

        // Reallocate the number of material asset reflisteners.
        while(materialAssets.size() > (size_t)materials.Size())
            materialAssets.pop_back();
        while(materialAssets.size() < (size_t)materials.Size())
            materialAssets.push_back(boost::shared_ptr<AssetRefListener>(new AssetRefListener));

        for(int i = 0; i < materials.Size(); ++i)
        {
            connect(materialAssets[i].get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnMaterialAssetLoaded(AssetPtr)), Qt::UniqueConnection);
            connect(materialAssets[i].get(), SIGNAL(TransferFailed(IAssetTransfer*, QString)), this, SLOT(OnMaterialAssetFailed(IAssetTransfer*, QString)), Qt::UniqueConnection);
            materialAssets[i]->HandleAssetRefChange(framework_->Asset(), materials[i].ref);
        }
    }
    else if((attribute == &skeletonRef) && (!skeletonRef.Get().ref.isEmpty()))
    {
        if (!ViewEnabled())
            return;
        
        // If same name skeleton already set no point to do it again.
//        if (entity_ && entity_->getSkeleton() && entity_->getSkeleton()->getName() == skeletonRef.Get().ref/*skeletonId.Get()*/.toStdString())
 //           return;

  //      AssetTransferPtr transfer = GetFramework()->Asset()->RequestAsset(skeletonRef.Get().ref);
   //     if (transfer)
    //        connect(transfer.get(), SIGNAL(Succeeded(AssetPtr)), SLOT(OnSkeletonAssetLoaded(AssetPtr)), Qt::UniqueConnection);
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
        LogError("OnMeshAssetLoaded: Mesh asset load finished for asset \"" + asset->Name().toStdString() + "\", but downloaded asset was not of type OgreMeshAsset!");
        return;
    }

    QString ogreMeshName = mesh->Name();
    if (mesh)
    {
        if (mesh->ogreMesh.get())
            ogreMeshName = QString::fromStdString(mesh->ogreMesh->getName()).trimmed();
        else
            LogError("OnMeshAssetLoaded: Mesh asset load finished for asset \"" + asset->Name().toStdString() + "\", but Ogre::Mesh pointer was null!");
    }

    SetMesh(ogreMeshName);

    // Force a re-application of the skeleton on this mesh. ///\todo This path should be re-evaluated to see if we have potential performance issues here. -jj.
    if (skeletonAsset->Asset())
        OnSkeletonAssetLoaded(skeletonAsset->Asset());

    // Apply pending materials, these were tried to be applied before the mesh was loaded
    if (!pendingMaterialApplies.empty())
    {
        for(int idx = 0; idx < (int)pendingMaterialApplies.size(); ++idx)
            SetMaterial(idx, pendingMaterialApplies[idx]);
        pendingMaterialApplies.clear();
    }
}

void EC_Mesh::OnSkeletonAssetLoaded(AssetPtr asset)
{
    OgreSkeletonAsset *skeletonAsset = dynamic_cast<OgreSkeletonAsset*>(asset.get());
    if (!skeletonAsset)
    {
        LogError("OnSkeletonAssetLoaded: Skeleton asset load finished for asset \"" +
            asset->Name().toStdString() + "\", but downloaded asset was not of type OgreSkeletonAsset!");
        return;
    }

    Ogre::SkeletonPtr skeleton = skeletonAsset->ogreSkeleton;
    if (skeleton.isNull())
    {
        LogError("OnSkeletonAssetLoaded: Skeleton asset load finished for asset \"" +
            asset->Name().toStdString() + "\", but Ogre::Skeleton pointer was null!");
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
            asset->Name().toStdString() + "\", but downloaded asset was not of type OgreMaterialAsset!");
        return;
    }

    Ogre::MaterialPtr material = ogreMaterial->ogreMaterial;
    bool assetUsed = false;

    AssetReferenceList materialList = meshMaterial.Get();
    for(int i = 0; i < materialList.Size(); ++i)
        if (materialList[i].ref == ogreMaterial->Name() ||
            framework_->Asset()->ResolveAssetRef("", materialList[i].ref) == ogreMaterial->Name()) ///<///\todo The design of whether the ResolveAssetRef should occur here, or internal to Asset API needs to be revisited.
        {
            SetMaterial(i, ogreMaterial->Name());
            assetUsed = true;
        }

    // This check & debug print is now in Debug mode only. Rapid changes in materials and the delay-loaded nature of assets makes it unavoidable in some cases.
    #ifdef _DEBUG
    if (!assetUsed)
    {
        LogDebug("OnMaterialAssetLoaded: Trying to apply material \"" + ogreMaterial->Name().toStdString() + "\" to mesh " +
            meshRef.Get().ref.toStdString() + ", but no submesh refers to the given material! The references are: ");
        for(int i = 0; i < materialList.Size(); ++i)
            LogDebug(QString::number(i).toStdString() + ": " + materialList[i].ref.toStdString());
    }
    #endif
}

void EC_Mesh::OnMaterialAssetFailed(IAssetTransfer* transfer, QString reason)
{
    // Check which of the material(s) match the failed ref
    AssetReferenceList materialList = meshMaterial.Get();
    for(int i = 0; i < materialList.Size(); ++i)
    {
        QString absoluteRef = framework_->Asset()->ResolveAssetRef("", materialList[i].ref);
        if (absoluteRef == transfer->source.ref)
            SetMaterial(i, QString("AssetLoadError"));
    }
}

void EC_Mesh::ApplyMaterial()
{
    AssetReferenceList materialList = meshMaterial.Get();
    AssetAPI *assetAPI = framework_->Asset();
    for(int i = 0; i < materialList.Size(); ++i)
    {
        if (!materialList[i].ref.isEmpty())
        {
            // Only apply the material if it is loaded and has no dependencies
            QString assetFullName = assetAPI->ResolveAssetRef("", materialList[i].ref);
            AssetPtr asset = assetAPI->GetAsset(assetFullName);
            if ((asset) && (assetAPI->NumPendingDependencies(asset) == 0))
                SetMaterial(i, assetFullName);
        }
    }
}

Ogre::Bone* EC_Mesh::GetBone(const QString& bone_name)
{
    std::string boneNameStd = bone_name.toStdString();
    
    if (!entity_)
        return 0;
    Ogre::Skeleton* skel = entity_->getSkeleton();
    if (skel && skel->hasBone(boneNameStd))
        return skel->getBone(boneNameStd);
    else
        return 0;
}

QStringList EC_Mesh::GetAvailableBones() const
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

Vector3df EC_Mesh::GetBonePosition(const QString& bone_name)
{
    Ogre::Bone* bone = GetBone(bone_name);
    if (bone)
    {
        const Ogre::Vector3& pos = bone->getPosition();
        return Vector3df(pos.x, pos.y, pos.z);
    }
    else
        return Vector3df::ZERO;
}

Vector3df EC_Mesh::GetBoneDerivedPosition(const QString& bone_name)
{
    Ogre::Bone* bone = GetBone(bone_name);
    if (bone)
    {
        Ogre::Vector3 pos = bone->_getDerivedPosition();
        return Vector3df(pos.x, pos.y, pos.z);
    }
    else
        return Vector3df::ZERO;
}

Quaternion EC_Mesh::GetBoneOrientation(const QString& bone_name)
{
    Ogre::Bone* bone = GetBone(bone_name);
    if (bone)
    {
        const Ogre::Quaternion& quat = bone->getOrientation();
        return Quaternion(quat.x, quat.y, quat.z, quat.w);
    }
    else
        return Quaternion::IDENTITY;
}

Quaternion EC_Mesh::GetBoneDerivedOrientation(const QString& bone_name)
{
    Ogre::Bone* bone = GetBone(bone_name);
    if (bone)
    {
        const Ogre::Quaternion& quat = bone->_getDerivedOrientation();
        return Quaternion(quat.x, quat.y, quat.z, quat.w);
    }
    else
        return Quaternion::IDENTITY;
}

Vector3df EC_Mesh::GetBoneOrientationEuler(const QString& bone_name)
{
    Ogre::Bone* bone = GetBone(bone_name);
    if (bone)
    {
        const Ogre::Quaternion& quat = bone->getOrientation();
        Quaternion q(quat.x, quat.y, quat.z, quat.w);
        Vector3df euler;
        q.toEuler(euler);
        return euler * RADTODEG;
    }
    else
        return Vector3df::ZERO;
}

Vector3df EC_Mesh::GetBoneDerivedOrientationEuler(const QString& bone_name)
{
    Ogre::Bone* bone = GetBone(bone_name);
    if (bone)
    {
        const Ogre::Quaternion& quat = bone->_getDerivedOrientation();
        Quaternion q(quat.x, quat.y, quat.z, quat.w);
        Vector3df euler;
        q.toEuler(euler);
        return euler * RADTODEG;
    }
    else
        return Vector3df::ZERO;
}

bool EC_Mesh::HasMaterialsChanged() const
{
    if(!entity_ || !meshMaterial.Get().Size())
        return false;

    AssetReferenceList materials = meshMaterial.Get();
    for(uint i = 0; i < entity_->getNumSubEntities(); i++)
    {
        // No point to continue if all materials are not set.
        if(i >= (uint)materials.Size())
            break;

        if(entity_->getSubEntity(i)->getMaterial()->getName() != SanitateAssetIdForOgre(materials[i].ref.toStdString()))
            return true;
    }
    return false;
}
