// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "Entity.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "OgreConversionUtils.h"
#include "OgreSkeletonAsset.h"
#include "OgreMeshAsset.h"
#include "OgreMaterialAsset.h"
#include "IAssetTransfer.h"
#include "AssetAPI.h"

#include <Ogre.h>
#include <OgreTagPoint.h>

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_Mesh")

#include "MemoryLeakCheck.h"

using namespace OgreRenderer;

EC_Mesh::EC_Mesh(IModule* module) :
    IComponent(module->GetFramework()),
    // Note: we put the opensim haxor adjust right here in the defaults, instead of hardcoding it in code.
    nodeTransformation(this, "Transform", Transform(Vector3df(0,0,0),Vector3df(90,0,180),Vector3df(1,1,1))),
    meshRef(this, "Mesh ref"),
    skeletonRef(this, "Skeleton ref"),
    meshMaterial(this, "Mesh materials"),
    drawDistance(this, "Draw distance", 0.0f),
    castShadows(this, "Cast shadows", false),
    renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
    entity_(0),
    bone_tagpoint_(0),
    bone_parent_mesh_(0),
    bone_attached_mesh_(0),
    attached_(false),
    attached_to_bone_(false)
{
    static AttributeMetadata drawDistanceData("", "0", "10000");
    drawDistance.SetMetadata(&drawDistanceData);

    static AttributeMetadata materialMetadata;
    materialMetadata.elementType = "assetreference";
    meshMaterial.SetMetadata(&materialMetadata);

    static AttributeMetadata meshRefMetadata;
    AttributeMetadata::ButtonInfoList meshRefButtons;
    meshRefButtons.push_back(AttributeMetadata::ButtonInfo(meshRef.GetName(), "V", "View"));
    meshRefMetadata.buttons = meshRefButtons;
    meshRef.SetMetadata(&meshRefMetadata);

    RendererPtr renderer = renderer_.lock();
    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    adjustment_node_ = scene_mgr->createSceneNode(renderer->GetUniqueObjectName("EC_Mesh_adjustment_node"));

    connect(this, SIGNAL(ParentEntitySet()), SLOT(UpdateSignals()));
    connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(AttributeUpdated(IAttribute*)));

    meshAsset = AssetRefListenerPtr(new AssetRefListener());
    connect(meshAsset.get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnMeshAssetLoaded(AssetPtr)), Qt::UniqueConnection);
    skeletonAsset = AssetRefListenerPtr(new AssetRefListener());
    connect(skeletonAsset.get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnSkeletonAssetLoaded(AssetPtr)), Qt::UniqueConnection);
}

