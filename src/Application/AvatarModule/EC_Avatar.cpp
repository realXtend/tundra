// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#define MATH_OGRE_INTEROP
#include "DebugOperatorNew.h"
#include "EC_Avatar.h"
#include "EC_Mesh.h"
#include "EC_Placeable.h"
#include "AssetAPI.h"
#include "IAssetTransfer.h"
#include "AvatarDescAsset.h"
#include "Entity.h"
#include "Profiler.h"
#include <Ogre.h>
#include <QDomDocument>

#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

void ApplyBoneModifier(Entity* entity, const BoneModifier& modifier, float value);
void ResetBones(Entity* entity);
Ogre::Bone* GetAvatarBone(Entity* entity, const std::string& bone_name);
void HideVertices(Ogre::Entity*, std::set<uint> vertices_to_hide);
void GetInitialDerivedBonePosition(Ogre::Node* bone, Ogre::Vector3& position);

// Regrettable magic value
static const float FIXED_HEIGHT_OFFSET = -0.87f;

EC_Avatar::EC_Avatar(Scene* scene) :
    IComponent(scene),
    appearanceRef(this, "Appearance ref", AssetReference("", "Avatar"))
{
    avatarAssetListener_ = AssetRefListenerPtr(new AssetRefListener());
    connect(avatarAssetListener_.get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnAvatarAppearanceLoaded(AssetPtr)), Qt::UniqueConnection);
    connect(avatarAssetListener_.get(), SIGNAL(TransferFailed(IAssetTransfer *, QString)), this, SLOT(OnAvatarAppearanceFailed(IAssetTransfer*, QString)));
}

EC_Avatar::~EC_Avatar()
{
}

void EC_Avatar::OnAvatarAppearanceFailed(IAssetTransfer* transfer, QString reason)
{
    LogError("OnAvatarAppearanceFailed, reason " + reason);
}

void EC_Avatar::OnAvatarAppearanceLoaded(AssetPtr asset)
{
    if (!asset)
        return;

    Entity* entity = ParentEntity();
    if (!entity)
        return;

    AvatarDescAssetPtr avatarAsset = dynamic_pointer_cast<AvatarDescAsset>(asset);
    if (!avatarAsset)
        return;
        
    // Disconnect old change signals, connect new
    AvatarDescAsset* oldDesc = avatarAsset_.lock().get();
    AvatarDescAsset* newDesc = avatarAsset.get();
    if (oldDesc != newDesc)
    {
        if (oldDesc)
        {
            disconnect(oldDesc, SIGNAL(AppearanceChanged()), this, SLOT(SetupAppearance()));
            disconnect(oldDesc, SIGNAL(DynamicAppearanceChanged()), this, SLOT(SetupDynamicAppearance()));
        }
        connect(newDesc, SIGNAL(AppearanceChanged()), this, SLOT(SetupAppearance()));
        connect(newDesc, SIGNAL(DynamicAppearanceChanged()), this, SLOT(SetupDynamicAppearance()));
    }
    
    avatarAsset_ = avatarAsset;
    
    // Create components the avatar needs, with network sync disabled, if they don't exist yet
    // Note: the mesh is created non-syncable on purpose, as each client's EC_Avatar should execute this code upon receiving the appearance
    ComponentPtr mesh = entity->GetOrCreateComponent(EC_Mesh::TypeNameStatic(), AttributeChange::LocalOnly, false);
    EC_Mesh *mesh_ptr = checked_static_cast<EC_Mesh*>(mesh.get());
    // Attach to placeable if not yet attached
    if (mesh_ptr && !mesh_ptr->GetPlaceable())
        mesh_ptr->SetPlaceable(entity->GetComponent(EC_Placeable::TypeNameStatic()));
    
    SetupAppearance();
}

void EC_Avatar::AttributesChanged()
{
    if (appearanceRef.ValueChanged())
    {
        QString ref = appearanceRef.Get().ref.trimmed();
        if (ref.isEmpty())
            return;
        
        avatarAssetListener_->HandleAssetRefChange(&appearanceRef, "Avatar");
    }
}

void EC_Avatar::SetupAppearance()
{
    PROFILE(Avatar_SetupAppearance);
    
    Entity* entity = ParentEntity();
    AvatarDescAssetPtr desc = AvatarDesc();
    if ((!desc) || (!entity))
        return;
    
    EC_Mesh* mesh = entity->GetComponent<EC_Mesh>().get();
    if (!mesh)
        return;

    // If mesh ref is empty, it would certainly be an epic fail. Do nothing.
    if (!desc->mesh_.length())
        return;
    
    // Setup appearance
    SetupMeshAndMaterials();
    SetupDynamicAppearance();
    SetupAttachments();
}

