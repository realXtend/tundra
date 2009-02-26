#include "StableHeaders.h"
#include "EC_ObjGeneralProps.h"

EC_ObjGeneralProps::EC_ObjGeneralProps()
{
    mName = "";
    mDescription = "";
    mCreator = "";
    mOwner = "";
    mGroup = "";
    
    mbShareWithGroup = false;
    mbAllowAnyOneToMove = false;
    mbAllowAnyOneToCopy = false;
    mbShowInSearch = false;
    
    mbForSale = false;
    mSalePrice = 0;
    mSaleType = 0;
    
    mbNextOwnerCanModify = false;
    mbNextOwnerCanCopy = false;
    mbNextOwnerCanResell = false;
    
    mLeftClickAction = 0;
}

EC_ObjGeneralProps::~EC_ObjGeneralProps()
{
}

void EC_ObjGeneralProps::handleNetworkData(std::string data) 
{
    // fixme, implement, set properties based on data
    mName = data.substr(0,1); // fixme, get name from real data
    mDescription = data.substr(1,2); // fixme, get description from real data
    // and so on...
    
}
