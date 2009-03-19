// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_SpatialSound_h
#define incl_EC_SpatialSound_h

#include "ComponentInterface.h"
#include "Foundation.h"


class EC_SpatialSound : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_SpatialSound);
public:
    virtual ~EC_SpatialSound();


    virtual void HandleNetworkData(std::string data);

    static std::vector<std::string> GetNetworkMessages()
    {
        std::vector<std::string> myinterest;
        myinterest.push_back("GeneralMessage_ExtraEntityData");
        return myinterest;
    } 

private:
    EC_SpatialSound(Foundation::ModuleInterface* module);

    std::string sound_id_;
    float volume_;
    float radius_;
};

#endif