EC_Mesh::~EC_Mesh()
{
    if (renderer_.expired())
        return;
    RendererPtr renderer = renderer_.lock();

    RemoveMesh();

    if (adjustment_node_)
    {
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        scene_mgr->destroySceneNode(adjustment_node_);
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
    Scene::Entity* entity = GetParentEntity();
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
    return transform.position;
}

Quaternion EC_Mesh::GetAdjustOrientation() const
{
    Transform transform = nodeTransformation.Get();
    Quaternion orientation(DEGTORAD * transform.rotation.x,
                      DEGTORAD * transform.rotation.y,
                      DEGTORAD * transform.rotation.z);
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
    
    if (renderer_.expired())
        return false;
    RendererPtr renderer = renderer_.lock();

    std::string mesh_name = meshResourceName.trimmed().toStdString();

    RemoveMesh();

    // If placeable is not set yet, set it manually by searching it from the parent entity
    if (!placeable_)
    {
        Scene::Entity* entity = GetParentEntity();
        if (entity)
        {
            ComponentPtr placeable = entity->GetComponent(EC_Placeable::TypeNameStatic());
            if (placeable)
                placeable_ = placeable;
        }
    }
    
    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    
    Ogre::Mesh* mesh = PrepareMesh(mesh_name, clone);
    if (!mesh)
        return false;
    
    try
    {
        entity_ = scene_mgr->createEntity(renderer->GetUniqueObjectName("EC_Mesh_entity"), mesh->getName());
        if (!entity_)
        {
            LogError("Could not set mesh " + mesh_name);
            return false;
        }
        
        entity_->setRenderingDistance(drawDistance.Get());
        entity_->setCastShadows(castShadows.Get());
        entity_->setUserAny(Ogre::Any(GetParentEntity()));
        // Set UserAny also on subentities
        for (uint i = 0; i < entity_->getNumSubEntities(); ++i)
            entity_->getSubEntity(i)->setUserAny(entity_->getUserAny());
                
        if (entity_->hasSkeleton())
        {
            Ogre::SkeletonInstance* skel = entity_->getSkeleton();
            // Enable cumulative mode on skeletal animations
            if (skel)
                skel->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);
        }
        
        // Make sure adjustment node is uptodate
        if (adjustment_node_)
        {
            Transform newTransform = nodeTransformation.Get();
            adjustment_node_->setPosition(newTransform.position.x, newTransform.position.y, newTransform.position.z);
            Quaternion adjust(DEGTORAD * newTransform.rotation.x,
                            DEGTORAD * newTransform.rotation.y,
                            DEGTORAD * newTransform.rotation.z);
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

        // Force a re-apply of all materials to this new mesh.
        ApplyMaterial();
    }
    catch (Ogre::Exception& e)
    {
        LogError("Could not set mesh " + mesh_name + ": " + std::string(e.what()));
        return false;
    }
    
    AttachEntity();
    emit OnMeshChanged();
    
    return true;
}

bool EC_Mesh::SetMeshWithSkeleton(const std::string& mesh_name, const std::string& skeleton_name, bool clone)
{
    if (!ViewEnabled())
        return false;
    if (renderer_.expired())
        return false;
    RendererPtr renderer = renderer_.lock();

    Ogre::SkeletonPtr skel = Ogre::SkeletonManager::getSingleton().getByName(SanitateAssetIdForOgre(skeleton_name));
    if (skel.isNull())
    {
        LogError("Could not set skeleton " + skeleton_name + " to mesh " + mesh_name + ": not found");
        return false;
    }
    
    RemoveMesh();

    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    
    Ogre::Mesh* mesh = PrepareMesh(mesh_name, clone);
    if (!mesh)
        return false;
    
    try
    {
        mesh->_notifySkeleton(skel);
        LogDebug("Set skeleton " + skeleton_name + " to mesh " + mesh_name);
    }
    catch (Ogre::Exception& e)
    {
        LogError("Could not set skeleton " + skeleton_name + " to mesh " + mesh_name + ": " + std::string(e.what()));
        return false;
    }
    
    try
    {
        entity_ = scene_mgr->createEntity(renderer->GetUniqueObjectName("EC_Mesh_entwithskel"), mesh->getName());
        if (!entity_)
        {
            LogError("Could not set mesh " + mesh_name);
            return false;
        }
        
        entity_->setRenderingDistance(drawDistance.Get());
        entity_->setCastShadows(castShadows.Get());
        entity_->setUserAny(Ogre::Any(GetParentEntity()));
        // Set UserAny also on subentities
        for (uint i = 0; i < entity_->getNumSubEntities(); ++i)
            entity_->getSubEntity(i)->setUserAny(entity_->getUserAny());
        
        if (entity_->hasSkeleton())
        {
            Ogre::SkeletonInstance* skel = entity_->getSkeleton();
            // Enable cumulative mode on skeletal animations
            if (skel)
                skel->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);
        }
    }
    catch (Ogre::Exception& e)
    {
        LogError("Could not set mesh " + mesh_name + ": " + std::string(e.what()));
        return false;
    }
    
    AttachEntity();
    
    emit OnMeshChanged();
    
    return true;
}