void EC_Avatar::SetupDynamicAppearance()
{
    Entity* entity = ParentEntity();
    AvatarDescAssetPtr desc = AvatarDesc();
    if ((!desc) || (!entity))
        return;
    
    EC_Mesh* mesh = entity->GetComponent<EC_Mesh>().get();

    if (!mesh)
        return;
    
    SetupMorphs();
    SetupBoneModifiers();
    AdjustHeightOffset();
}

AvatarDescAssetPtr EC_Avatar::AvatarDesc() const
{
    return avatarAsset_.lock();
}

QString EC_Avatar::AvatarProperty(const QString& name) const
{
    AvatarDescAssetPtr desc = AvatarDesc();
    if (!desc)
        return QString();
    else
        return desc->properties_[name];
}

void EC_Avatar::AdjustHeightOffset()
{
    Entity* entity = ParentEntity();
    AvatarDescAssetPtr desc = AvatarDesc();
    if ((!desc) || (!entity))
        return;
    EC_Mesh* mesh = entity->GetComponent<EC_Mesh>().get();
    if (!mesh)
        return;
    
    Ogre::Vector3 offset = Ogre::Vector3::ZERO;
    Ogre::Vector3 initial_base_pos = Ogre::Vector3::ZERO;

    if (desc->HasProperty("baseoffset"))
    {
        initial_base_pos = Ogre::StringConverter::parseVector3(desc->GetProperty("baseoffset").toStdString());
    }

    if (desc->HasProperty("basebone"))
    {
        Ogre::Bone* base_bone = GetAvatarBone(entity, desc->GetProperty("basebone").toStdString());
        if (base_bone)
        {
            Ogre::Vector3 temp;
            GetInitialDerivedBonePosition(base_bone, temp);
            initial_base_pos += temp;
            offset = initial_base_pos;

            // Additionally, if has the rootbone property, can do dynamic adjustment for sitting etc.
            // and adjust the name overlay height
            if (desc->HasProperty("rootbone"))
            {
                Ogre::Bone* root_bone = GetAvatarBone(entity, desc->GetProperty("rootbone").toStdString());
                if (root_bone)
                {
                    Ogre::Vector3 initial_root_pos;
                    Ogre::Vector3 current_root_pos = root_bone->_getDerivedPosition();
                    GetInitialDerivedBonePosition(root_bone, initial_root_pos);

                    float c = abs(current_root_pos.y / initial_root_pos.y);
                    if (c > 1.0) c = 1.0;
                    offset = initial_base_pos * c;

                }
            }
        }
    }

    mesh->SetAdjustPosition(float3(0.0f, -offset.y + FIXED_HEIGHT_OFFSET, 0.0f));
}

void EC_Avatar::SetupMeshAndMaterials()
{
    Entity* entity = ParentEntity();
    AvatarDescAssetPtr desc = AvatarDesc();
    if ((!desc) || (!entity))
        return;
    EC_Mesh* mesh = entity->GetComponent<EC_Mesh>().get();
    if (!mesh)
        return;

    // Mesh needs to be cloned if there are attachments which need to hide vertices
    bool need_mesh_clone = false;
    
    const std::vector<AvatarAttachment>& attachments = desc->attachments_;
    std::set<uint> vertices_to_hide;
    for (uint i = 0; i < attachments.size(); ++i)
    {
        if (attachments[i].vertices_to_hide_.size())
        {
            need_mesh_clone = true;
            for (uint j = 0; j < attachments[i].vertices_to_hide_.size(); ++j)
                vertices_to_hide.insert(attachments[i].vertices_to_hide_[j]);
        }
    }
    
    QString meshName = LookupAsset(desc->mesh_);
    
    if (desc->skeleton_.length())
    {
        QString skeletonName = LookupAsset(desc->skeleton_);
        mesh->SetMeshWithSkeleton(meshName.toStdString(), skeletonName.toStdString(), need_mesh_clone);
    }
    else
        mesh->SetMesh(meshName, need_mesh_clone);
    
    if (need_mesh_clone)
        HideVertices(mesh->GetEntity(), vertices_to_hide);
    
    for (uint i = 0; i < desc->materials_.size(); ++i)
        mesh->SetMaterial(i, LookupAsset(desc->materials_[i]), AttributeChange::Default);
    
    // Position approximately within the bounding box
    // Will be overridden by bone-based height adjust, if available
    mesh->SetAdjustPosition(float3(0.0f, FIXED_HEIGHT_OFFSET, 0.0f));
    mesh->castShadows.Set(true, AttributeChange::Default);
}

