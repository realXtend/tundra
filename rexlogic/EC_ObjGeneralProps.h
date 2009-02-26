#ifndef __incl_EC_ObjGeneralProps_h__
#define __incl_EC_ObjGeneralProps_h__

#include "ComponentInterface.h"
#include "Foundation.h"


class EC_ObjGeneralProps : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_ObjGeneralProps);
public:
    virtual ~EC_ObjGeneralProps();

    virtual void handleNetworkData(std::string data);

    static std::vector<std::string> getNetworkMessages()
    {
        std::vector<std::string> myinterest;
        myinterest.push_back("ObjectUpdate");
        return myinterest;
    } 

private:
    EC_ObjGeneralProps();

    std::string mName;
    std::string mDescription;
    std::string mCreator;
    std::string mOwner;
    std::string mGroup;
    
    bool mbShareWithGroup;
    bool mbAllowAnyOneToMove;
    bool mbAllowAnyOneToCopy;
    bool mbShowInSearch;
    
    bool mbForSale;
    int mSalePrice;
    int mSaleType;
    
    bool mbNextOwnerCanModify;
    bool mbNextOwnerCanCopy;
    bool mbNextOwnerCanResell;
    
    Core::uchar mLeftClickAction;
};

#endif