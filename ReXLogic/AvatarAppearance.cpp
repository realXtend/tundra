// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarAppearance.h"
#include "AvatarExporter.h"
#include "LegacyAvatarSerializer.h"
#include "RexLogicModule.h"
#include "SceneManager.h"
#include "EC_AvatarAppearance.h"
#include "EC_OpenSimAvatar.h"
#include "EC_OgreMesh.h"
#include "EC_OgreMovableTextOverlay.h"
#include "OgreMaterialResource.h"
#include "OgreMaterialUtils.h"
#include "Renderer.h"
#include "OgreConversionUtils.h"
#include "OgreMeshResource.h"
#include "OgreSkeletonResource.h"
#include "OgreMaterialResource.h"
#include "OgreImageTextureResource.h"
#include "HttpTask.h"
#include "HttpUtilities.h"
#include "LLSDUtilities.h"

#include "Poco/URI.h"

#include <QDomDocument>
#include <QFile>

static const Core::Real FIXED_HEIGHT_OFFSET = -0.87f;
static const Core::Real OVERLAY_HEIGHT_MULTIPLIER = 1.5f;
static const Core::uint XMLRPC_ASSET_HASH_LENGTH = 28;

namespace RexLogic
{
    AvatarAppearance::AvatarAppearance(RexLogicModule *rexlogicmodule) :
        rexlogicmodule_(rexlogicmodule)
    {
        std::string default_avatar_path = rexlogicmodule_->GetFramework()->GetDefaultConfig().DeclareSetting("RexAvatar", "default_avatar_file", std::string("./data/default_avatar.xml"));
        
        ReadDefaultAppearance(default_avatar_path);
    }

    AvatarAppearance::~AvatarAppearance()
    {
    }

    void AvatarAppearance::Update(Core::f64 frametime)
    {
        ProcessAppearanceDownloads();
        ProcessAvatarExport();
    }
    
    void AvatarAppearance::DownloadAppearance(Scene::EntityPtr entity)
    {
        if (!entity)
            return;
        
        Foundation::ComponentPtr avatarptr = entity->GetComponent(EC_OpenSimAvatar::NameStatic());
        if (!avatarptr)
            return;
        EC_OpenSimAvatar& avatar = *checked_static_cast<EC_OpenSimAvatar*>(avatarptr.get());
        
        std::string appearance_address = avatar.GetAppearanceAddress();
        if (appearance_address.empty())
            return;
        
        // See if download already exists for this avatar
        if (appearance_downloaders_.find(entity->GetId()) != appearance_downloaders_.end())
            return;
        
        // Setup new http task running in the background
        HttpUtilities::HttpTaskPtr new_download(new HttpUtilities::HttpTask());
        HttpUtilities::HttpTaskRequestPtr new_request(new HttpUtilities::HttpTaskRequest());
        new_request->url_ = appearance_address;
        appearance_downloaders_[entity->GetId()] = new_download;
        new_download->AddRequest<HttpUtilities::HttpTaskRequest>(new_request);
    }
    
    void AvatarAppearance::ReadDefaultAppearance(const std::string& filename)
    {
        default_appearance_ = boost::shared_ptr<QDomDocument>(new QDomDocument("defaultappearance"));
        
        QFile file(filename.c_str());
        if (!file.open(QIODevice::ReadOnly))
        {
            RexLogicModule::LogError("Could not open avatar default appearance file " + filename);
            return;
        }
        
        if (!default_appearance_->setContent(&file))
        {
            file.close();
            RexLogicModule::LogError("Could not load avatar default appearance file " + filename);
            return;
        }
        file.close();
    }
    
    void AvatarAppearance::SetupDefaultAppearance(Scene::EntityPtr entity)
    {
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        if (!appearanceptr)
            return;
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(appearanceptr.get());
        
        // Deserialize appearance from the document into the EC
        LegacyAvatarSerializer::ReadAvatarAppearance(appearance, *default_appearance_);
        
        SetupAppearance(entity);
    }
    
    void AvatarAppearance::SetupAppearance(Scene::EntityPtr entity)
    {
        if (!entity)
            return;
        
        Foundation::ComponentPtr meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        if (!meshptr || !appearanceptr)
            return;
        
        // Fix up material/texture references
        FixupResources(entity);
        
        // Setup appearance
        SetupMeshAndMaterials(entity);
        SetupDynamicAppearance(entity);
        SetupAttachments(entity);
    }
    
