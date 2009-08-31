// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_AvatarAppearance_h
#define incl_RexLogic_AvatarAppearance_h

#include "EC_AvatarAppearance.h"

class QDomDocument;

namespace RexLogic
{
    class RexLogicModule;
    
    //! Handles setting up and updating avatars' appearance. Used internally by RexLogicModule::Avatar.
    class AvatarAppearance
    {
    public:
        AvatarAppearance(RexLogicModule *rexlogicmodule);
        ~AvatarAppearance();
        
        //! Reads default appearance of avatar from file to xml document
        void ReadDefaultAppearance(const std::string& filename);
        
        //! Sets up an avatar entity's appearance
        void SetupAppearance(Scene::EntityPtr entity);
        
    private:
        //! Sets up an avatar mesh
        void SetupMeshAndMaterials(Scene::EntityPtr entity);
        
        //! Default avatar appearance xml document
        boost::shared_ptr<QDomDocument> default_appearance_;
        
        RexLogicModule *rexlogicmodule_;
    };
}

#endif
