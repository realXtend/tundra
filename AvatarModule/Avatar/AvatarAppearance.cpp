// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Avatar/Avatar.h"
#include "Avatar/AvatarAppearance.h"
#include "Avatar/AvatarEditor.h"
#include "Avatar/AvatarExporter.h"
#include "LegacyAvatarSerializer.h"

#include "EntityComponent/EC_OpenSimAvatar.h"

#include "SceneManager.h"
#include "SceneEvents.h"
#include "EC_OgreMesh.h"
#include "EC_OgreMovableTextOverlay.h"
#include "OgreMaterialResource.h"
#include "OgreMaterialUtils.h"
#include "OgreLocalResourceUtils.h"
#include "Renderer.h"
#include "OgreConversionUtils.h"
#include "OgreMeshResource.h"
#include "OgreSkeletonResource.h"
#include "OgreMaterialResource.h"
#include "OgreImageTextureResource.h"
#include "OgreTextureResource.h"
#include "HttpTask.h"
#include "HttpUtilities.h"
#include "LLSDUtilities.h"
#include "AssetEvents.h"
#include "AssetServiceInterface.h"
#include "RenderServiceInterface.h"
#include "Inventory/InventoryEvents.h"
#include "ConfigurationManager.h"
#include "CoreStringUtils.h"
#include "ServiceManager.h"
#include "EventManager.h"
#include "WorldStream.h"
#include "EC_HoveringText.h"

#include <QDomDocument>
#include <QFile>
#include <QTime>

#include <Ogre.h>

using namespace RexTypes;

static const float FIXED_HEIGHT_OFFSET = -0.87f;
static const float OVERLAY_HEIGHT_MULTIPLIER = 1.5f;
static const uint XMLRPC_ASSET_HASH_LENGTH = 28;

namespace AvatarModule
{ 
    std::string ReplaceSpaces(const std::string& orig_str)
    {
        return ReplaceChar(orig_str, ' ', '_');
    }
        
    AvatarAppearance::AvatarAppearance(Foundation::Framework *framework, AvatarModule *avatar_module) :
        framework_(framework),
        avatar_module_(avatar_module),
        inv_export_state_(Idle)
    {
        std::string default_avatar_path = avatar_module_->GetFramework()->GetDefaultConfig().DeclareSetting("RexAvatar", "default_avatar_file", std::string("./data/default_avatar.xml"));
        
        ReadDefaultAppearance(default_avatar_path);
    }

    AvatarAppearance::~AvatarAppearance()
    {
    }

    void AvatarAppearance::Update(f64 frametime)
    {
        ProcessAppearanceDownloads();
        ProcessAvatarExport();
    }
    
    void AvatarAppearance::DownloadAppearance(Scene::EntityPtr entity, bool use_default)
    {
        if (!entity)
            return;
        
        EC_OpenSimAvatar* avatar = entity->GetComponent<EC_OpenSimAvatar>().get();
        if (!avatar)
            return;
            
        std::string appearance_address = avatar->GetAppearanceAddress();
        if (appearance_address.empty())
        {
            if (use_default)
                SetupDefaultAppearance(entity);
            return;
        }
        
        // See if it's NOT legacy avatar storage based address
        
        // OS INVENTORY BASED AVATAR
        QString q_app_address = QString::fromStdString(appearance_address);
        if (!q_app_address.contains("/avatar/"))
        {
            AvatarModule::LogDebug("Inventory based avatar address received for avatar entity " + ToString<int>(entity->GetId()) + ": " +
                appearance_address);
            boost::shared_ptr<Foundation::AssetServiceInterface> asset_service =
                avatar_module_->GetFramework()->GetServiceManager()->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
            if (!asset_service)
            {
                AvatarModule::LogError("Could not get asset service");
                return;
            }      
            request_tag_t tag = asset_service->RequestAsset(appearance_address, ASSETTYPENAME_GENERIC_AVATAR_XML);
            // Remember the request
            if (tag)
                avatar_appearance_tags_[tag] = entity->GetId();            
              
            return;         
        }
        // WEBDAV INVENTORY BASED AVATAR
        else if (q_app_address.endsWith("Avatar.xml"))
        {
            AvatarModule::LogDebug("Fetching webdav appearance from " + appearance_address);
            boost::shared_ptr<Foundation::AssetServiceInterface> asset_service =
                avatar_module_->GetFramework()->GetServiceManager()->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
            if (asset_service)
            {
                asset_service->RemoveAssetFromCache(appearance_address);
                request_tag_t tag = asset_service->RequestAsset(appearance_address, ASSETTYPENAME_GENERIC_AVATAR_XML);
                if (tag)
                    avatar_appearance_tags_[tag] = entity->GetId();
            }
            else
                AvatarModule::LogError("Could not get asset service");
            return;
        }
           
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
        default_appearance_ = boost::shared_ptr<QDomDocument>(new QDomDocument("Avatar"));
        
        QFile file(filename.c_str());
        if (!file.open(QIODevice::ReadOnly))
        {
            AvatarModule::LogError("Could not open avatar default appearance file " + filename);
            return;
        }
        
        if (!default_appearance_->setContent(&file))
        {
            file.close();
            AvatarModule::LogError("Could not parse avatar default appearance file " + filename);
            return;
        }
        file.close();
    }
    
    void AvatarAppearance::SetupDefaultAppearance(Scene::EntityPtr entity)
    {
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return;
        
        // Deserialize appearance from the document into the EC
        LegacyAvatarSerializer::ReadAvatarAppearance(*appearance, *default_appearance_);
        
        SetupAppearance(entity);
    }
    
    void AvatarAppearance::SetupAppearance(Scene::EntityPtr entity)
    {
        PROFILE(Avatar_SetupAppearance);
        
        if (!entity)
            return;
        
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        OgreRenderer::EC_OgreMesh* mesh = entity->GetComponent<OgreRenderer::EC_OgreMesh>().get();

        if (!mesh || !appearance)
            return;
                                          
        // If document contains no animations, use ones from default
        if (appearance->GetAnimations().empty())
        {
            AnimationDefinitionMap animations;
            LegacyAvatarSerializer::ReadAnimationDefinitions(animations, *default_appearance_);
            appearance->SetAnimations(animations);
        }
        
        // If mesh name is empty, it would certainly be an epic fail. Do nothing.
        if (appearance->GetMesh().name_.empty())
            return;
                
        // Fix up resource references
        FixupResources(entity);
        
        // Setup appearance
        SetupMeshAndMaterials(entity);
        SetupDynamicAppearance(entity);
        SetupAttachments(entity);
        
        // If there's no morph controls, interrogate the mesh for them
        if (!appearance->GetMorphModifiers().size())
        {
            MorphModifierVector new_morphs;
            
            Ogre::Entity *ogre_entity = mesh->GetEntity();
            if (ogre_entity)
            {
                // Add all pose animations
                Ogre::MeshPtr ogre_mesh = ogre_entity->getMesh();
                size_t numanims = ogre_mesh->getNumAnimations();
                for (uint i = 0; i < numanims; ++i)
                {
                    Ogre::Animation* anim = ogre_mesh->getAnimation(i);
                    Ogre::Animation::VertexTrackIterator it = anim->getVertexTrackIterator();
                    bool is_pose = false;
            
                    while (it.hasMoreElements())
                    {
                        Ogre::VertexAnimationTrack* vat = it.getNext();
                        if (vat->getAnimationType() == Ogre::VAT_POSE) is_pose = true;
                    }

                    if (is_pose)
                    {
                        MorphModifier new_morph;
                        new_morph.morph_name_ = anim->getName();
                        new_morph.value_ = 0.0f;
       
                        new_morph.name_ = new_morph.morph_name_;
                        // Strip away "Morph_" from the beginning for nicer human-readable name
                        if (new_morph.name_.find("Morph_") == 0)
                        {
                            new_morph.name_ = new_morph.name_.substr(6, new_morph.name_.length() - 6);
                        } 
                        new_morphs.push_back(new_morph);
                    }
                }  
                appearance->SetMorphModifiers(new_morphs);       
            }
        }
        
        // If this is the user's avatar, make the editor refresh its view (morphs, textures etc.)
        if (entity == avatar_module_->GetAvatarHandler()->GetUserAvatar())
        {
            AvatarModule::LogDebug("User avatar changed, rebuilding editor view");
            avatar_module_->GetAvatarEditor()->RebuildEditView(); 
        }  
    }
    
