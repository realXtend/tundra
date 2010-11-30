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
        FrameworkEventHandler(ProtocolUtilities::WorldStream *connection, RexLogicModule *rexLogic) :
            connection_(connection), rexLogic_(rexLogic) {}
        ~FrameworkEventHandler() {}

        //! handle framework event
        bool HandleFrameworkEvent(event_id_t event_id, IEventData* data);

		QString GetAvatarAddress() { return avatar_address_; };
		void SetAvatarAddress(QString avatar_address) { avatar_address_ = avatar_address; };

    private:
        //! server connection
        ProtocolUtilities::WorldStream *connection_;
        //! owner module
        RexLogicModule *rexLogic_;

		QString avatar_address_;

    };
}

#endif
