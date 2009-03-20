// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_ObjGeneralProps.h"

namespace RexLogic
{
    EC_ObjGeneralProps::EC_ObjGeneralProps(Foundation::ModuleInterface* module)
    {
        name_ = "";
        description_ = "";
        creator_ = "";
        owner_ = "";
        group_ = "";
        
        b_share_with_group_ = false;
        b_allow_any_one_to_move_ = false;
        b_allow_any_one_to_copy_ = false;
        b_show_in_search_ = false;
        
        b_for_sale_ = false;
        sale_price_ = 0;
        sale_type_ = 0;
        
        b_next_owner_can_modify_ = false;
        b_next_owner_can_copy_ = false;
        b_next_owner_can_resell_ = false;
        
        left_click_action_ = 0;
    }

    EC_ObjGeneralProps::~EC_ObjGeneralProps()
    {
    }

    void EC_ObjGeneralProps::HandleNetworkData(std::string data) 
    {
        // fixme, implement, set properties based on data
        name_ = data.substr(0,1); // fixme, get name from real data
        description_ = data.substr(1,2); // fixme, get description from real data
        // and so on...
        
    }
}