void EC_Avatar::SetupAttachments()
{
    Entity* entity = ParentEntity();
    AvatarDescAssetPtr desc = AvatarDesc();
    if ((!desc) || (!entity))
        return;
    EC_Mesh* mesh = entity->GetComponent<EC_Mesh>().get();
    if (!mesh)
        return;
    
    mesh->RemoveAllAttachments();
    
    const std::vector<AvatarAttachment>& attachments = desc->attachments_;
    for (uint i = 0; i < attachments.size(); ++i)
    {
        // Setup attachment meshes
        mesh->SetAttachmentMesh(i, LookupAsset(attachments[i].mesh_).toStdString(), attachments[i].bone_name_.toStdString(), attachments[i].link_skeleton_);
        // Setup attachment mesh materials
        for (uint j = 0; j < attachments[i].materials_.size(); ++j)
            mesh->SetAttachmentMaterial(i, j, LookupAsset(attachments[i].materials_[j]).toStdString());
        mesh->SetAttachmentPosition(i, attachments[i].transform_.position_);
        mesh->SetAttachmentOrientation(i, attachments[i].transform_.orientation_);
        mesh->SetAttachmentScale(i, attachments[i].transform_.scale_);
    }
}

void EC_Avatar::SetupMorphs()
{
    Entity* entity = ParentEntity();
    AvatarDescAssetPtr desc = AvatarDesc();
    if (!desc || !entity)
        return;
    EC_Mesh* mesh = entity->GetComponent<EC_Mesh>().get();
    if (!mesh)
        return;
    
    const std::vector<MorphModifier> morphs = desc->morphModifiers_;
    
    for (uint i = 0; i < morphs.size(); ++i)
    {
        mesh->SetMorphWeight(morphs[i].morph_name_, morphs[i].value_);
        // Also set position in attachment entities, if have the same morph
        for (uint j = 0; j < mesh->GetNumAttachments(); ++j)
            mesh->SetAttachmentMorphWeight(j, morphs[i].morph_name_, morphs[i].value_);
    }
}

void EC_Avatar::SetupBoneModifiers()
{
    Entity* entity = ParentEntity();
    AvatarDescAssetPtr desc = AvatarDesc();
    if (!desc || !entity)
        return;
    ResetBones(entity);
    
    const std::vector<BoneModifierSet>& bone_modifiers = desc->boneModifiers_;
    for (uint i = 0; i < bone_modifiers.size(); ++i)
    {
        for (uint j = 0; j < bone_modifiers[i].modifiers_.size(); ++j)
            ApplyBoneModifier(entity, bone_modifiers[i].modifiers_[j], bone_modifiers[i].value_);
    }
}

QString EC_Avatar::LookupAsset(const QString& ref)
{
    QString descName;
    AvatarDescAssetPtr desc = AvatarDesc();
    if (desc)
        descName = desc->Name();
    
    return framework->Asset()->ResolveAssetRef(descName, ref);
}

void ResetBones(Entity* entity)
{
    EC_Mesh* mesh = entity->GetComponent<EC_Mesh>().get();
    if (!mesh)
        return;
    
    Ogre::Entity* ogre_entity = mesh->GetEntity();
    if (!ogre_entity)
        return;
    // See that we actually have a skeleton
    Ogre::SkeletonInstance* skeleton = ogre_entity->getSkeleton();
    Ogre::Skeleton* orig_skeleton = ogre_entity->getMesh()->getSkeleton().get();
    if ((!skeleton) || (!orig_skeleton))
        return;
    
    if (skeleton->getNumBones() != orig_skeleton->getNumBones())
        return;
    
    for (uint i = 0; i < orig_skeleton->getNumBones(); ++i)
    {
        Ogre::Bone* bone = skeleton->getBone(i);
        Ogre::Bone* orig_bone = orig_skeleton->getBone(i);

        bone->setPosition(orig_bone->getInitialPosition());
        bone->setOrientation(orig_bone->getInitialOrientation());
        bone->setScale(orig_bone->getInitialScale());
        bone->setInitialState();
    }
}