    void AvatarAppearance::SetupDynamicAppearance(Scene::EntityPtr entity)
    {
        if (!entity)
            return;
        
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        OgreRenderer::EC_OgreMesh* mesh = entity->GetComponent<OgreRenderer::EC_OgreMesh>().get();

        if (!mesh || !appearance)
            return;
        
        SetupMorphs(entity);
        SetupBoneModifiers(entity);
        AdjustHeightOffset(entity);
    }
    
    void AvatarAppearance::AdjustHeightOffset(Scene::EntityPtr entity)
    {
        if (!entity)
            return;
        
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        OgreRenderer::EC_OgreMesh* mesh = entity->GetComponent<OgreRenderer::EC_OgreMesh>().get();

        if (!mesh || !appearance)
            return;
        
        Ogre::Vector3 offset = Ogre::Vector3::ZERO;
        Ogre::Vector3 initial_base_pos = Ogre::Vector3::ZERO;

        if (appearance->HasProperty("baseoffset"))
        {
            initial_base_pos = Ogre::StringConverter::parseVector3(appearance->GetProperty("baseoffset"));
        }

        if (appearance->HasProperty("basebone"))
        {
            Ogre::Bone* base_bone = GetAvatarBone(entity, appearance->GetProperty("basebone"));
            if (base_bone)
            {
                Ogre::Vector3 temp;
                GetInitialDerivedBonePosition(base_bone, temp);
                initial_base_pos += temp;
                offset = initial_base_pos;

                // Additionally, if has the rootbone property, can do dynamic adjustment for sitting etc.
                // and adjust the name overlay height
                if (appearance->HasProperty("rootbone"))
                {
                    Ogre::Bone* root_bone = GetAvatarBone(entity, appearance->GetProperty("rootbone"));
                    if (root_bone)
                    {
                        Ogre::Vector3 initial_root_pos;
                        Ogre::Vector3 current_root_pos = root_bone->_getDerivedPosition();
                        GetInitialDerivedBonePosition(root_bone, initial_root_pos);

                        float c = abs(current_root_pos.y / initial_root_pos.y);
                        if (c > 1.0) c = 1.0;
                        offset = initial_base_pos * c;

                        // Ali: testing EC_HoveringText instead of EC_OgreMovableTextOverlay
                        // Set name overlay height according to base + root distance.
                        //OgreRenderer::EC_OgreMovableTextOverlay* overlay = entity->GetComponent<OgreRenderer::EC_OgreMovableTextOverlay>().get();
                        EC_HoveringText* overlay = entity->GetComponent<EC_HoveringText>().get();
                        if (overlay)
                        {
                            overlay->SetPosition(Vector3df(0, 0, abs(initial_base_pos.y - initial_root_pos.y) * OVERLAY_HEIGHT_MULTIPLIER));
                            //overlay->SetOffset(Vector3df(0, 0, abs(initial_base_pos.y - initial_root_pos.y) * OVERLAY_HEIGHT_MULTIPLIER));
                        }
                    }
                }
            }
        }

        mesh->SetAdjustPosition(Vector3df(0.0f, 0.0f, -offset.y + FIXED_HEIGHT_OFFSET));
    }
    
    void AvatarAppearance::SetupMeshAndMaterials(Scene::EntityPtr entity)
    {
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        OgreRenderer::EC_OgreMesh* mesh = entity->GetComponent<OgreRenderer::EC_OgreMesh>().get();
                
        // Mesh needs to be cloned if there are attachments which need to hide vertices
        bool need_mesh_clone = false;
        
        const AvatarAttachmentVector& attachments = appearance->GetAttachments();
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
        
        if (!appearance->GetSkeleton().GetLocalOrResourceName().empty())
            mesh->SetMeshWithSkeleton(appearance->GetMesh().GetLocalOrResourceName(), appearance->GetSkeleton().GetLocalOrResourceName(), need_mesh_clone);
        else
            mesh->SetMesh(appearance->GetMesh().GetLocalOrResourceName(), need_mesh_clone);
            
        if (need_mesh_clone)
            HideVertices(mesh->GetEntity(), vertices_to_hide);
        
        AvatarMaterialVector materials = appearance->GetMaterials();
        for (uint i = 0; i < materials.size(); ++i)
        {
            mesh->SetMaterial(i, materials[i].asset_.GetLocalOrResourceName());
        }
        
        // Store the modified materials vector (with created temp resources) to the EC
        appearance->SetMaterials(materials);
        
        // Set adjustment orientation for mesh (Ogre meshes usually have Y-axis as vertical)
        Quaternion adjust(PI/2, 0, -PI/2);
        mesh->SetAdjustOrientation(adjust);
        // Position approximately within the bounding box
        // Will be overridden by bone-based height adjust, if available
        mesh->SetAdjustPosition(Vector3df(0.0f, 0.0f, FIXED_HEIGHT_OFFSET));
        mesh->SetCastShadows(true);
        
        Scene::Events::EntityEventData event_data;
        event_data.entity = entity;
        Foundation::EventManagerPtr event_manager = avatar_module_->GetFramework()->GetEventManager();
        event_manager->SendEvent("Scene", Scene::Events::EVENT_ENTITY_VISUALS_MODIFIED, &event_data);
    }
    
    void AvatarAppearance::SetupAttachments(Scene::EntityPtr entity)
    {
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        OgreRenderer::EC_OgreMesh* mesh = entity->GetComponent<OgreRenderer::EC_OgreMesh>().get();
                
        mesh->RemoveAllAttachments();
        
        const AvatarAttachmentVector& attachments = appearance->GetAttachments();
        
        for (uint i = 0; i < attachments.size(); ++i)
        {
            // Setup attachment meshes
            mesh->SetAttachmentMesh(i, attachments[i].mesh_.GetLocalOrResourceName(), attachments[i].bone_name_, attachments[i].link_skeleton_);
            // Setup attachment mesh materials
            for (uint j = 0; j < attachments[i].materials_.size(); ++j)
            {
                mesh->SetAttachmentMaterial(i, j, attachments[i].materials_[j].asset_.GetLocalOrResourceName());
            }
            mesh->SetAttachmentPosition(i, attachments[i].transform_.position_);
            mesh->SetAttachmentOrientation(i, attachments[i].transform_.orientation_);
            mesh->SetAttachmentScale(i, attachments[i].transform_.scale_);
        }
    }
    
