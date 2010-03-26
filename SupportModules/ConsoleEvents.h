// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SupportModules_ConsoleEvents_h
#define incl_SupportModules_ConsoleEvents_h

//#include "StableHeaders.h"
#include "EventDataInterface.h"

namespace Console
{
    namespace Events
    {
        static const event_id_t EVENT_CONSOLE_TOGGLE = 0x01;

        static const event_id_t EVENT_CONSOLE_PRINT_LINE = 0x02;

        static const event_id_t EVENT_CONSOLE_COMMAND_ISSUED = 0x03;

        static const event_id_t EVENT_CONSOLE_CONSOLE_VIEW_INITIALIZED = 0x04;
    }

    /// Event data interface for Consolemessages
    class ConsoleEventData : public Foundation::EventDataInterface
    {
    public:
        ConsoleEventData(const std::string string) :
            message(string) {}
        virtual ~ConsoleEventData() {}
        std::string message;
        
    };
}

#endif
