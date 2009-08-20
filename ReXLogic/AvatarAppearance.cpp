// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarAppearance.h"
#include "LegacyAvatarSerializer.h"
#include "RexLogicModule.h"
#include "EC_AvatarAppearance.h"
#include "SceneManager.h"
#include "../OgreRenderingModule/EC_OgreMesh.h"

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
            
            mesh.SetMesh(appearance.GetMesh().name_, entity.get());
            // Set adjustment orientation for mesh (Ogre meshes usually have Y-axis as vertical)
            Core::Quaternion adjust(Core::PI/2, 0, -Core::PI/2);
            mesh.SetAdjustOrientation(adjust);
            // Position approximately within the bounding box
            mesh.SetAdjustPosition(Core::Vector3df(0.0f, 0.0f, -0.8f));
            mesh.SetCastShadows(true);
        }
    }
}