    void AvatarAppearance::SetupDynamicAppearance(Scene::EntityPtr entity)
    {
        if (!entity)
            return;
        
        Foundation::ComponentPtr meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        
        if (!meshptr || !appearanceptr)
            return;

        SetupMorphs(entity);
        SetupBoneModifiers(entity);
        AdjustHeightOffset(entity);
    }
    
    void AvatarAppearance::AdjustHeightOffset(Scene::EntityPtr entity)
    {
        if (!entity)
            return;
        
        Foundation::ComponentPtr meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        
        if (!meshptr || !appearanceptr)
            return;
        
        OgreRenderer::EC_OgreMesh &mesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(meshptr.get());
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(appearanceptr.get());
        
        Ogre::Vector3 offset = Ogre::Vector3::ZERO;
        Ogre::Vector3 initial_base_pos = Ogre::Vector3::ZERO;

        if (appearance.HasProperty("baseoffset"))
        {
            initial_base_pos = Ogre::StringConverter::parseVector3(appearance.GetProperty("baseoffset"));
        }

        if (appearance.HasProperty("basebone"))
        {
            Ogre::Bone* base_bone = GetAvatarBone(entity, appearance.GetProperty("basebone"));
            if (base_bone)
            {
                Ogre::Vector3 temp;
                GetInitialDerivedBonePosition(base_bone, temp);
                initial_base_pos += temp;
                offset = initial_base_pos;

                // Additionally, if has the rootbone property, can do dynamic adjustment for sitting etc.
                // and adjust the name overlay height
                if (appearance.HasProperty("rootbone"))
                {
                    Ogre::Bone* root_bone = GetAvatarBone(entity, appearance.GetProperty("rootbone"));
                    if (root_bone)
                    {
                        Ogre::Vector3 initial_root_pos;
                        Ogre::Vector3 current_root_pos = root_bone->_getDerivedPosition();
                        GetInitialDerivedBonePosition(root_bone, initial_root_pos);
                        
                        float c = abs(current_root_pos.y / initial_root_pos.y);
                        if (c > 1.0) c = 1.0;
                        offset = initial_base_pos * c;

                        // Set name overlay height according to base + root distance.
                        Foundation::ComponentPtr overlay = entity->GetComponent(OgreRenderer::EC_OgreMovableTextOverlay::NameStatic());
                        if (overlay)
                        {
                            OgreRenderer::EC_OgreMovableTextOverlay &name_overlay = *checked_static_cast<OgreRenderer::EC_OgreMovableTextOverlay*>(overlay.get());
                            name_overlay.SetOffset(Core::Vector3df(0, 0, abs(initial_base_pos.y - initial_root_pos.y) * OVERLAY_HEIGHT_MULTIPLIER));
                        }
                    }
                }
            }
        }

        mesh.SetAdjustPosition(Core::Vector3df(0.0f, 0.0f, -offset.y + FIXED_HEIGHT_OFFSET));
    }
    