void ApplyBoneModifier(Entity* entity, const BoneModifier& modifier, float value)
{
    EC_Mesh* mesh = entity->GetComponent<EC_Mesh>().get();
    if (!mesh)
        return;
    Ogre::Entity* ogre_entity = mesh->GetEntity();
    if (!ogre_entity)
        return;
    // See that we actually have a skeleton
    Ogre::SkeletonInstance* skeleton = ogre_entity->getSkeleton();
    Ogre::Skeleton* orig_skeleton = ogre_entity->getMesh()->getSkeleton().get();
    if ((!skeleton) || (!orig_skeleton))
        return;
    
    if (!skeleton->hasBone(modifier.bone_name_.toStdString()) || !orig_skeleton->hasBone(modifier.bone_name_.toStdString()))
        return; // Bone not found, nothing to do
        
    Ogre::Bone* bone = skeleton->getBone(modifier.bone_name_.toStdString());
    Ogre::Bone* orig_bone = orig_skeleton->getBone(modifier.bone_name_.toStdString());

    value = Clamp(value, 0.0f, 1.0f);

    // Rotation
    {
        Ogre::Matrix3 rot_start, rot_end, rot_base, rot_orig;
        Ogre::Radian sx, sy, sz;
        Ogre::Radian ex, ey, ez;
        Ogre::Radian bx, by, bz;
        Ogre::Radian rx, ry, rz;
        rot_start = float3x3(modifier.start_.orientation_);
        rot_end = float3x3(modifier.end_.orientation_);
//        OgreRenderer::ToOgreQuaternion(modifier.start_.orientation_).ToRotationMatrix(rot_start);
 //       OgreRenderer::ToOgreQuaternion(modifier.end_.orientation_).ToRotationMatrix(rot_end);
        bone->getInitialOrientation().ToRotationMatrix(rot_orig);
        rot_start.ToEulerAnglesXYZ(sx, sy, sz);
        rot_end.ToEulerAnglesXYZ(ex, ey, ez);
        rot_orig.ToEulerAnglesXYZ(rx, ry, rz);
        
        switch(modifier.orientation_mode_)
        {
        case BoneModifier::Absolute:
            bx = by = bz = 0;
            break;
        case BoneModifier::Relative:
            orig_bone->getInitialOrientation().ToRotationMatrix(rot_base);
            rot_base.ToEulerAnglesXYZ(bx, by, bz);
            break;
        case BoneModifier::Cumulative:
            bone->getInitialOrientation().ToRotationMatrix(rot_base);
            rot_base.ToEulerAnglesXYZ(bx, by, bz);
            break;
        }
        
        if (sx != Ogre::Radian(0) || ex != Ogre::Radian(0))
            rx = bx + sx * (1.0 - value) + ex * (value);
        if (sy != Ogre::Radian(0) || ey != Ogre::Radian(0))
            ry = by + sy * (1.0 - value) + ey * (value);
        if (sz != Ogre::Radian(0) || ez != Ogre::Radian(0))
            rz = bz + sz * (1.0 - value) + ez * (value);
        
        Ogre::Matrix3 rot_new;
        rot_new.FromEulerAnglesXYZ(rx, ry, rz);
        Ogre::Quaternion q_new(rot_new);
        bone->setOrientation(Ogre::Quaternion(rot_new));
    }
    
    // Translation
    {
        float sx = modifier.start_.position_.x;
        float sy = modifier.start_.position_.y;
        float sz = modifier.start_.position_.z;
        float ex = modifier.end_.position_.x;
        float ey = modifier.end_.position_.y;
        float ez = modifier.end_.position_.z;
        
        Ogre::Vector3 base(0,0,0);
        Ogre::Vector3 trans = bone->getInitialPosition();
        switch(modifier.position_mode_)
        {
        case BoneModifier::Relative:
            base = orig_bone->getInitialPosition();
            break;
        case BoneModifier::Absolute:
        case BoneModifier::Cumulative:
            break;
        }
        
        if (sx != 0 || ex != 0)
            trans.x = base.x + sx * (1.0 - value) + ex * value;
        if (sy != 0 || ey != 0)
            trans.y = base.y + sy * (1.0 - value) + ey * value;
        if (sz != 0 || ez != 0)
            trans.z = base.z + sz * (1.0 - value) + ez * value;
        
        bone->setPosition(trans);
    }
    
    // Scale
    {
        Ogre::Vector3 scale = bone->getInitialScale();
        float sx = modifier.start_.scale_.x;
        float sy = modifier.start_.scale_.y;
        float sz = modifier.start_.scale_.z;
        float ex = modifier.end_.scale_.x;
        float ey = modifier.end_.scale_.y;
        float ez = modifier.end_.scale_.z;
        
        if (sx != 1 || ex != 1)
            scale.x = sx * (1.0 - value) + ex * value;
        if (sy != 1 || ey != 1)
            scale.y = sy * (1.0 - value) + ey * value;
        if (sz != 1 || ez != 1)
            scale.z = sz * (1.0 - value) + ez * value;
        
        bone->setScale(scale);
    }
    
    bone->setInitialState();
}

