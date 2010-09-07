// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraLogicModule_TundraEvents_h
#define incl_TundraLogicModule_TundraEvents_h

#include "EventDataInterface.h"

namespace TundraLogic
{

namespace Events
{
    //! Attempt Tundra login with the specified server address, port, and credentials. TundraLoginEventData structure.
    static const event_id_t EVENT_TUNDRA_LOGIN = 0x01;
    
    //! Tundra connection & authentication made successfully, and client scene has been created. TundraConnectedEventData structure.
    static const event_id_t EVENT_TUNDRA_CONNECTED = 0x02;
    
    //! Tundra disconnected. Deletion of client scene imminent. No eventdata structure.
    static const event_id_t EVENT_TUNDRA_DISCONNECTED = 0x03;
    
    //! Tundra connection or login failed. No eventdata structure.
    static const event_id_t EVENT_TUNDRA_LOGIN_FAILED = 0x04;
    
    class TundraLoginEventData : public Foundation::EventDataInterface
    {
    public:
        std::string address_;
        unsigned short port_;
        std::string username_;
        std::string password_;
    };
    
    class TundraConnectedEventData : public Foundation::EventDataInterface
    {
    public:
        u8 user_id_;
    };
}

}

#endif