    void AvatarAppearance::SetupMeshAndMaterials(Scene::EntityPtr entity)
    {
        OgreRenderer::EC_OgreMesh &mesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic()).get());
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(entity->GetComponent(EC_AvatarAppearance::NameStatic()).get());
        
        // Mesh needs to be cloned if there are attachments which need to hide vertices
        bool need_mesh_clone = true;
        
        const AvatarAttachmentVector& attachments = appearance.GetAttachments();
        std::set<Core::uint> vertices_to_hide;
        for (Core::uint i = 0; i < attachments.size(); ++i)
        {
            if (attachments[i].vertices_to_hide_.size())
            {
                need_mesh_clone = true;
                for (Core::uint j = 0; j < attachments[i].vertices_to_hide_.size(); ++j)
                    vertices_to_hide.insert(attachments[i].vertices_to_hide_[j]);
            }
        }
        
        if (appearance.GetSkeleton().resource_)
            mesh.SetMeshWithSkeleton(appearance.GetMesh().GetLocalOrResourceName(), appearance.GetSkeleton().GetLocalOrResourceName(), entity.get(), need_mesh_clone);
        else
            mesh.SetMesh(appearance.GetMesh().GetLocalOrResourceName(), entity.get(), need_mesh_clone);
            
        if (need_mesh_clone)
            HideVertices(mesh.GetEntity(), vertices_to_hide);
        
        AvatarMaterialVector materials = appearance.GetMaterials();
        for (Core::uint i = 0; i < materials.size(); ++i)
        {
            mesh.SetMaterial(i, materials[i].asset_.GetLocalOrResourceName());
        }
        
        // Store the modified materials vector (with created temp resources) to the EC
        appearance.SetMaterials(materials);
        
        // Set adjustment orientation for mesh (Ogre meshes usually have Y-axis as vertical)
        Core::Quaternion adjust(Core::PI/2, 0, -Core::PI/2);
        mesh.SetAdjustOrientation(adjust);
        // Position approximately within the bounding box
        // Will be overridden by bone-based height adjust, if available
        mesh.SetAdjustPosition(Core::Vector3df(0.0f, 0.0f, FIXED_HEIGHT_OFFSET));
        mesh.SetCastShadows(true);
    }
    
    void AvatarAppearance::SetupAttachments(Scene::EntityPtr entity)
    {
        OgreRenderer::EC_OgreMesh &mesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic()).get());
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(entity->GetComponent(EC_AvatarAppearance::NameStatic()).get());
        
        mesh.RemoveAllAttachments();
        
        const AvatarAttachmentVector& attachments = appearance.GetAttachments();
        
        for (Core::uint i = 0; i < attachments.size(); ++i)
        {
            // Setup attachment meshes
            mesh.SetAttachmentMesh(i, attachments[i].mesh_.GetLocalOrResourceName(), attachments[i].bone_name_, attachments[i].link_skeleton_);
            // Setup attachment mesh materials
            for (Core::uint j = 0; j < attachments[i].materials_.size(); ++j)
            {
                mesh.SetAttachmentMaterial(i, j, attachments[i].materials_[j].asset_.GetLocalOrResourceName());
            }
            mesh.SetAttachmentPosition(i, attachments[i].transform_.position_);
            mesh.SetAttachmentOrientation(i, attachments[i].transform_.orientation_);
            mesh.SetAttachmentScale(i, attachments[i].transform_.scale_);
        }
    }
    
    void AvatarAppearance::SetupMorphs(Scene::EntityPtr entity)
    {
        OgreRenderer::EC_OgreMesh &mesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic()).get());
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(entity->GetComponent(EC_AvatarAppearance::NameStatic()).get());
        
        Ogre::Entity* ogre_entity = mesh.GetEntity();
        if (!ogre_entity)
            return;
        Ogre::AnimationStateSet* anims = ogre_entity->getAllAnimationStates();
        if (!anims)
            return;
            
        const MorphModifierVector& morphs = appearance.GetMorphModifiers();
        for (Core::uint i = 0; i < morphs.size(); ++i)
        {
            if (anims->hasAnimationState(morphs[i].morph_name_))
            {
                float timePos = morphs[i].value_;
                if (timePos < 0.0f)
                    timePos = 0.0f;
                // Clamp very close to 1.0, but do not actually go to 1.0 or the morph animation will wrap
                if (timePos > 0.99995f)
                    timePos = 0.99995f;
                
                Ogre::AnimationState* anim = anims->getAnimationState(morphs[i].morph_name_);
                anim->setTimePosition(timePos);
                anim->setEnabled(timePos > 0.0f);
                
                // Also set position in attachment entities, if have the same morph
                for (Core::uint j = 0; j < mesh.GetNumAttachments(); ++j)
                {
                    Ogre::Entity* attachment = mesh.GetAttachmentEntity(j);
                    if (!attachment)
                        continue;
                    Ogre::AnimationStateSet* attachment_anims = attachment->getAllAnimationStates();
                    if (!attachment_anims)
                        continue;
                    if (!attachment_anims->hasAnimationState(morphs[i].morph_name_))
                        continue;
                    Ogre::AnimationState* attachment_anim = attachment_anims->getAnimationState(morphs[i].morph_name_);
                    attachment_anim->setTimePosition(timePos);
                    attachment_anim->setEnabled(timePos > 0.0f);
                }
            }
        }
    }
    
    void AvatarAppearance::SetupBoneModifiers(Scene::EntityPtr entity)
    {
        OgreRenderer::EC_OgreMesh &mesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic()).get());
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(entity->GetComponent(EC_AvatarAppearance::NameStatic()).get());
        
        ResetBones(entity);
        
        const BoneModifierSetVector& bone_modifiers = appearance.GetBoneModifiers();
        for (Core::uint i = 0; i < bone_modifiers.size(); ++i)
        {
            for (Core::uint j = 0; j < bone_modifiers[i].modifiers_.size(); ++j)
            {
                ApplyBoneModifier(entity, bone_modifiers[i].modifiers_[j], bone_modifiers[i].value_);
            }
        }
    }
    
    void AvatarAppearance::ResetBones(Scene::EntityPtr entity)
    {
        OgreRenderer::EC_OgreMesh &mesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic()).get());
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(entity->GetComponent(EC_AvatarAppearance::NameStatic()).get());
        
        Ogre::Entity* ogre_entity = mesh.GetEntity();
        if (!ogre_entity)
            return;
        // See that we actually have a skeleton
        Ogre::SkeletonInstance* skeleton = ogre_entity->getSkeleton();
        Ogre::Skeleton* orig_skeleton = ogre_entity->getMesh()->getSkeleton().get();
        if ((!skeleton) || (!orig_skeleton))
            return;
        
        if (skeleton->getNumBones() != orig_skeleton->getNumBones())
            return;
        
        for (Core::uint i = 0; i < orig_skeleton->getNumBones(); ++i)
        {
            Ogre::Bone* bone = skeleton->getBone(i);
            Ogre::Bone* orig_bone = orig_skeleton->getBone(i);

            bone->setPosition(orig_bone->getInitialPosition());
            bone->setOrientation(orig_bone->getInitialOrientation());
            bone->setScale(orig_bone->getInitialScale());
            bone->setInitialState();
        }
    }
    
    void AvatarAppearance::ApplyBoneModifier(Scene::EntityPtr entity, const BoneModifier& modifier, Core::Real value)
    {
        OgreRenderer::EC_OgreMesh &mesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic()).get());
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(entity->GetComponent(EC_AvatarAppearance::NameStatic()).get());
        
        Ogre::Entity* ogre_entity = mesh.GetEntity();
        if (!ogre_entity)
            return;
        // See that we actually have a skeleton
        Ogre::SkeletonInstance* skeleton = ogre_entity->getSkeleton();
        Ogre::Skeleton* orig_skeleton = ogre_entity->getMesh()->getSkeleton().get();
        if ((!skeleton) || (!orig_skeleton))
            return;
        
        if ((!skeleton->hasBone(modifier.bone_name_)) || (!orig_skeleton->hasBone(modifier.bone_name_)))
            return; // Bone not found, nothing to do
            
        Ogre::Bone* bone = skeleton->getBone(modifier.bone_name_);
        Ogre::Bone* orig_bone = orig_skeleton->getBone(modifier.bone_name_);
        
        if (value < 0.0f)
            value = 0.0f;
        if (value > 1.0f)
            value = 1.0f;
        
        // Rotation
        {
            Ogre::Matrix3 rot_start, rot_end, rot_base, rot_orig;
            Ogre::Radian sx, sy, sz;
            Ogre::Radian ex, ey, ez;
            Ogre::Radian bx, by, bz;
            Ogre::Radian rx, ry, rz;
            OgreRenderer::ToOgreQuaternion(modifier.start_.orientation_).ToRotationMatrix(rot_start);
            OgreRenderer::ToOgreQuaternion(modifier.end_.orientation_).ToRotationMatrix(rot_end);
            bone->getInitialOrientation().ToRotationMatrix(rot_orig);
            rot_start.ToEulerAnglesXYZ(sx, sy, sz);
            rot_end.ToEulerAnglesXYZ(ex, ey, ez);
            rot_orig.ToEulerAnglesXYZ(rx, ry, rz);
            
            switch (modifier.orientation_mode_)
            {
            case Absolute:
                bx = 0;
                by = 0;
                bz = 0;
                break;
                
            case Relative:
                orig_bone->getInitialOrientation().ToRotationMatrix(rot_base);
                rot_base.ToEulerAnglesXYZ(bx, by, bz);
                break;
                
            case Cumulative:
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
            Core::Real sx = modifier.start_.position_.x;
            Core::Real sy = modifier.start_.position_.y;
            Core::Real sz = modifier.start_.position_.z;
            Core::Real ex = modifier.end_.position_.x;
            Core::Real ey = modifier.end_.position_.y;
            Core::Real ez = modifier.end_.position_.z;
            
            Ogre::Vector3 trans, base;
            trans = bone->getInitialPosition();
            switch (modifier.position_mode_)
            {
            case Absolute:
                base = Ogre::Vector3(0,0,0);
                break;
            case Relative:
                base = orig_bone->getInitialPosition();
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
            Core::Real sx = modifier.start_.scale_.x;
            Core::Real sy = modifier.start_.scale_.y;
            Core::Real sz = modifier.start_.scale_.z;
            Core::Real ex = modifier.end_.scale_.x;
            Core::Real ey = modifier.end_.scale_.y;
            Core::Real ez = modifier.end_.scale_.z;
            
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

    void AvatarAppearance::GetInitialDerivedBonePosition(Ogre::Node* bone, Ogre::Vector3& position)
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
    
    Ogre::Bone* AvatarAppearance::GetAvatarBone(Scene::EntityPtr entity, const std::string& bone_name)
    {
        Foundation::ComponentPtr meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
        if (!meshptr)
            return 0;
        
        OgreRenderer::EC_OgreMesh &mesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(meshptr.get());
        Ogre::Entity* ogre_entity = mesh.GetEntity();
        if (!ogre_entity)
            return 0;
        Ogre::SkeletonInstance* skeleton = ogre_entity->getSkeleton();
        if (!skeleton)
            return 0;
        if (!skeleton->hasBone(bone_name))
            return 0;
        return skeleton->getBone(bone_name);
    }
    
    void AvatarAppearance::HideVertices(Ogre::Entity* entity, std::set<Core::uint> vertices_to_hide)
    {
        if (!entity)
            return;
        Ogre::MeshPtr mesh = entity->getMesh();
        if (mesh.isNull())
            return;
        
        for (Core::uint m = 0; m < 1; ++m)
        {
            // Under current system, it seems vertices should only be hidden from first submesh
            Ogre::SubMesh *submesh = mesh->getSubMesh(m);
            Ogre::IndexData *data = submesh->indexData;
            Ogre::HardwareIndexBufferSharedPtr ibuf = data->indexBuffer;

            unsigned long* lIdx = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
            unsigned short* pIdx = reinterpret_cast<unsigned short*>(lIdx);
            bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

            for (Core::uint n = 0; n < data->indexCount; n += 3)
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
    
    void AvatarAppearance::ProcessAppearanceDownloads()
    {
        // Check download results
        std::map<Core::entity_id_t, HttpUtilities::HttpTaskPtr>::iterator i = appearance_downloaders_.begin();
        while (i != appearance_downloaders_.end())
        {
            bool done = false;
            
            if (i->second)
            {
                HttpUtilities::HttpTaskResultPtr result = i->second->GetResult<HttpUtilities::HttpTaskResult>();
                if (result)
                {
                    if (result->GetSuccess())
                    {
                        Scene::EntityPtr entity = rexlogicmodule_->GetAvatarEntity(i->first);
                        if (entity)
                            ProcessAppearanceDownload(entity, result->data_);
                    }
                    else
                        RexLogicModule::LogInfo("Error downloading avatar appearance for avatar " + Core::ToString<int>(i->first) + ": " + result->reason_);
                    
                    done = true;
                }
            }
            
            if (!done)
                ++i;
            else
                appearance_downloaders_.erase(i++);
        }
    }
    
    void AvatarAppearance::ProcessAppearanceDownload(Scene::EntityPtr entity, const std::vector<Core::u8>& data)
    {
        if (!entity)
            return;
        Foundation::ComponentPtr avatarptr = entity->GetComponent(EC_OpenSimAvatar::NameStatic());
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        if (!avatarptr || !appearanceptr)
            return;
        EC_OpenSimAvatar& avatar = *checked_static_cast<EC_OpenSimAvatar*>(avatarptr.get());
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(appearanceptr.get());
        
        std::string data_str((const char*)&data[0], data.size());
        std::map<std::string, std::string> contents = RexTypes::ParseLLSDMap(data_str);
        
        // Get the avatar appearance description ("generic xml")
        std::map<std::string, std::string>::iterator i = contents.find("generic xml");
        if (i == contents.end())
        {
            // If not found, use default appearance
            // (at this point, it's nice to just have *some* appearance change, for example
            // changing back to default human from fish in the fishworld, if no avatar stored)
            RexLogicModule::LogDebug("No generic xml found, setting default appearance");
            SetupDefaultAppearance(entity);
            return;
        }
        
        std::string& appearance_str = i->second;

        // Return to original format by substituting to < >
        ReplaceSubstring(appearance_str, "&lt;", "<");
        ReplaceSubstring(appearance_str, "&gt;", ">");
        
        QDomDocument avatar_doc("Avatar");
        avatar_doc.setContent(QString::fromStdString(appearance_str));
        
        std::map<std::string, std::string>::iterator j = contents.begin();
        // Build mapping of human-readable asset names to id's
        std::string host = HttpUtilities::GetHostFromUrl(avatar.GetAppearanceAddress());
        AvatarAssetMap assets;
        while (j != contents.end())
        {
            // Don't add the name field or the avatar description
            if ((j->first != "generic xml") && (j->first != "name"))
            {
                assets[j->first] = host + "/item/" + j->second;
            }
            ++j;
        }
        
        // Deserialize appearance from the document into the EC
        if (!LegacyAvatarSerializer::ReadAvatarAppearance(appearance, avatar_doc))
        {
            SetupDefaultAppearance(entity);
            return;
        }
        
        appearance.SetAssetMap(assets);
        
        // Request needed avatar resources
        boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->
            GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        Core::RequestTagVector tags;
        AvatarAssetMap::iterator k = assets.begin();
        Core::uint pending_requests = 0;
        
        while (k != assets.end())
        {
            std::string resource_id = k->second;
            Core::request_tag_t tag = renderer->RequestResource(resource_id, GetResourceTypeFromName(k->first));
            if (tag)
            {
                tags.push_back(tag);
                avatar_resource_tags_[tag] = entity->GetId();
                pending_requests++;
            }
            ++k;
        }
        avatar_pending_requests_[entity->GetId()] = pending_requests;
        
        // In the unlikely case of no requests at all, rebuild avatar now
        if (!pending_requests)
            SetupAppearance(entity);
    }
    
    bool AvatarAppearance::HandleResourceEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (!event_id == Resource::Events::RESOURCE_READY)
            return false;

        Resource::Events::ResourceReady* event_data = checked_static_cast<Resource::Events::ResourceReady*>(data);
        if (!event_data)
            return false;
        std::map<Core::request_tag_t, Core::entity_id_t>::iterator i = avatar_resource_tags_.find(event_data->tag_);
        if (i == avatar_resource_tags_.end())
            return false;
        Core::entity_id_t id = i->second;
        avatar_resource_tags_.erase(i);
        Scene::EntityPtr entity = rexlogicmodule_->GetAvatarEntity(id);
        if (!entity)
            return true;
        
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        if (!appearanceptr)
            return true;
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(appearanceptr.get());
        
        if (avatar_pending_requests_[id])
        {
            avatar_pending_requests_[id]--;
            // If was the last request, rebuild avatar
            if (avatar_pending_requests_[id] == 0)
                SetupAppearance(entity);
        }
        
        return true;
    }
    
    const std::string& AvatarAppearance::GetResourceTypeFromName(const std::string& name)
    {
        if (name.find(".mesh") != std::string::npos)
            return OgreRenderer::OgreMeshResource::GetTypeStatic();
        if (name.find(".skeleton") != std::string::npos)
            return OgreRenderer::OgreSkeletonResource::GetTypeStatic();
        if (name.find(".material") != std::string::npos)
            return OgreRenderer::OgreMaterialResource::GetTypeStatic();
        
        // If not any of these, assume a texture image (.png, .jpg etc.)
        return OgreRenderer::OgreImageTextureResource::GetTypeStatic();
    }
    
    void AvatarAppearance::FixupResources(Scene::EntityPtr entity)
    {
        if (!entity)
            return;
            
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        if (!appearanceptr)
            return;
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(appearanceptr.get());
        const AvatarAssetMap& asset_map = appearance.GetAssetMap();
        
        // Get mesh, skeleton, materials & attachments
        AvatarAsset mesh = appearance.GetMesh();
        AvatarAsset skeleton = appearance.GetSkeleton();
        AvatarMaterialVector materials = appearance.GetMaterials();
        AvatarAttachmentVector attachments = appearance.GetAttachments();
        
        // Fix mesh & skeleton
        FixupResource(mesh, asset_map, OgreRenderer::OgreMeshResource::GetTypeStatic());
        FixupResource(skeleton, asset_map, OgreRenderer::OgreSkeletonResource::GetTypeStatic());
        
        // Fix avatar mesh materials
        for (Core::uint i = 0; i < materials.size(); ++i)
            FixupMaterial(materials[i], asset_map);
        // Fix attachment meshes & their materials
        for (Core::uint i = 0; i < attachments.size(); ++i)
        {
            FixupResource(attachments[i].mesh_, asset_map, OgreRenderer::OgreMeshResource::GetTypeStatic());
            
            if (attachments[i].mesh_.resource_)
            {
                OgreRenderer::OgreMeshResource* mesh_res = dynamic_cast<OgreRenderer::OgreMeshResource*>(attachments[i].mesh_.resource_.get());
                if (mesh_res)
                {
                    const Core::StringVector& attach_matnames = mesh_res->GetOriginalMaterialNames();
                    attachments[i].materials_.clear();
                    AvatarMaterial attach_newmat;
                    for (Core::uint j = 0; j < attach_matnames.size(); ++j)
                    {
                        attach_newmat.asset_.name_ = attach_matnames[j];
                        FixupMaterial(attach_newmat, asset_map);
                        attachments[i].materials_.push_back(attach_newmat);
                    }
                }
            }
        }
        
        // Set modified mesh, skeleton, materials & attachments
        appearance.SetMesh(mesh);
        appearance.SetSkeleton(skeleton);
        appearance.SetMaterials(materials);
        appearance.SetAttachments(attachments);
    }
    
    void AvatarAppearance::FixupResource(AvatarAsset& asset, const AvatarAssetMap& asset_map, const std::string& resource_type)
    {
        boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->
            GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (!renderer)
            return;
        
        if (!asset.resource_)
        {
            AvatarAssetMap::const_iterator i = asset_map.find(asset.name_);
            if (i != asset_map.end())
            {
                asset.resource_id_ = i->second;
                asset.resource_ = renderer->GetResource(asset.resource_id_, resource_type);
            }
        }
    }
    
    void AvatarAppearance::FixupMaterial(AvatarMaterial& mat, const AvatarAssetMap& asset_map)
    {
        boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->
            GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (!renderer)
            return;
            
        // First thing to do: append .material to name if it doesn't exist, because our asset map is based on full asset name,
        // and also the storage kind of does not like if it cannot identify the asset
        std::string fixed_mat_name = mat.asset_.name_;
        if (fixed_mat_name.find(".material") == std::string::npos)
            fixed_mat_name.append(".material");
        
        // First find resource for the material itself
        if (!mat.asset_.resource_)
        {
            AvatarAssetMap::const_iterator i = asset_map.find(fixed_mat_name);
            if (i != asset_map.end())
            {
                mat.asset_.resource_id_ = i->second;
                mat.asset_.resource_ = renderer->GetResource(mat.asset_.resource_id_, OgreRenderer::OgreMaterialResource::GetTypeStatic());
            }
        }
        // If couldn't be found, abort
        if (!mat.asset_.resource_)
            return;
        
        OgreRenderer::OgreMaterialResource* mat_res = dynamic_cast<OgreRenderer::OgreMaterialResource*>(mat.asset_.resource_.get());
        if (!mat_res)
            return;
            
        const Core::StringVector& orig_textures = mat_res->GetOriginalTextureNames();
        if (mat.textures_.size() < orig_textures.size())
            mat.textures_.resize(orig_textures.size());
        for (Core::uint i = 0; i < mat.textures_.size(); ++i)
        {
            if (!mat.textures_[i].resource_)
            {
                // Fill in name if not specified
                if ((mat.textures_[i].name_.empty()) && (i < orig_textures.size()))
                    mat.textures_[i].name_ = orig_textures[i];
                
                AvatarAssetMap::const_iterator j = asset_map.find(mat.textures_[i].name_);
                if (j != asset_map.end())
                {
                    mat.textures_[i].resource_id_ = j->second;
                    mat.textures_[i].resource_ = renderer->GetResource(mat.textures_[i].resource_id_, OgreRenderer::OgreImageTextureResource::GetTypeStatic());
                }
            }
            // If we found the texture, modify the material to use it. Note: if there are any resource clashes, should be trivial to clone the material here to allow unique overrides
            if (mat.textures_[i].resource_)
            {
                Ogre::MaterialPtr ogremat = mat_res->GetMaterial();
                OgreRenderer::ReplaceTextureOnMaterial(ogremat, mat.textures_[i].name_, mat.textures_[i].resource_->GetId());
            }
        }
    }
    
    void AvatarAppearance::ExportAvatar(Scene::EntityPtr entity, const std::string& account, const std::string& authserver, const std::string& password)
    {
        Foundation::ComponentPtr avatarptr = entity->GetComponent(EC_OpenSimAvatar::NameStatic());
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        if (!avatarptr || !appearanceptr)
            return;
        EC_OpenSimAvatar& avatar = *checked_static_cast<EC_OpenSimAvatar*>(avatarptr.get());
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(appearanceptr.get());
        
        // Have only one export running at a time
        if (avatar_exporter_)
        {
            RexLogicModule::LogInfo("Avatar export already running");
            return;
        }
        
        RexLogicModule::LogInfo("Avatar export for user " + account + " @ " + authserver);
        
        //! Instantiate new avatar exporter & give it the work request
        avatar_exporter_ = AvatarExporterPtr(new AvatarExporter());
        
        AvatarExporterRequestPtr request(new AvatarExporterRequest());
        request->account_ = account;
        request->authserver_ = authserver;
        request->password_ = password;
        
        //! Convert avatar appearance to xml
        //! Note: the exporter task will convert < > to &lt; &gt; to not confuse the eventual xmlrpc call
        QDomDocument avatar_export("Avatar");
        LegacyAvatarSerializer::WriteAvatarAppearance(avatar_export, appearance);
        
        std::string avatar_export_str = avatar_export.toString().toStdString();
        request->avatar_xml_ = avatar_export_str;
        
        GetAvatarAssetsForExport(request, appearance);
        
        avatar_exporter_->AddRequest<AvatarExporterRequest>(request);
    }
    
    void AvatarAppearance::GetAvatarAssetsForExport(AvatarExporterRequestPtr request, EC_AvatarAppearance& appearance)
    {
        GetAvatarAssetForExport(request, appearance.GetMesh());
        GetAvatarAssetForExport(request, appearance.GetSkeleton());
        
        AvatarMaterialVector materials = appearance.GetMaterials();
        for (Core::uint i = 0; i < materials.size(); ++i)
        {
            GetAvatarAssetForExport(request, materials[i].asset_, true);
            for (Core::uint j = 0; j < materials[i].textures_.size(); ++j)
            {
                GetAvatarAssetForExport(request, materials[i].textures_[j]);
            }
        }
        
        AvatarAttachmentVector attachments = appearance.GetAttachments();
        for (Core::uint i = 0; i < attachments.size(); ++i)
        {
            GetAvatarAssetForExport(request, attachments[i].mesh_);
            for (Core::uint j = 0; j < attachments[i].materials_.size(); ++j)
            {
                GetAvatarAssetForExport(request, attachments[i].materials_[j].asset_, true);
                for (Core::uint k = 0; k < attachments[i].materials_[j].textures_.size(); ++k)
                {
                    GetAvatarAssetForExport(request, attachments[i].materials_[j].textures_[k]);
                }
            }
        }
    }
    
    void AvatarAppearance::GetAvatarAssetForExport(AvatarExporterRequestPtr request, const AvatarAsset& asset, bool is_material)
    {
        std::string export_name = asset.name_;
        // Add .material to name if necessary
        if (is_material)
        {
            if (export_name.find(".material") == std::string::npos)
                export_name.append(".material");
        }
        
        // Skip if already exists with this name
        if (request->assets_.find(export_name) != request->assets_.end())
            return;
        
        ExportAsset new_export_asset;
        
        // If it's loaded from resource, we should be able to get at the original raw asset data for export
        if (asset.resource_)
        {
            boost::shared_ptr<Foundation::AssetServiceInterface> asset_service =
                rexlogicmodule_->GetFramework()->GetServiceManager()->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
            if (!asset_service)
            {
                RexLogicModule::LogError("Could not get asset service");
                return;
            }
            // The assettype doesn't matter here
            Foundation::AssetPtr raw_asset = asset_service->GetAsset(asset.resource_id_, std::string());
            if (!raw_asset)
            {
                RexLogicModule::LogError("Could not get raw asset data for resource " + asset.resource_id_);
                return;
            }
            if (raw_asset->GetSize())
            {
                new_export_asset.data_.resize(raw_asset->GetSize());
                memcpy(&new_export_asset.data_[0], raw_asset->GetData(), raw_asset->GetSize());
            }
            else
            {
                RexLogicModule::LogError("Zero size asset data");
                return;
            }
        }
        else
        {
            //! \todo get local assets some way
            RexLogicModule::LogError("Export of local avatar asset " + export_name + " not yet supported!");
            return;
        }
        
        new_export_asset.CalculateHash();
        
        // Check for hash duplicate
        ExportAssetMap::const_iterator i = request->assets_.begin();
        while (i != request->assets_.end())
        {
            if (new_export_asset.hash_ == i->second.hash_)
            {
                RexLogicModule::LogDebug("Skipping export of avatar asset " + export_name + ", has same hash as " + i->first);
                return;
            }
            ++i;
        }
        
        request->assets_[export_name] = new_export_asset;
    }
    
    void AvatarAppearance::ProcessAvatarExport()
    {
        if (avatar_exporter_)
        {
            AvatarExporterResultPtr result = avatar_exporter_->GetResult<AvatarExporterResult>();
            if (result)
            {
                if (result->success_)
                {
                    RexLogicModule::LogInfo("Avatar exported successfully");
                    // Send information of appearance change
                    RexServerConnectionPtr conn = rexlogicmodule_->GetServerConnection();
                    
                    std::string method = "RexAppearance";
                    Core::StringVector strings;
                    conn->SendGenericMessage(method, strings);
                }
                else
                    RexLogicModule::LogInfo("Avatar export failed: " + result->message_);
                
                avatar_exporter_.reset();
            }
        }
    }
}