void GetInitialDerivedBonePosition(Ogre::Node* bone, Ogre::Vector3& position)
{
    // Hacky and slow way to derive the initial position of the base bone. Do not use current position
    // because animations change it
    position = bone->getInitialPosition();
    Ogre::Vector3 scale = bone->getInitialScale();
    Ogre::Quaternion orient = bone->getInitialOrientation();

    while (bone->getParent())
    {
       Ogre::Node* parent = bone->getParent();

       if (bone->getInheritOrientation())
       {
          orient = parent->getInitialOrientation() * orient;
       }
       if (bone->getInheritScale())
       {
          scale = parent->getInitialScale() * scale;
       }

       position = parent->getInitialOrientation() * (parent->getInitialScale() * position);
       position += parent->getInitialPosition();

       bone = parent;
    }
}

Ogre::Bone* GetAvatarBone(Entity* entity, const std::string& bone_name)
{
    if (!entity)
        return 0;
    EC_Mesh* mesh = entity->GetComponent<EC_Mesh>().get();
    if (!mesh)
        return 0;
    
    Ogre::Entity* ogre_entity = mesh->GetEntity();
    if (!ogre_entity)
        return 0;
    Ogre::SkeletonInstance* skeleton = ogre_entity->getSkeleton();
    if (!skeleton)
        return 0;
    if (!skeleton->hasBone(bone_name))
        return 0;
    return skeleton->getBone(bone_name);
}

void HideVertices(Ogre::Entity* entity, std::set<uint> vertices_to_hide)
{
    if (!entity)
        return;
    Ogre::MeshPtr mesh = entity->getMesh();
    if (mesh.isNull())
        return;
    if (!mesh->getNumSubMeshes())
        return;
    for (uint m = 0; m < 1; ++m)
    {
        // Under current system, it seems vertices should only be hidden from first submesh
        Ogre::SubMesh *submesh = mesh->getSubMesh(m);
        if (!submesh)
            return;
        Ogre::IndexData *data = submesh->indexData;
        if (!data)
            return;
        Ogre::HardwareIndexBufferSharedPtr ibuf = data->indexBuffer;
        if (ibuf.isNull())
            return;

        unsigned long* lIdx = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
        unsigned short* pIdx = reinterpret_cast<unsigned short*>(lIdx);
        bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

        for (uint n = 0; n < data->indexCount; n += 3)
        {
            if (!use32bitindexes)
            {
                if (vertices_to_hide.find(pIdx[n]) != vertices_to_hide.end() ||
                    vertices_to_hide.find(pIdx[n+1]) != vertices_to_hide.end() ||
                    vertices_to_hide.find(pIdx[n+2]) != vertices_to_hide.end())
                {
                    if (n + 3 < data->indexCount)
                    {
                        for (size_t i = n ; i<data->indexCount-3 ; ++i)
                        {
                            pIdx[i] = pIdx[i+3];
                        }
                    }
                    data->indexCount -= 3;
                    n -= 3;
                }
            }
            else
            {
                if (vertices_to_hide.find(lIdx[n]) != vertices_to_hide.end() ||
                    vertices_to_hide.find(lIdx[n+1]) != vertices_to_hide.end() ||
                    vertices_to_hide.find(lIdx[n+2]) != vertices_to_hide.end())
                {
                    if (n + 3 < data->indexCount)
                    {
                        for (size_t i = n ; i<data->indexCount-3 ; ++i)
                        {
                            lIdx[i] = lIdx[i+3];
                        }
                    }
                    data->indexCount -= 3;
                    n -= 3;
                }
            }
        }
        ibuf->unlock();
    }
}

