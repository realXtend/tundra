// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_OpenSimAvatar_h
#define incl_EC_OpenSimAvatar_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "RexUUID.h"

namespace RexLogic
{
    //! This component is present on all avatars when connected to an
    //! OpenSim world. 
    class EC_OpenSimAvatar : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_OpenSimAvatar);
       
    public:
        virtual ~EC_OpenSimAvatar();

        //! set first name
        void SetFirstName(const std::string &name);    
        //! get first name
        std::string GetFirstName();  
        //! set last name
        void SetLastName(const std::string &name);    
        //! get last name
        std::string GetLastName(); 
        //! get full name
        std::string GetFullName(); 

        //! set appearance address 
        void SetAppearanceAddress(const std::string &address, bool overrideappearance);
        // get appearance address that is used, return override if that is defined, otherwise the default address
        std::string GetAppearanceAddress();

     
        // !ID related
        uint64_t RegionHandle;
        uint32_t LocalId;
        RexTypes::RexUUID FullId;
        uint32_t ParentId;
                
    private:
        //! first name of avatar
        std::string first_name_;
        //! last name of avatar
        std::string last_name_;

        //! appearance address
        std::string avatar_address_;
        //! appearance override address
        std::string avatar_override_address_;

        EC_OpenSimAvatar(Foundation::ModuleInterface* module);
    };
}

#endif