void EC_Mesh::RemoveMesh()
{
    if (renderer_.expired())
        return;
    RendererPtr renderer = renderer_.lock();

    if (entity_)
    {
        if (attached_to_bone_)
            DetachMeshFromBone();
        
        RemoveAllAttachments();
        DetachEntity();
        
        if (bone_attached_mesh_)
            bone_attached_mesh_->DetachMeshFromBone();
        
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        scene_mgr->destroyEntity(entity_);
        
        entity_ = 0;
    }
    
    if (!cloned_mesh_name_.empty())
    {
        try
        {
            Ogre::MeshManager::getSingleton().remove(cloned_mesh_name_);
        }
        catch (Ogre::Exception& e)
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
    if (renderer_.expired())
        return false;
    RendererPtr renderer = renderer_.lock();

    if (!entity_)
    {
        LogError("No mesh entity created yet, can not create attachments");
        return false;
    }
    
    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    
    size_t oldsize = attachment_entities_.size();
    size_t newsize = index + 1;
    
    if (oldsize < newsize)
    {
        attachment_entities_.resize(newsize);
        attachment_nodes_.resize(newsize);
        for (uint i = oldsize; i < newsize; ++i)
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
        catch (Ogre::Exception e)
        {
            LogError("Could not set shared skeleton for attachment");
            return false;
        }
    }

    try
    {
        QString entityName = QString("EC_Mesh_attach") + QString::number(index);
        attachment_entities_[index] = scene_mgr->createEntity(renderer->GetUniqueObjectName(entityName.toStdString()), mesh->getName());
        if (!attachment_entities_[index])
        {
            LogError("Could not set attachment mesh " + mesh_name);
            return false;
        }

        attachment_entities_[index]->setRenderingDistance(drawDistance.Get());
        attachment_entities_[index]->setCastShadows(castShadows.Get());
        attachment_entities_[index]->setUserAny(entity_->getUserAny());
        // Set UserAny also on subentities
        for (uint i = 0; i < attachment_entities_[index]->getNumSubEntities(); ++i)
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
            Ogre::SceneNode* node = scene_mgr->createSceneNode(renderer->GetUniqueObjectName(nodeName.toStdString()));
            node->attachObject(attachment_entities_[index]);
            adjustment_node_->addChild(node);
            attachment_nodes_[index] = node;
        }
        
        if (share_skeleton && entity_->hasSkeleton() && attachment_entities_[index]->hasSkeleton())
        {
            attachment_entities_[index]->shareSkeletonInstanceWith(entity_);
        }
    }
    catch (Ogre::Exception& e)
    {
        LogError("Could not set attachment mesh " + mesh_name + ": " + std::string(e.what()));
        return false;
    }
    return true;
}

void EC_Mesh::RemoveAttachmentMesh(uint index)
{
    if (renderer_.expired())
        return;
    RendererPtr renderer = renderer_.lock();   
        
    if (!entity_)
        return;
        
    if (index >= attachment_entities_.size())
        return;
    
    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    
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
                scene_mgr->destroySceneNode(scenenode);
            }
        }
        
        attachment_nodes_[index] = 0;
    }
    if (attachment_entities_[index])
    {
        if (attachment_entities_[index]->sharesSkeletonInstance())
            attachment_entities_[index]->stopSharingSkeletonInstance();
        scene_mgr->destroyEntity(attachment_entities_[index]);
        attachment_entities_[index] = 0;
    }
}

void EC_Mesh::RemoveAllAttachments()
{
    for (uint i = 0; i < attachment_entities_.size(); ++i)
        RemoveAttachmentMesh(i);
    attachment_entities_.clear();
    attachment_nodes_.clear();
}

