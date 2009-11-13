// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_FrameworkEventHandler_h
#define incl_FrameworkEventHandler_h


namespace RexLogic
{
    class RexServerConnection;
    class RexLogicModule;

    //! Handles framework events
    class FrameworkEventHandler
    {
    public:
        FrameworkEventHandler(RexServerConnection *connection, Foundation::Framework *framework, RexLogicModule *rexLogic)
            : connection_(connection), framework_(framework), rexLogic_(rexLogic) {}

        //! handle framework event
        bool HandleFrameworkEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);
    private:
        //! server connection
        RexServerConnection *connection_;
        RexLogicModule *rexLogic_;
        Foundation::Framework *framework_;
    };
}

#endif
