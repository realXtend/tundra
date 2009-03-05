// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_ObjGeneralProps_h
#define incl_EC_ObjGeneralProps_h

#include "ComponentInterface.h"
#include "Foundation.h"


class EC_ObjGeneralProps : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_ObjGeneralProps);
public:
    virtual ~EC_ObjGeneralProps();

    virtual void HandleNetworkData(std::string data);

    static std::vector<std::string> GetNetworkMessages()
    {
        std::vector<std::string> myinterest;
        myinterest.push_back("ObjectUpdate");
        return myinterest;
    } 

private:
    EC_ObjGeneralProps();

    std::string name_;
    std::string description_;
    std::string creator_;
    std::string owner_;
    std::string group_;
    
    bool b_share_with_group_;
    bool b_allow_any_one_to_move_;
    bool b_allow_any_one_to_copy_;
    bool b_show_in_search_;
    
    bool b_for_sale_;
    int sale_price_;
    int sale_type_;
    
    bool b_next_owner_can_modify_;
    bool b_next_owner_can_copy_;
    bool b_next_owner_can_resell_;
    
    Core::uchar left_click_action_;
};

#endif