bool EC_Mesh::SetMaterial(uint index, const std::string& material_name)
{
    if (!entity_)
        return false;
    
    if (index >= entity_->getNumSubEntities())
    {
        LogError("Could not set material " + material_name + ": illegal submesh index " + ToString<uint>(index));
        return false;
    }
    
    try
    {
        entity_->getSubEntity(index)->setMaterialName(SanitateAssetIdForOgre(material_name));
        emit OnMaterialChanged(index, QString(material_name.c_str()));
    }
    catch (Ogre::Exception& e)
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
    catch (Ogre::Exception& e)
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

const QString& EC_Mesh::GetMatName(uint index) const
{
    const QString &name(GetMaterialName(index).c_str());
    return name;
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
    bbox.scale(adjustment_node_->getScale());

    // Get size and take placeable scale into consideration to get real naali inworld size
    const Ogre::Vector3& bbsize = bbox.getSize();
    const Vector3df &placeable_scale = placeable->GetScale();
    // Swap y and z to make it align with other naali vectors
    size = QVector3D(bbsize.x*placeable_scale.x, bbsize.z*placeable_scale.z, bbsize.y*placeable_scale.y);
    return size;
}

void EC_Mesh::DetachEntity()
{
    if (attached_to_bone_)
        return;
    
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
    if (attached_to_bone_)
        return;
    
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
    if (renderer_.expired())
        return 0;
    RendererPtr renderer = renderer_.lock();   
        
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
        catch (Ogre::Exception& e)
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
            mesh = mesh->clone(renderer->GetUniqueObjectName("EC_Mesh_clone"));
            mesh->setAutoBuildEdgeLists(false);
            cloned_mesh_name_ = mesh->getName();
        }
        catch (Ogre::Exception& e)
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
    Scene::Entity* parent = GetParentEntity();
    if (parent)
    {
        // Connect to ComponentRemoved signal of the parent entity, so we can check if the mesh gets removed
        connect(parent, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), SLOT(OnComponentRemoved(IComponent*, AttributeChange::Type)));
    }
}

void EC_Mesh::AttributeUpdated(IAttribute *attribute)
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
            //! \todo might want to disable shadows for some attachments
            for (uint i = 0; i < attachment_entities_.size(); ++i)
            {
                if (attachment_entities_[i])
                    attachment_entities_[i]->setCastShadows(castShadows.Get());
            }
        }
    }
    else if (attribute == &nodeTransformation)
    {
        Ogre::Node* adjustmentTarget = adjustment_node_;
        if (bone_tagpoint_)
            adjustmentTarget = bone_tagpoint_;
        
        if (adjustmentTarget)
        {
            Transform newTransform = nodeTransformation.Get();
            adjustmentTarget->setPosition(newTransform.position.x, newTransform.position.y, newTransform.position.z);
            Quaternion adjust(DEGTORAD * newTransform.rotation.x,
                              DEGTORAD * newTransform.rotation.y,
                              DEGTORAD * newTransform.rotation.z);
            // Let's not assume the needed haxor adjustment here, but let user specify it as necessary
            //adjust = Quaternion(PI/2, 0, PI) * adjust;
            adjustmentTarget->setOrientation(Ogre::Quaternion(adjust.w, adjust.x, adjust.y, adjust.z));
            
            // Prevent Ogre exception from zero scale
            if (newTransform.scale.x < 0.0000001f)
                newTransform.scale.x = 0.0000001f;
            if (newTransform.scale.y < 0.0000001f)
                newTransform.scale.y = 0.0000001f;
            if (newTransform.scale.z < 0.0000001f)
                newTransform.scale.z = 0.0000001f;
            
            adjustmentTarget->setScale(newTransform.scale.x, newTransform.scale.y, newTransform.scale.z);
        }
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
            connect(transfer.get(), SIGNAL(Loaded(AssetPtr)), SLOT(OnMeshAssetLoaded()), Qt::UniqueConnection);
        }
        else
        {
            RemoveMesh();
        }
        */
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

        // Reallocate the number of material asset reflisteners.
        while(materialAssets.size() > materials.Size())
            materialAssets.pop_back();
        while(materialAssets.size() < materials.Size())
            materialAssets.push_back(boost::shared_ptr<AssetRefListener>(new AssetRefListener));

        for(int i = 0; i < materials.Size(); ++i)
        {
            connect(materialAssets[i].get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnMaterialAssetLoaded(AssetPtr)), Qt::UniqueConnection);
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
    //        connect(transfer.get(), SIGNAL(Loaded(AssetPtr)), SLOT(OnSkeletonAssetLoaded(AssetPtr)), Qt::UniqueConnection);
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
            ogreMeshName = mesh->ogreMesh->getName().c_str();
        else
            LogError("OnMeshAssetLoaded: Mesh asset load finished for asset \"" + asset->Name().toStdString() + "\", but Ogre::Mesh pointer was null!");
    }

    SetMesh(ogreMeshName);

    // Force a re-application of the skeleton on this mesh. ///\todo This path should be re-evaluated to see if we have potential performance issues here. -jj.
    if (skeletonAsset->Asset())
        OnSkeletonAssetLoaded(skeletonAsset->Asset());
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
        LogDebug("Could not set skeleton yet because entity is not yet created");
        return;
    }

    try
    {
        // If old skeleton is same as a new one no need to replace it.
        if (entity_->getSkeleton() && entity_->getSkeleton()->getName() == skeleton->getName())
            return;
        
        entity_->getMesh()->_notifySkeleton(skeleton);
        
        LogDebug("Set skeleton " + skeleton->getName() + " to mesh " + entity_->getName());
        emit OnSkeletonChanged(QString::fromStdString(skeleton->getName()));
    }
    catch (...)
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
            framework_->Asset()->LookupAssetRefToStorage(materialList[i].ref) == ogreMaterial->Name()) ///<///\todo The design of whether the LookupAssetRefToStorage should occur here, or internal to Asset API needs to be revisited.
        {
            SetMaterial(i, ogreMaterial->Name());
            assetUsed = true;
        }

    if (!assetUsed)
    {
        LogWarning("OnMaterialAssetLoaded: Trying to apply material \"" + ogreMaterial->Name().toStdString() + "\" to mesh " +
            meshRef.Get().ref.toStdString() + ", but no submesh refers to the given material! The references are: ");
        for(int i = 0; i < materialList.Size(); ++i)
            LogWarning(QString::number(i).toStdString() + ": " + materialList[i].ref.toStdString());
    }
}

