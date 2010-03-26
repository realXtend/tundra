// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_FrameworkEventHandler_h
#define incl_FrameworkEventHandler_h

namespace ProtocolUtilities
{
    class WorldStream;
}

namespace RexLogic
{
    class RexLogicModule;

    //! Handles framework events
    class FrameworkEventHandler
    {
    public:
        FrameworkEventHandler(ProtocolUtilities::WorldStream *connection, Foundation::Framework *framework, RexLogicModule *rexLogic)
            : connection_(connection), framework_(framework), rexLogic_(rexLogic) {}

        //! handle framework event
        bool HandleFrameworkEvent(event_id_t event_id, Foundation::EventDataInterface* data);
    private:
        //! server connection
        ProtocolUtilities::WorldStream *connection_;
        RexLogicModule *rexLogic_;
        Foundation::Framework *framework_;
    };
}

#endif
