// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarAppearance.h"
#include "LegacyAvatarSerializer.h"
#include "RexLogicModule.h"
#include "EC_AvatarAppearance.h"
#include "SceneManager.h"
#include "../OgreRenderingModule/EC_OgreMesh.h"
#include "../OgreRenderingModule/OgreMaterialResource.h"
#include "../OgreRenderingModule/OgreMaterialUtils.h"
#include "../OgreRenderingModule/Renderer.h"

#include <QDomDocument>
#include <QFile>

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
    
    void AvatarAppearance::SetupAppearance(Scene::EntityPtr entity)
    {
        if (!entity)
            return;
        
        Foundation::ComponentPtr meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        if (meshptr && appearanceptr)
        {
            OgreRenderer::EC_OgreMesh &mesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(meshptr.get());
            EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(appearanceptr.get());
            
            // Deserialize appearance from the document into the EC
            LegacyAvatarSerializer::ReadAvatarAppearance(appearance, *default_appearance_);
            
            // Setup appearance
            SetupMeshAndMaterials(entity);
            SetupMorphs(entity);
        }
    }
    
    void AvatarAppearance::SetupMeshAndMaterials(Scene::EntityPtr entity)
    {
        OgreRenderer::EC_OgreMesh &mesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic()).get());
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(entity->GetComponent(EC_AvatarAppearance::NameStatic()).get());
        
        // Setup mesh
        //! \todo use mesh resource
        mesh.SetMesh(appearance.GetMesh().name_, entity.get());
        
        // Arbitrary materials would cause problems, because we really would want avatar materials to use the SuperShader style materials for
        // proper shadowing. For now, we force all materials to be based on LitTextured shader material
        
        AvatarMaterialVector materials = appearance.GetMaterials();
        for (Core::uint i = 0; i < materials.size(); ++i)
        {
            //! \todo handle multitextured materials, for now only one used (avatar generator only uses one texture per material)
            //! \todo use material/texture resources
            
            // See if a texture is specified, if not, assume default
            if (materials[i].textures_.size())
            {
                AvatarAsset& texture = materials[i].textures_[0];
                if (!texture.name_.empty())
                {
                    // Create a new temporary material resource for texture override. Should be deleted when the appearance EC is deleted
                    boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->
                        GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
                    
                    Ogre::MaterialPtr override_mat = OgreRenderer::GetOrCreateLitTexturedMaterial(renderer->GetUniqueObjectName().c_str());
                    materials[i].asset_.resource_ = OgreRenderer::CreateResourceFromMaterial(override_mat);
                    
                    // Load local texture if not yet loaded
                    //! \todo remove once local avatar resources are not needed anymore
                    if (texture.id_.empty())
                    {
                        Ogre::TextureManager& tex_man = Ogre::TextureManager::getSingleton();
                        Ogre::TexturePtr tex = tex_man.getByName(texture.name_);
                        if (tex.isNull())
                            tex_man.load(texture.name_, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
                        OgreRenderer::SetTextureUnitOnMaterial(override_mat, texture.name_);
                    }
                    
                    mesh.SetMaterial(i, override_mat->getName());
                }
            }
        }
        
        // Store the modified materials vector (with created temp resources) to the EC
        appearance.SetMaterials(materials);
        
        // Set adjustment orientation for mesh (Ogre meshes usually have Y-axis as vertical)
        Core::Quaternion adjust(Core::PI/2, 0, -Core::PI/2);
        mesh.SetAdjustOrientation(adjust);
        //! \todo use bones/properties for positioning adjustment
        // Position approximately within the bounding box
        mesh.SetAdjustPosition(Core::Vector3df(0.0f, 0.0f, -0.8f));
        mesh.SetCastShadows(true);
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
            }
        }
    }
}