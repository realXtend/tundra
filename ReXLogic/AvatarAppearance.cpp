// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarAppearance.h"
#include "LegacyAvatarSerializer.h"
#include "RexLogicModule.h"
#include "EC_AvatarAppearance.h"
#include "SceneManager.h"
#include "../OgreRenderingModule/EC_OgreMesh.h"

namespace RexLogic
{
    AvatarAppearance::AvatarAppearance(RexLogicModule *rexlogicmodule) :
        rexlogicmodule_(rexlogicmodule)
    {
        default_avatar_mesh_ = rexlogicmodule_->GetFramework()->GetDefaultConfig().DeclareSetting("RexAvatar", "default_mesh_name", std::string("Jack.mesh"));
        std::string default_animation_path = rexlogicmodule_->GetFramework()->GetDefaultConfig().DeclareSetting("RexAvatar", "default_animations_file", std::string("./data/default_animations.xml"));
        
        // Read default animation definitions
        LegacyAvatarSerializer::ReadAnimationDefinitions(default_animations_, default_animation_path);
    }

    AvatarAppearance::~AvatarAppearance()
    {
    }

    void AvatarAppearance::SetupAppearance(Scene::EntityPtr entity)
    {
        if (!entity)
            return;
            
        Foundation::ComponentPtr meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
        if (meshptr)
        {
            OgreRenderer::EC_OgreMesh &mesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(meshptr.get());
            
            mesh.SetMesh(default_avatar_mesh_, entity.get());
            // Set adjustment orientation for mesh (Ogre meshes usually have Y-axis as vertical)
            Core::Quaternion adjust(Core::PI/2, 0, -Core::PI/2);
            mesh.SetAdjustOrientation(adjust);
            // Position approximately within the bounding box
            mesh.SetAdjustPosition(Core::Vector3df(0.0f, 0.0f, -0.8f));
            mesh.SetCastShadows(true);
        }
        
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        if (appearanceptr)
        {
            EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(appearanceptr.get());
            appearance.SetAnimations(default_animations_);
        }
    }
}