void EC_Mesh::ApplyMaterial()
{
    AssetReferenceList materialList = meshMaterial.Get();
    for(int i = 0; i < materialList.Size(); ++i)
        if (!materialList[i].ref.isEmpty())
            SetMaterial(i, materialList[i].ref);
}

bool EC_Mesh::HasMaterialsChanged() const
{
    if(!entity_ || !meshMaterial.Get().Size())
        return false;

    AssetReferenceList materials = meshMaterial.Get();
    for(uint i = 0; i < entity_->getNumSubEntities(); i++)
    {
        // No point to continue if all materials are not set.
        if(i >= materials.Size())
            break;

        if(entity_->getSubEntity(i)->getMaterial()->getName() != SanitateAssetIdForOgre(materials[i].ref.toStdString()))
            return true;
    }
    return false;
}

bool EC_Mesh::AttachMeshToBone(QObject* targetMesh, const QString& boneName)
{
    if (!entity_)
        return false;
    
    // First make sure that the target mesh is valid, and the bone can be found
    EC_Mesh* targetMeshPtr = dynamic_cast<EC_Mesh*>(targetMesh);
    if (!targetMeshPtr)
        return false;
    Ogre::Entity* targetEntity = targetMeshPtr->GetEntity();
    if (!targetEntity)
        return false;
    std::string boneNameStd = boneName.toStdString();
    Ogre::SkeletonInstance* skeleton = targetEntity->getSkeleton();
    if (!skeleton)
        return false;
    if (!skeleton->hasBone(boneNameStd))
        return false;
    
    // We are ready to go. Detach the entity from its normal scene node first
    DetachMeshFromBone();
    DetachEntity();
    
    bone_tagpoint_ = targetEntity->attachObjectToBone(boneNameStd, entity_);
    bone_parent_mesh_ = targetMeshPtr;
    bone_parent_mesh_->bone_attached_mesh_ = this;
    attached_to_bone_ = true;
    
    // Force the adjustment for the tagpoint now
    AttributeUpdated(&nodeTransformation);
    
    return true;
}

void EC_Mesh::DetachMeshFromBone()
{
    if ((!entity_) || (!attached_to_bone_) || (!bone_parent_mesh_))
        return;
    
    Ogre::Entity* targetEntity = bone_parent_mesh_->GetEntity();
    if (targetEntity)
        targetEntity->detachObjectFromBone(entity_);
    else
        return;
    
    bone_parent_mesh_->bone_attached_mesh_ = 0;
    bone_parent_mesh_ = 0;
    attached_to_bone_ = false;
    bone_tagpoint_ = 0;
    
    // Reattach entity to normal placeable
    AttachEntity();
}