    void AvatarAppearance::SetupMorphs(Scene::EntityPtr entity)
    {
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        OgreRenderer::EC_OgreMesh* mesh = entity->GetComponent<OgreRenderer::EC_OgreMesh>().get();
                
        Ogre::Entity* ogre_entity = mesh->GetEntity();
        if (!ogre_entity)
            return;
        Ogre::AnimationStateSet* anims = ogre_entity->getAllAnimationStates();
        if (!anims)
            return;
            
        const MorphModifierVector& morphs = appearance->GetMorphModifiers();
        for (uint i = 0; i < morphs.size(); ++i)
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
                for (uint j = 0; j < mesh->GetNumAttachments(); ++j)
                {
                    Ogre::Entity* attachment = mesh->GetAttachmentEntity(j);
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
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
                
        ResetBones(entity);
        
        const BoneModifierSetVector& bone_modifiers = appearance->GetBoneModifiers();
        for (uint i = 0; i < bone_modifiers.size(); ++i)
        {
            for (uint j = 0; j < bone_modifiers[i].modifiers_.size(); ++j)
            {
                ApplyBoneModifier(entity, bone_modifiers[i].modifiers_[j], bone_modifiers[i].value_);
            }
        }
    }
    
    void AvatarAppearance::ResetBones(Scene::EntityPtr entity)
    {
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        OgreRenderer::EC_OgreMesh* mesh = entity->GetComponent<OgreRenderer::EC_OgreMesh>().get();
                
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
    
    void AvatarAppearance::ApplyBoneModifier(Scene::EntityPtr entity, const BoneModifier& modifier, float value)
    {
        OgreRenderer::EC_OgreMesh* mesh = entity->GetComponent<OgreRenderer::EC_OgreMesh>().get();
        
        Ogre::Entity* ogre_entity = mesh->GetEntity();
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
            case BoneModifier::Absolute:
                bx = 0;
                by = 0;
                bz = 0;
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
            
            Ogre::Vector3 trans, base;
            trans = bone->getInitialPosition();
            switch (modifier.position_mode_)
            {
            case BoneModifier::Absolute:
                base = Ogre::Vector3(0,0,0);
                break;
            case BoneModifier::Relative:
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
        if (!entity)
            return 0;            
        OgreRenderer::EC_OgreMesh* mesh = entity->GetComponent<OgreRenderer::EC_OgreMesh>().get();
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
    
    void AvatarAppearance::HideVertices(Ogre::Entity* entity, std::set<uint> vertices_to_hide)
    {
        if (!entity)
            return;
        Ogre::MeshPtr mesh = entity->getMesh();
        if (mesh.isNull())
            return;
        
        for (uint m = 0; m < 1; ++m)
        {
            // Under current system, it seems vertices should only be hidden from first submesh
            Ogre::SubMesh *submesh = mesh->getSubMesh(m);
            Ogre::IndexData *data = submesh->indexData;
            Ogre::HardwareIndexBufferSharedPtr ibuf = data->indexBuffer;

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
    
    void AvatarAppearance::ProcessAppearanceDownloads()
    {
        // Check download results
        std::map<entity_id_t, HttpUtilities::HttpTaskPtr>::iterator i = appearance_downloaders_.begin();
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
                        Scene::EntityPtr entity = avatar_module_->GetAvatarEntity(i->first);
                        if (entity)
                            ProcessAppearanceDownload(entity, &result->data_[0], result->data_.size());
                    }
                    else
                        AvatarModule::LogInfo("Error downloading avatar appearance for avatar " + ToString<int>(i->first) + ": " + result->reason_);
                    
                    done = true;
                }
            }
            
            if (!done)
                ++i;
            else
                appearance_downloaders_.erase(i++);
        }
    }

    void AvatarAppearance::ProcessInventoryAppearance(Scene::EntityPtr entity, const u8* data, uint size, QString base_url)
    {       
        if (!entity)
            return;
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return;
        
        std::string data_str((const char*)data, size);

        QDomDocument avatar_doc("Avatar");
        avatar_doc.setContent(QString::fromStdString(data_str));

        // Deserialize appearance from the document into the EC
        if (!LegacyAvatarSerializer::ReadAvatarAppearance(*appearance, avatar_doc))
        {
            // If fails badly, setup default instead
            AvatarModule::LogInfo("Failed to parse avatar description, setting default appearance");
            SetupDefaultAppearance(entity);
            return;
        }
        
        const AvatarAssetMap& assets = appearance->GetAssetMap(); 
        
        uint pending_requests;
        if (base_url.isEmpty())
            pending_requests = RequestAvatarResources(entity, assets, true);
        else
        {
            // If base url exists, this is webdav inventory avatar
            // Lets clear the cache as the id == url doesnt chance so
            // we can be sure the asset is fetched again from the web
            boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = 
                avatar_module_->GetFramework()->GetServiceManager()->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
            if (asset_service)
            {
                AvatarAssetMap::const_iterator iter = assets.begin();
                AvatarAssetMap::const_iterator end = assets.end();
                while (iter != end)
                {
                    std::string asset_id = iter->second;
                    asset_service->RemoveAssetFromCache(asset_id);
                    ++iter;
                }
            }
            pending_requests = RequestAvatarResources(entity, assets, false, base_url);
        }
        
        // In the unlikely case of no requests at all, rebuild avatar now
        if (!pending_requests)
            SetupAppearance(entity);
    }
        
    uint AvatarAppearance::RequestAvatarResources(Scene::EntityPtr entity, const AvatarAssetMap& assets, bool inventorymode, QString base_url)
    {
        // Erase any old pending requests for this avatar, they are no longer interesting
        std::vector<std::map<request_tag_t, entity_id_t>::iterator> tags_to_remove;
        std::map<request_tag_t, entity_id_t>::iterator i = avatar_resource_tags_.begin();
        while (i != avatar_resource_tags_.end())
        {
            if (i->second == entity->GetId())
                tags_to_remove.push_back(i);
            ++i;
        }
        for (uint j = 0; j < tags_to_remove.size(); ++j)
        {
            avatar_resource_tags_.erase(tags_to_remove[j]);
        } 
                
        // Request needed avatar resources
        boost::shared_ptr<OgreRenderer::Renderer> renderer = avatar_module_->GetFramework()->GetServiceManager()->
            GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (!renderer)
        {
            AvatarModule::LogError("Renderer does not exist");
            return 0;
        }    
    
        RequestTagVector tags;
        AvatarAssetMap::const_iterator k = assets.begin();
        uint pending_requests = 0;        
        while (k != assets.end())
        {
            std::string resource_id = k->second;
            request_tag_t tag = renderer->RequestResource(resource_id, GetResourceTypeFromName(k->first, inventorymode));

            if (tag)
            {
                tags.push_back(tag);
                avatar_resource_tags_[tag] = entity->GetId();
                pending_requests++;
            }
            ++k;
        }
        avatar_pending_requests_[entity->GetId()] = pending_requests;

        return pending_requests;    
    }
    
    
    void AvatarAppearance::ProcessAppearanceDownload(Scene::EntityPtr entity, const u8* data, uint size)
    {        
        if (!entity)
            return;
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        EC_OpenSimAvatar* avatar = entity->GetComponent<EC_OpenSimAvatar>().get();
        if (!appearance || !avatar)
            return;
        
        std::string data_str((const char*)data, size);
        std::map<std::string, std::string> contents = RexTypes::ParseLLSDMap(data_str);
        
        // Get the avatar appearance description ("generic xml")
        std::map<std::string, std::string>::iterator i = contents.find("generic xml");
        if (i == contents.end())
        {
            // If not found, use default appearance
            // (at this point, it's nice to just have *some* appearance change, for example
            // changing back to default human from fish in the fishworld, if no avatar stored)
            AvatarModule::LogInfo("Got empty avatar description from storage, setting default appearance");
            SetupDefaultAppearance(entity);
            return;
        }
        
        std::string& appearance_str = i->second;

        // Return to original format by substituting to < >
        ReplaceSubstringInplace(appearance_str, "&lt;", "<");
        ReplaceSubstringInplace(appearance_str, "&gt;", ">");
        
        QDomDocument avatar_doc("Avatar");
        avatar_doc.setContent(QString::fromStdString(appearance_str));
        
        std::map<std::string, std::string>::iterator j = contents.begin();
        // Build mapping of human-readable asset names to id's
        std::string host = HttpUtilities::GetHostFromUrl(avatar->GetAppearanceAddress());
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
        if (!LegacyAvatarSerializer::ReadAvatarAppearance(*appearance, avatar_doc))
        {
            // If fails badly, setup default instead
            AvatarModule::LogInfo("Failed to parse avatar description, setting default appearance");
            SetupDefaultAppearance(entity);
            return;
        }
        
        appearance->SetAssetMap(assets);

        uint pending_requests = RequestAvatarResources(entity, assets);
        
        // In the unlikely case of no requests at all, rebuild avatar now
        if (!pending_requests)
            SetupAppearance(entity);
    }
    
    bool AvatarAppearance::HandleResourceEvent(event_id_t event_id, IEventData* data)
    {
        if (event_id != Resource::Events::RESOURCE_READY)
            return false;

        Resource::Events::ResourceReady* event_data = checked_static_cast<Resource::Events::ResourceReady*>(data);
        if (!event_data)
            return false;
        std::map<request_tag_t, entity_id_t>::iterator i = avatar_resource_tags_.find(event_data->tag_);
        if (i == avatar_resource_tags_.end())
            return false;
           
        // Now we know it is our request, can erase it and don't need to propagate this event      
        AvatarModule::LogDebug("Got avatar resource " + event_data->id_ + " type " + event_data->resource_->GetType());      
        entity_id_t id = i->second;

        avatar_resource_tags_.erase(i);
        if (avatar_pending_requests_[id])
            avatar_pending_requests_[id]--;
            
        Scene::EntityPtr entity = avatar_module_->GetAvatarEntity(id);
        if (!entity)
            return true;

        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return true;
        
        // If was the last request, rebuild avatar
        if (avatar_pending_requests_[id] == 0)
        {
            AvatarModule::LogDebug("All resources received, rebuilding avatar");
            SetupAppearance(entity);
        }
    
        return true;
    }
    
    bool AvatarAppearance::HandleAssetEvent(event_id_t event_id, IEventData* data)
    {
        if (event_id != Asset::Events::ASSET_READY)
            return false;
            
        Asset::Events::AssetReady *event_data = checked_static_cast<Asset::Events::AssetReady*>(data); 
        // See that tag matches
        std::map<request_tag_t, entity_id_t>::iterator i = avatar_appearance_tags_.find(event_data->tag_);
        if (i == avatar_appearance_tags_.end())
            return false;
            
        // Now we know it is our request, can erase it and don't need to propagate this event
        entity_id_t id = i->second;
        avatar_appearance_tags_.erase(i);
        Scene::EntityPtr entity = avatar_module_->GetAvatarEntity(id);
        if (!entity)
            return true;

        Foundation::AssetPtr asset = event_data->asset_;
        if (!asset) 
            return true;

        QString asset_id = QString::fromStdString(asset->GetId());

        // WEBDAV
        if (asset_id.endsWith("Avatar.xml"))
        {
            QString base_url = asset_id.left(asset_id.length() - QString("Avatar.xml").length());
            ProcessInventoryAppearance(entity, asset->GetData(), asset->GetSize(), base_url);
        }
        // OS INVENTORY
        else
            ProcessInventoryAppearance(entity, asset->GetData(), asset->GetSize());

        return true;
    }
    
    bool AvatarAppearance::HandleInventoryEvent(event_id_t event_id, IEventData* data)
    {
        if (event_id == Inventory::Events::EVENT_INVENTORY_DESCENDENT)
        {
            Inventory::InventoryItemEventData* event_data = dynamic_cast<Inventory::InventoryItemEventData*>(data);
            if (event_data)
            {
                if (event_data->assetType == RexTypes::RexAT_GenericAvatarXml && inv_export_state_ == Avatar)
                {
                    // See that the asset is actually an avatar description we uploaded
                    if (event_data->name.find("Avatar") != std::string::npos)
                    {
                        ProtocolUtilities::WorldStreamPtr conn = avatar_module_->GetServerConnection();
                        if (conn)
                        {
                            AvatarModule::LogDebug("Sending info about new inventory based appearance " + event_data->assetId.ToString());
                            StringVector strings;
                            std::string method = "RexSetAppearance";
                            strings.push_back(conn->GetInfo().agentID.ToString());
                            strings.push_back(event_data->assetId.ToString());
                            conn->SendGenericMessage(method, strings);
                        }
                        
                        // Inventory based export done!
                        inv_export_state_ = Idle;
                    }
                }
                
                if ((inv_export_state_ == Assets) && (inv_export_request_))
                {
                    // Check if asset is one from our upload request
                    ExportAssetMap::const_iterator i = inv_export_request_->assets_.begin();
                    while (i != inv_export_request_->assets_.end())
                    {
                        if (i->first == event_data->fileName)
                        {
                            // Store the asset ID to assetmap 
                            inv_export_assetmap_[i->first] = event_data->assetId.ToString();
                            // If it was the last, now we can export avatar itself
                            if (inv_export_assetmap_.size() == inv_export_request_->assets_.size())
                            {
                                InventoryExportAvatarFinalize(inv_export_entity_.lock());
                            }
                            break;
                        }
                        ++i;
                    }
                }
            }
        }

        if (event_id == Inventory::Events::EVENT_INVENTORY_WEBDAV_AVATAR_ASSETS_UPLOAD_COMPLETE)
        {
            Inventory::WebDavInventoryUploadedData* event_data = dynamic_cast<Inventory::WebDavInventoryUploadedData*>(data);
            if (event_data)
            {
                WebDavExportAvatarFinalize(inv_export_entity_.lock(), event_data->file_list);
                return true;
            }
        }

        if (event_id == Inventory::Events::EVENT_INVENTORY_WEBDAV_AVATAR_XML_UPLOAD_COMPLETE)
        {
            ProtocolUtilities::WorldStreamPtr connection = avatar_module_->GetServerConnection();
            EC_OpenSimAvatar* avatar = inv_export_entity_.lock()->GetComponent<EC_OpenSimAvatar>().get();
            if (connection && avatar)
            {
                AvatarModule::LogDebug("Informing ModRex about webdav avatar appearance update.");
                StringVector strings;
                strings.push_back(connection->GetInfo().agentID.ToString());
                strings.push_back(avatar->GetAppearanceAddress());
                connection->SendGenericMessage("RexSetAppearance", strings);
            }
            inv_export_state_ = Idle;
            inv_export_entity_.reset();
        }
        
        return false;
    }   

    const std::string& AvatarAppearance::GetResourceTypeFromName(const std::string& name, bool inventorymode)
    {
        if (name.find(".mesh") != std::string::npos)
            return OgreRenderer::OgreMeshResource::GetTypeStatic();
        if (name.find(".skeleton") != std::string::npos)
            return OgreRenderer::OgreSkeletonResource::GetTypeStatic();
        if (name.find(".material") != std::string::npos)
            return OgreRenderer::OgreMaterialResource::GetTypeStatic();
        
        // If not any of these, assume a texture image (.png, .jpg etc.)
        if (!inventorymode)
            return OgreRenderer::OgreImageTextureResource::GetTypeStatic();
        // In inventory mode, we first have no option but to j2k-decode everything (no general image asset)
        else
            return OgreRenderer::OgreTextureResource::GetTypeStatic();
    }
    
    void AvatarAppearance::FixupResources(Scene::EntityPtr entity)
    {
        if (!entity)
            return;
            
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return;
            
        const AvatarAssetMap& asset_map = appearance->GetAssetMap();
        
        // Get mesh, skeleton, materials & attachments
        AvatarAsset mesh = appearance->GetMesh();
        AvatarAsset skeleton = appearance->GetSkeleton();
        AvatarMaterialVector materials = appearance->GetMaterials();
        AvatarAttachmentVector attachments = appearance->GetAttachments();
        
        // Fix mesh & skeleton
        FixupResource(mesh, asset_map, OgreRenderer::OgreMeshResource::GetTypeStatic());
        // If mesh is local, need to setup the skeleton & materials
        if (!mesh.resource_.lock().get())
        {
            Ogre::MeshPtr ogremesh = OgreRenderer::GetLocalMesh(mesh.name_);
            if (!ogremesh.isNull())
            {
                skeleton = AvatarAsset();
                skeleton.name_ = ogremesh->getSkeletonName();
                
                // Interrogate materials only if not known yet
                if (!materials.size())
                {
                    for (uint j = 0; j < ogremesh->getNumSubMeshes(); ++j)
                    {
                        Ogre::SubMesh* submesh = ogremesh->getSubMesh(j);
                        AvatarMaterial attach_newmat;
                        attach_newmat.asset_.name_ = submesh->getMaterialName();
                        materials.push_back(attach_newmat);
                    }
                }
            }
        }
                
        FixupResource(skeleton, asset_map, OgreRenderer::OgreSkeletonResource::GetTypeStatic());
        
        // Fix avatar mesh materials
        for (uint i = 0; i < materials.size(); ++i)
            FixupMaterial(materials[i], asset_map);
        // Fix attachment meshes & their materials
        for (uint i = 0; i < attachments.size(); ++i)
        {
            FixupResource(attachments[i].mesh_, asset_map, OgreRenderer::OgreMeshResource::GetTypeStatic());
            
            if (attachments[i].mesh_.resource_.lock())
            {
                OgreRenderer::OgreMeshResource* mesh_res = dynamic_cast<OgreRenderer::OgreMeshResource*>(attachments[i].mesh_.resource_.lock().get());
                if (mesh_res)
                {
                    const StringVector& attach_matnames = mesh_res->GetOriginalMaterialNames();
                    attachments[i].materials_.clear();

                    for (uint j = 0; j < attach_matnames.size(); ++j)
                    {
                        AvatarMaterial attach_newmat;
                        attach_newmat.asset_.name_ = attach_matnames[j];
                        FixupMaterial(attach_newmat, asset_map);
                        attachments[i].materials_.push_back(attach_newmat);
                    }
                }
            }
            else
            {
                // If attachment mesh is local, get material names directly from the mesh
                Ogre::MeshPtr ogremesh = OgreRenderer::GetLocalMesh(attachments[i].mesh_.name_);
                if (!ogremesh.isNull())
                {
                    attachments[i].materials_.clear();
                    for (uint j = 0; j < ogremesh->getNumSubMeshes(); ++j)
                    {
                        Ogre::SubMesh* submesh = ogremesh->getSubMesh(j);
                        AvatarMaterial attach_newmat;
                        attach_newmat.asset_.name_ = submesh->getMaterialName();
                        FixupMaterial(attach_newmat, asset_map);
                        attachments[i].materials_.push_back(attach_newmat);
                    }
                }
            }
        }
        
        // Set modified mesh, skeleton, materials & attachments
        appearance->SetMesh(mesh);
        appearance->SetSkeleton(skeleton);
        appearance->SetMaterials(materials);
        appearance->SetAttachments(attachments);
    }
    
    void AvatarAppearance::FixupResource(AvatarAsset& asset, const AvatarAssetMap& asset_map, const std::string& resource_type)
    {
        boost::shared_ptr<OgreRenderer::Renderer> renderer = avatar_module_->GetFramework()->GetServiceManager()->
            GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (!renderer)
        {
            AvatarModule::LogError("Renderer does not exist");
            return;
        }
        
        if (!asset.resource_.lock().get())
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
        boost::shared_ptr<OgreRenderer::Renderer> renderer = avatar_module_->GetFramework()->GetServiceManager()->
            GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (!renderer)
            return;
            
        // First thing to do: append .material to name if it doesn't exist, because our asset map is based on full asset name,
        // and also the storage kind of does not like if it cannot identify the asset
        std::string fixed_mat_name = mat.asset_.name_;
        if (fixed_mat_name.find(".material") == std::string::npos)
            fixed_mat_name.append(".material");
        
        // First find resource for the material itself
        if (!mat.asset_.resource_.lock().get())
        {
            AvatarAssetMap::const_iterator i = asset_map.find(fixed_mat_name);
            if (i != asset_map.end())
            {
                mat.asset_.resource_id_ = i->second;
                mat.asset_.resource_ = renderer->GetResource(mat.asset_.resource_id_, OgreRenderer::OgreMaterialResource::GetTypeStatic());
            }
        }
        // If couldn't still be found, it's a local resource. In that case, fixup the default texture names for eventual export
        if (!mat.asset_.resource_.lock().get()) 
        {
            if (!mat.textures_.size())
            {
                Ogre::MaterialPtr ogremat = OgreRenderer::GetLocalMaterial(mat.asset_.name_);
                if (ogremat.isNull())
                    return;
                
                // Use a set to avoid duplicates
                std::set<std::string> textures_set;
                
                Ogre::Material::TechniqueIterator iter = ogremat->getTechniqueIterator();
                while(iter.hasMoreElements())
                {
                    Ogre::Technique *tech = iter.getNext();
                    assert(tech);
                    Ogre::Technique::PassIterator passIter = tech->getPassIterator();
                    while(passIter.hasMoreElements())
                    {
                        Ogre::Pass *pass = passIter.getNext();
                        
                        Ogre::Pass::TextureUnitStateIterator texIter = pass->getTextureUnitStateIterator();
                        
                        while(texIter.hasMoreElements())
                        {
                            Ogre::TextureUnitState *texUnit = texIter.getNext();
                            const std::string& texname = texUnit->getTextureName();
                            
                            if (!texname.empty())
                                textures_set.insert(texname);
                        }
                    }
                }
                
                std::set<std::string>::iterator i = textures_set.begin();
                
                while (i != textures_set.end())
                {
                    AvatarAsset new_tex;
                    new_tex.name_ = *i;
                    mat.textures_.push_back(new_tex);
                    ++i;
                }
            }
        }
        
        OgreRenderer::OgreMaterialResource* mat_res = dynamic_cast<OgreRenderer::OgreMaterialResource*>(mat.asset_.resource_.lock().get());
        if (!mat_res)
            return;
            
        const StringVector& orig_textures = mat_res->GetOriginalTextureNames();
        if (mat.textures_.size() < orig_textures.size())
            mat.textures_.resize(orig_textures.size());
        for (uint i = 0; i < mat.textures_.size(); ++i)
        {
            if (!mat.textures_[i].resource_.lock().get())
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
            // If we found the texture, modify the material to use it.
            if (mat.textures_[i].resource_.lock().get())
            {
                Ogre::MaterialPtr ogremat = mat_res->GetMaterial();
                OgreRenderer::ReplaceTextureOnMaterial(ogremat, mat.textures_[i].name_, mat.textures_[i].resource_.lock()->GetId());
            }
        }
    }

    void AvatarAppearance::WebDavExportAvatar(Scene::EntityPtr entity)
    {
        if (!entity)
            return;

        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return;

        if (inv_export_state_ != Idle)
        {
            AvatarModule::LogInfo("Avatar export already running");
            return;
        }

        // Get assets for export, inventory mode, using a dummy export request
        inv_export_entity_ = entity;
        inv_export_assetmap_ = AvatarAssetMap();
        inv_export_request_ = AvatarExporterRequestPtr(new AvatarExporterRequest());
        GetAvatarAssetsForExport(inv_export_request_, *appearance, true);   
        
        // Check if there were any assets, if not, we can go to final phase directly
        if (inv_export_request_->assets_.empty())
        {
            // Is this gonna happen under what circumstances??
            WebDavExportAvatarFinalize(entity, QStringList());
            return;
        }
         
        // If there are assets, stuff them all
        inv_export_state_ = Assets;
        Foundation::EventManagerPtr eventmgr = avatar_module_->GetFramework()->GetEventManager();
        Inventory::InventoryUploadBufferEventData event_data;
        
        ExportAssetMap::const_iterator i = inv_export_request_->assets_.begin();
        while (i != inv_export_request_->assets_.end())
        {
            QVector<u8> data_buffer;
            data_buffer.resize(i->second.data_.size());
            memcpy(&data_buffer[0], &i->second.data_[0], i->second.data_.size());
                        
            // Slashes in filenames cause problems. Replace. The exact format of the filename should not
            // matter (never used to reference the asset), as long as the asset type is deduced correctly
            event_data.filenames.push_back(QString::fromStdString(ReplaceChar(i->first, '/', '_')));
            event_data.buffers.push_back(data_buffer);
            ++i;
        }

        eventmgr->SendEvent("Inventory", Inventory::Events::EVENT_INVENTORY_WEBDAV_AVATAR_ASSETS_UPLOAD_REQUEST, &event_data);   
    }

    void AvatarAppearance::WebDavExportAvatarFinalize(Scene::EntityPtr entity, const QStringList &file_list)
    {
        // Webdav based export, final phase
        inv_export_state_ = Idle;
        inv_export_request_.reset();  
                
        if (!entity)
            return;     
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return;
        
        EC_OpenSimAvatar* avatar = entity->GetComponent<EC_OpenSimAvatar>().get();
        if (!avatar)
            return;

        QString app_address = QString::fromStdString(avatar->GetAppearanceAddress());
        if (app_address.endsWith("Avatar.xml"))
            app_address = app_address.left(app_address.length() - QString("Avatar.xml").length());

        // Fill the asset map with files
        inv_export_assetmap_ = AvatarAssetMap();
        foreach(QString asset_name, file_list)
            inv_export_assetmap_[asset_name.toStdString()] = app_address.toStdString() + asset_name.toStdString();

        // Convert avatar appearance to xml
        inv_export_state_ = Avatar;
        EC_AvatarAppearance temp_appearance = *appearance;
        temp_appearance.SetAssetMap(inv_export_assetmap_);
        inv_export_assetmap_ = AvatarAssetMap();
                
        QDomDocument avatar_export("Avatar");
        LegacyAvatarSerializer::WriteAvatarAppearance(avatar_export, temp_appearance, true);
        std::string avatar_export_str = avatar_export.toString().toStdString();

        QVector<u8> data_buffer;
        data_buffer.resize(avatar_export_str.length());
        memcpy(&data_buffer[0], avatar_export_str.c_str(), data_buffer.size());
        
        // Get current time/date to "version" the avatar inventory item
        QDateTime time = QDateTime::currentDateTime();
        std::string avatarfilename = "Avatar.xml";
                
        // Upload appearance as inventory asset. 
        Foundation::EventManagerPtr eventmgr = avatar_module_->GetFramework()->GetEventManager();
        Inventory::InventoryUploadBufferEventData event_data;
        event_data.filenames.push_back(QString(avatarfilename.c_str()));
        event_data.buffers.push_back(data_buffer);
        eventmgr->SendEvent("Inventory", Inventory::Events::EVENT_INVENTORY_WEBDAV_AVATAR_XML_UPLOAD_REQUEST, &event_data);       
    }
    
    void AvatarAppearance::InventoryExportAvatar(Scene::EntityPtr entity)
    {
        if (!entity)
            return;                           
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return;

        if (inv_export_state_ != Idle)
        {
            AvatarModule::LogInfo("Avatar export already running");
            return;
        }

        // Get assets for export, inventory mode, using a dummy export request
        inv_export_entity_ = entity;
        inv_export_assetmap_ = AvatarAssetMap();
        inv_export_request_ = AvatarExporterRequestPtr(new AvatarExporterRequest());
        GetAvatarAssetsForExport(inv_export_request_, *appearance, true);   
        
        // Check if there were any assets, if not, we can go to final phase directly
        if (inv_export_request_->assets_.empty())
        {
            InventoryExportAvatarFinalize(entity);
            return;
        }
         
        // If there are assets, stuff them all
        inv_export_state_ = Assets;
        Foundation::EventManagerPtr eventmgr = avatar_module_->GetFramework()->GetEventManager();
        Inventory::InventoryUploadBufferEventData event_data;
        
        ExportAssetMap::const_iterator i = inv_export_request_->assets_.begin();
        while (i != inv_export_request_->assets_.end())
        {
            QVector<u8> data_buffer;
            data_buffer.resize(i->second.data_.size());
            memcpy(&data_buffer[0], &i->second.data_[0], i->second.data_.size());
                        
            // Slashes in filenames cause problems. Replace. The exact format of the filename should not
            // matter (never used to reference the asset), as long as the asset type is deduced correctly
            event_data.filenames.push_back(QString::fromStdString(ReplaceChar(i->first, '/', '_')));
            event_data.buffers.push_back(data_buffer);
            ++i;
        }

        eventmgr->SendEvent("Inventory", Inventory::Events::EVENT_INVENTORY_UPLOAD_BUFFER, &event_data);                   
    }                 
    
    void AvatarAppearance::InventoryExportAvatarFinalize(Scene::EntityPtr entity)
    {
        // Inventory based export, final phase
        inv_export_state_ = Idle;
        inv_export_request_.reset();  
        inv_export_entity_.reset();
                
        if (!entity)
            return;                             
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return;
            
        // Convert avatar appearance to xml
        inv_export_state_ = Avatar;
        EC_AvatarAppearance temp_appearance = *appearance;
        temp_appearance.SetAssetMap(inv_export_assetmap_);
        inv_export_assetmap_ = AvatarAssetMap();           
                
        QDomDocument avatar_export("Avatar");
        LegacyAvatarSerializer::WriteAvatarAppearance(avatar_export, temp_appearance, true);
        std::string avatar_export_str = avatar_export.toString().toStdString();

        QVector<u8> data_buffer;
        data_buffer.resize(avatar_export_str.length());
        memcpy(&data_buffer[0], avatar_export_str.c_str(), data_buffer.size());
        
        // Get current time/date to "version" the avatar inventory item
        QDateTime time = QDateTime::currentDateTime();
        std::string avatarfilename = "Avatar" + time.toString(" yyyy.MM.dd hh:mm:ss").toStdString() + ".xml";
                
        // Upload appearance as inventory asset. 
        Foundation::EventManagerPtr eventmgr = avatar_module_->GetFramework()->GetEventManager();
        Inventory::InventoryUploadBufferEventData event_data;
        event_data.filenames.push_back(QString(avatarfilename.c_str()));
        event_data.buffers.push_back(data_buffer);
        eventmgr->SendEvent("Inventory", Inventory::Events::EVENT_INVENTORY_UPLOAD_BUFFER, &event_data);       
    }

    void AvatarAppearance::InventoryExportReset()
    {
        inv_export_state_ = Idle;
        inv_export_request_.reset();  
        inv_export_entity_.reset();
        inv_export_assetmap_ = AvatarAssetMap();        
    }
        
    void AvatarAppearance::ExportAvatar(Scene::EntityPtr entity, const std::string& account, const std::string& authserver, const std::string& password)
    {
        if (!entity)
            return;                             
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return;
            
        // Have only one export running at a time
        if (avatar_exporter_)
        {
            AvatarModule::LogInfo("Avatar export already running");
            return;
        }
        
        AvatarModule::LogInfo("Avatar export for user " + account + " @ " + authserver);
        
        // Instantiate new avatar exporter & give it the work request
        avatar_exporter_ = AvatarExporterPtr(new AvatarExporter());
        
        AvatarExporterRequestPtr request(new AvatarExporterRequest());
        request->account_ = account;
        request->authserver_ = authserver;
        request->password_ = password;

        // Convert avatar appearance to xml
        QDomDocument avatar_export("Avatar");
        LegacyAvatarSerializer::WriteAvatarAppearance(avatar_export, *appearance);
        std::string avatar_export_str = avatar_export.toString().toStdString();
        request->avatar_xml_ = avatar_export_str;

        // Get assets for export
        GetAvatarAssetsForExport(request, *appearance);
        
        avatar_exporter_->AddRequest<AvatarExporterRequest>(request);
    }
    
    void AvatarAppearance::ExportAvatarLocal(Scene::EntityPtr entity, const std::string& outname)
    {
        if (!entity)
            return;
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return;
        
        boost::filesystem::path path(outname);
        std::string dirname = path.branch_path().string();
        
        // Convert avatar appearance to xml
        QDomDocument avatar_export("Avatar");
        LegacyAvatarSerializer::WriteAvatarAppearance(avatar_export, *appearance);
        QByteArray bytes = avatar_export.toByteArray();
        QFile avatarxmlfile(outname.c_str());
        if (avatarxmlfile.open(QFile::WriteOnly))
        {
            avatarxmlfile.write(bytes);
            avatarxmlfile.close();
        }
        else
            AvatarModule::LogError("Could not save avatar description file " + outname);
        
        // Get assets & dump them all
        AvatarExporterRequestPtr request(new AvatarExporterRequest());
        GetAvatarAssetsForExport(request, *appearance, false);
        ExportAssetMap::const_iterator i = request->assets_.begin();
        while (i != request->assets_.end())
        {
            // Replace problematic chars in asset names
            std::string filename = i->first;
            ReplaceCharInplace(filename, '/', '_');
            ReplaceCharInplace(filename, '\\', '_');
            filename = dirname + "/" + filename;
            
            QFile assetfile(filename.c_str());
            QByteArray assetbytes((const char*)&i->second.data_[0], i->second.data_.size());
            if (assetfile.open(QFile::WriteOnly))
            {
                assetfile.write(assetbytes);
                assetfile.close();
            }
            else
                AvatarModule::LogError("Could not save avatar asset " + filename);
            ++i;
        }
    }
    
    void AvatarAppearance::GetAvatarAssetsForExport(AvatarExporterRequestPtr request, EC_AvatarAppearance& appearance, bool inventorymode)
    {
        AvatarModule::LogDebug("Getting mesh for export");
        GetAvatarAssetForExport(request, appearance.GetMesh(), false, inventorymode);
        AvatarModule::LogDebug("Getting skeleton for export");
        GetAvatarAssetForExport(request, appearance.GetSkeleton(), false, inventorymode);

        AvatarModule::LogDebug("Getting materials for export");
        AvatarMaterialVector materials = appearance.GetMaterials();
        for (uint i = 0; i < materials.size(); ++i)
        {
            GetAvatarMaterialForExport(request, materials[i], inventorymode);
        }
        
        AvatarModule::LogDebug("Getting attachments for export");
        AvatarAttachmentVector attachments = appearance.GetAttachments();
        for (uint i = 0; i < attachments.size(); ++i)
        {
            GetAvatarAssetForExport(request, attachments[i].mesh_);
            for (uint j = 0; j < attachments[i].materials_.size(); ++j)
            {
                GetAvatarMaterialForExport(request, attachments[i].materials_[j]);
            }
        }
    }
    
    bool AvatarAppearance::GetAvatarMaterialForExport(AvatarExporterRequestPtr request, const AvatarMaterial& material, bool inventorymode)
    {
        boost::shared_ptr<OgreRenderer::Renderer> renderer = avatar_module_->GetFramework()->GetServiceManager()->
            GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (!renderer)
        {
            AvatarModule::LogError("Renderer does not exist");
            return false;
        }
        
        Ogre::MaterialManager& mat_mgr = Ogre::MaterialManager::getSingleton();
                
        std::string export_name = material.asset_.name_;
        if (export_name.find(".material") == std::string::npos)
            export_name.append(".material");

        Ogre::MaterialPtr clone;
        Ogre::MaterialPtr ogre_mat;

        // Resource-based or local?
        if (material.asset_.resource_.lock().get())
        {
            // In inventory mode, being resource based means it already exists on the server, do not store again
            if (inventorymode)
                return true;
            
            OgreRenderer::OgreMaterialResource* mat_res = dynamic_cast<OgreRenderer::OgreMaterialResource*>(material.asset_.resource_.lock().get());
            if (!mat_res)
            {
                AvatarModule::LogError("Material resource " + export_name + " was not valid");
                return false;
            }
            ogre_mat = mat_res->GetMaterial();
            if (ogre_mat.isNull())
            {
                AvatarModule::LogError("Material resource " + export_name + " could not be found");
                return false;
            }
        }
        else
        {
            ogre_mat = OgreRenderer::GetLocalMaterial(material.asset_.name_);
            if (ogre_mat.isNull())
            {
                AvatarModule::LogError("Material resource " + export_name + " could not be found");
                return false;
            }
        }
        
        // Create a clone of the material to be messed with during export
        std::string clonename = renderer->GetUniqueObjectName();
        clone = ogre_mat->clone(clonename);
        
        // Now remove unsupported techniques before exporting, or we will crash inside OGRE
        // This has, however, the side-effect that after exporting, the technique will not show for anyone!
        // (ie. for even those with better hardware)
        for (unsigned i = 0; i < clone->getNumTechniques(); ++i)
        {
            Ogre::Technique* tech = clone->getTechnique(i);
            if (tech)
            {
                if (!tech->isSupported())
                {
                    clone->removeTechnique(i);
                    --i;
                }
            }
        }
        // If number of techniques happened to drop to zero, just return
        if (!clone->getNumTechniques())
            return false;

        // Now ensure that the material has correct texture names
        // With non-local assets, we will get errors to Ogre log, but it does not really matter
        Ogre::Material::TechniqueIterator iter = clone->getTechniqueIterator();
        while(iter.hasMoreElements())
        {
            Ogre::Technique *tech = iter.getNext();
            assert(tech);
            Ogre::Technique::PassIterator passIter = tech->getPassIterator();
            while(passIter.hasMoreElements())
            {
                Ogre::Pass *pass = passIter.getNext();
                
                Ogre::Pass::TextureUnitStateIterator texIter = pass->getTextureUnitStateIterator();
                uint index = 0;
                
                while(texIter.hasMoreElements())
                {
                    Ogre::TextureUnitState *texUnit = texIter.getNext();
                    // Skip shadow textures
                    if (texUnit->getContentType() != Ogre::TextureUnitState::CONTENT_SHADOW)
                    {
                        if (index < material.textures_.size())
                            texUnit->setTextureName(ReplaceSpaces(material.textures_[index].name_));
                        index++;
                    }
                }
            }
        }
        
        // Export the clone
        Ogre::MaterialSerializer serializer;
        serializer.queueForExport(clone, true, false);
        std::string mat_string = serializer.getQueuedAsString();
        // Rename the exported material to the original name, so that we don't get lots of duplicates
        // to the storage with only the name changed
        std::string new_mat_name = ReplaceSubstring(export_name, ".material", "");
        ReplaceSubstringInplace(mat_string, "material " + clone->getName(), "material " + new_mat_name);
        
        if (request->assets_.find(export_name) == request->assets_.end())
        {
            ExportAsset new_export_asset;
            new_export_asset.data_.resize(mat_string.length());
            memcpy(&new_export_asset.data_[0], mat_string.c_str(), mat_string.length());
            new_export_asset.CalculateHash();
            // Check for hash duplicate
            bool duplicate = false;
            
            // In inventory mode, hashes are not used. No need to check
            if (!inventorymode)
            {
                ExportAssetMap::const_iterator i = request->assets_.begin();
                while (i != request->assets_.end())
                {
                    if (new_export_asset.hash_ == i->second.hash_)
                    {
                        AvatarModule::LogDebug("Skipping export of avatar asset " + export_name + ", has same hash as " + i->first);
                        duplicate = true;
                        break;
                    }
                    ++i;
                }
            }
            
            if (!duplicate)
                request->assets_[export_name] = new_export_asset;
        }
        else
        {
            AvatarModule::LogDebug("Skipping export of avatar asset " + export_name + ", same name already exists");
        }
        
        // Remove the clone
        clone.setNull();
        try
        {
            mat_mgr.remove(clonename);
        }
        catch (...) {}
        
        // Export textures used by material
        for (uint i = 0; i < material.textures_.size(); ++i)
            GetAvatarAssetForExport(request, material.textures_[i], true);
            
        return true;
    }
    
    bool AvatarAppearance::GetAvatarAssetForExport(AvatarExporterRequestPtr request, const AvatarAsset& asset, bool replace_spaces, bool inventorymode)
    {
        std::string export_name = asset.name_;
        // If name is empty, skip
        if (export_name.empty())
        {
            AvatarModule::LogDebug("Skipping unnamed asset");
            return true;
        }

        if (replace_spaces)
            export_name = ReplaceSpaces(export_name);

        // Skip if already exists with this name
        if (request->assets_.find(export_name) != request->assets_.end())
        {
            AvatarModule::LogDebug("Skipping export of avatar asset " + export_name + ", same name already exists");
            return true;
        }
        
        ExportAsset new_export_asset;
        
        // If it's loaded from resource, we should be able to get at the original raw asset data for export
        if (asset.resource_.lock().get())
        {
            // In inventory mode, being resource based means it already exists on the server, do not store again
            if (inventorymode)
                return true;        
        
            boost::shared_ptr<Foundation::AssetServiceInterface> asset_service =
                avatar_module_->GetFramework()->GetServiceManager()->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
            if (!asset_service)
            {
                AvatarModule::LogError("Could not get asset service");
                return false;
            }
            // The assettype doesn't matter here
            Foundation::AssetPtr raw_asset = asset_service->GetAsset(asset.resource_id_, std::string());
            if (!raw_asset)
            {
                AvatarModule::LogError("Could not get raw asset data for resource " + asset.resource_id_);
                return false;
            }
            if (raw_asset->GetSize())
            {
                new_export_asset.data_.resize(raw_asset->GetSize());
                memcpy(&new_export_asset.data_[0], raw_asset->GetData(), raw_asset->GetSize());
            }
            else
            {
                AvatarModule::LogError("Zero size data for avatar asset " + asset.name_);
                return false;
            }
        }
        else
        {
            // If it's a local resource, get data directly from Ogre
            try
            {
                Ogre::DataStreamPtr data = Ogre::ResourceGroupManager::getSingleton().openResource(asset.name_);
                uint size = data->size();
                if (size)
                {
                    new_export_asset.data_.resize(size);
                    data->read(&new_export_asset.data_[0], size);
                }
                else
                {
                    AvatarModule::LogError("Zero size data for local avatar asset " + asset.name_);
                    return false;
                }
            }
            catch (Ogre::Exception e)
            {
                AvatarModule::LogError("Could not get local avatar asset " + asset.name_ + " for export");
                return false;
            }
        }
        
        new_export_asset.CalculateHash();
        
        // Check for hash duplicate, only when not in inventory mode
        if (!inventorymode)
        {
            ExportAssetMap::const_iterator i = request->assets_.begin();
            while (i != request->assets_.end())
            {
                if (new_export_asset.hash_ == i->second.hash_)
                {
                    AvatarModule::LogDebug("Skipping export of avatar asset " + export_name + ", has same hash as " + i->first);
                    return true;
                }
                ++i;
            }
        } 
        
        AvatarModule::LogDebug("Added export asset " + export_name);
        request->assets_[export_name] = new_export_asset;
        return true;
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
                    AvatarModule::LogInfo("Avatar exported successfully");
                    // Send information of appearance change
                    ProtocolUtilities::WorldStreamPtr conn = avatar_module_->GetServerConnection();
                    if (conn)
                    {
                        std::string method = "RexAppearance";
                        StringVector strings;
                        conn->SendGenericMessage(method, strings);
                    }
                }
                else
                    AvatarModule::LogInfo("Avatar export failed: " + result->message_);
                
                avatar_exporter_.reset();
            }
        }
    }
    
    bool AvatarAppearance::LoadAppearance(Scene::EntityPtr entity, const std::string& filename)
    {
        boost::filesystem::path path(filename);
        std::string dirname = path.branch_path().string();
        
        if (!entity)
            return false;
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return false;
        
        if (filename.find(".mesh") != std::string::npos)
        {
            if (!PrepareAppearanceFromMesh(entity, filename))
                return false;
        }
        else
        {
            if (!PrepareAppearanceFromXml(entity, filename))
                return false;
        }
           
        // This whole operation is potentially evil
        try
        {            
            AddTempResourceDirectory(dirname);
            SetupAppearance(entity);
        }
        catch (Ogre::Exception& e)
        {
            AvatarModule::LogError("Error while loading avatar " + filename + ": " + e.what());
            return false;
        }
        
        return true;
    }
    
    bool AvatarAppearance::PrepareAppearanceFromXml(Scene::EntityPtr entity, const std::string& filename)
    {
        boost::filesystem::path path(filename);
        std::string leafname = path.leaf();
        
        if (!entity)
            return false;                             
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return false;        
                   
        QFile file(filename.c_str());
        QDomDocument avatar_doc("Avatar");
        
        if (!file.open(QIODevice::ReadOnly))
        {
            AvatarModule::LogError("Could not open avatar appearance file " + filename);
            return false;
        }
        
        if (!avatar_doc.setContent(&file))
        {
            file.close();
            AvatarModule::LogError("Could not parse avatar appearance file " + filename);
            return false;
        }
        file.close();
        
        if (!LegacyAvatarSerializer::ReadAvatarAppearance(*appearance, avatar_doc))
            return false;
            
        AvatarAsset mesh = appearance->GetMesh();
        // If mesh name is empty, deduce mesh name from filename
        if (mesh.name_.empty())
        {
            AvatarModule::LogInfo("Empty mesh name in avatar xml. Deducing from filename...");

            mesh.name_ = ReplaceSubstring(leafname, ".xml", ".mesh");
            appearance->SetMesh(mesh);
        }      
        
        return true;    
    }    
    
    bool AvatarAppearance::PrepareAppearanceFromMesh(Scene::EntityPtr entity, const std::string& filename)
    {   
        boost::filesystem::path path(filename);
        std::string leafname = path.leaf();
        
        if (!entity)
            return false;
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return false;
                
        appearance->Clear();
        AvatarAsset mesh;
        mesh.name_ = leafname;
        appearance->SetMesh(mesh);
        
        std::string xmlname = ReplaceSubstring(filename, ".mesh", ".xml");
        
        // Now optionally read parameters from an xml file that perhaps exists, but it's not fatal if it's not found
        QFile file(xmlname.c_str());
        QDomDocument avatar_doc("Avatar");
        
        if (!file.open(QIODevice::ReadOnly))
            return true;
        
        if (!avatar_doc.setContent(&file))
        {
            file.close();
            return true;
        }
        file.close();
        
        LegacyAvatarSerializer::ReadAvatarAppearance(*appearance, avatar_doc, false);
    
        return true;
    }    
    
    void AvatarAppearance::AddTempResourceDirectory(const std::string& dirname)
    {
        boost::shared_ptr<OgreRenderer::Renderer> renderer = avatar_module_->GetFramework()->GetServiceManager()->
            GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (!renderer)
        {
            AvatarModule::LogError("Renderer does not exist");
            return;
        }
        
        renderer->AddResourceDirectory(QString::fromStdString(dirname));
    }    
    
    bool AvatarAppearance::ChangeAvatarMaterial(Scene::EntityPtr entity, uint index, const std::string& filename)
    {
        boost::filesystem::path path(filename);
        std::string leafname = path.leaf();
        std::string dirname = path.branch_path().string();
                
        std::string matname;
        
        if (!entity)
            return false; 
                   
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        OgreRenderer::EC_OgreMesh* mesh = entity->GetComponent<OgreRenderer::EC_OgreMesh>().get();
        
        if (!mesh || !appearance)
            return false;       
        
        AddTempResourceDirectory(dirname);
        
        // New texture from material
        if (leafname.find(".material") != std::string::npos)
        {     
            // Note: we have to scan inside the material script to get the actual material name
            try
            {
                Ogre::DataStreamPtr data = Ogre::ResourceGroupManager::getSingleton().openResource(leafname);    
                while (!data->eof())
                {
                    Ogre::String line = data->getLine();
                    
                    // Skip empty lines & comments
                    if ((!line.length()) || (line.substr(0, 2) == "//"))
                        continue;
                        
                    std::size_t pos = line.find("material ");
                    if (pos == std::string::npos)
                        continue;
                    
                    // Use the first material name encountered
                    matname = line.substr(pos+9, line.length()-pos-9);
                    break;
                }
            }
            catch (...) {}

            if (matname.empty())
                return false;
        }
        // New texture from image
        else
        {
            boost::shared_ptr<OgreRenderer::Renderer> renderer = avatar_module_->GetFramework()->GetServiceManager()->
                GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
            if (!renderer)
            {
                AvatarModule::LogError("Renderer does not exist");
                return false;
            }
            
            matname = renderer->GetUniqueObjectName();
            
            //! \todo this temp material will not be deleted ever. Should delete it
            Ogre::MaterialPtr ogremat = OgreRenderer::GetOrCreateLitTexturedMaterial(matname);
            OgreRenderer::GetLocalTexture(leafname);
            OgreRenderer::SetTextureUnitOnMaterial(ogremat, leafname);
        }
        
        mesh->SetMaterial(index, matname);
        
        AvatarMaterialVector materials = appearance->GetMaterials();        
        if (index >= materials.size())
            return false;
        materials[index] = AvatarMaterial();
        materials[index].asset_.name_ = matname; 
        FixupMaterial(materials[index], AvatarAssetMap());
        appearance->SetMaterials(materials);
        
        Scene::Events::EntityEventData event_data;
        event_data.entity = entity;
        Foundation::EventManagerPtr event_manager = avatar_module_->GetFramework()->GetEventManager();
        event_manager->SendEvent("Scene", Scene::Events::EVENT_ENTITY_VISUALS_MODIFIED, &event_data);
                
        return true;
    }        
    
    bool AvatarAppearance::AddAttachment(Scene::EntityPtr entity, const std::string& filename)
    {
        boost::filesystem::path path(filename);
        std::string leafname = path.leaf();
        std::string dirname = path.branch_path().string();
                
        std::string matname;
        
        if (!entity)
            return false; 
                   
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        OgreRenderer::EC_OgreMesh* mesh = entity->GetComponent<OgreRenderer::EC_OgreMesh>().get();
        
        if (!mesh || !appearance)
            return false;    
  
        QFile file(filename.c_str());
        QDomDocument attachment_doc("Attachment");
        
        if (!file.open(QIODevice::ReadOnly))
        {
            AvatarModule::LogError("Could not open attachment description file " + filename);
            return false;
        }
        
        if (!attachment_doc.setContent(&file))
        {
            file.close();
            AvatarModule::LogError("Could not parse attachment description file " + filename);
            return false;
        }
        file.close();  
        
        AvatarAttachment new_attachment;
        if (!LegacyAvatarSerializer::ReadAttachment(new_attachment, attachment_doc, *appearance, leafname))
            return false;
        
        AvatarAttachmentVector attachments = appearance->GetAttachments();
        attachments.push_back(new_attachment);
        appearance->SetAttachments(attachments);
         
        // Assume any resources needed by the attachment are in the same dir as the .xml      
        AddTempResourceDirectory(dirname);
        
        SetupAppearance(entity);
        return true;
    }         
}
