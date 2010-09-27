/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   NetworkEventEventHandler.h
 *  @brief  Handles incoming SLUDP network events (messages) in a reX-specific way.
 */

#ifndef incl_RexLogicModule_NetworkEventHandler_h
#define incl_RexLogicModule_NetworkEventHandler_h

namespace ProtocolUtilities
{
    class ProtocolModuleInterface;
    class NetworkEventInboundData;
}

namespace RexLogic
{
    class RexLogicModule;
    class ScriptDialogHandler;
    typedef boost::shared_ptr<ScriptDialogHandler> ScriptDialogHandlerPtr;

    namespace InWorldChat
    {
        class Provider;
    }
    typedef boost::shared_ptr<InWorldChat::Provider> InWorldChatProviderPtr;

    /// Handles incoming SLUDP network events (messages) in a reX-specific way.
    class NetworkEventHandler
    {
    public:
        //! Constructor
        //! \param owner Owner module.
        explicit NetworkEventHandler(RexLogicModule *owner);

        //! Destructor.
        virtual ~NetworkEventHandler();

        // !Handle network events coming from OpenSimProtocolModule
        bool HandleOpenSimNetworkEvent(event_id_t event_id, IEventData *data);

    private:
        //! Handles AgentMovementComplete network message.
        //! \param data Network event data.
        bool HandleOSNE_AgentMovementComplete(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles ImprovedTerseObjectUpdate network message.
        //! \param data Network event data.
        bool HandleOSNE_ImprovedTerseObjectUpdate(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles KillObject network message.
        //! \param data Network event data.
        bool HandleOSNE_KillObject(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles LogoutReply network message.
        //! \param data Network event data.
        bool HandleOSNE_LogoutReply(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles ObjectUpdate network message.
        //! \param data Network event data.
        bool HandleOSNE_ObjectUpdate(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles RegionHandshake network message.
        //! \param data Network event data.
        bool HandleOSNE_RegionHandshake(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles SoundTrigger network message.
        //! \param data Network event data.
        bool HandleOSNE_SoundTrigger(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles PreloadSound network message.
        //! \param data Network event data.
        bool HandleOSNE_PreloadSound(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles ScriptDialog network message.
        //! \param data Network event data.
        bool HandleOSNE_ScriptDialog(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles LoadURL network message.
        //! \param data Network event data.
        bool HandleOSNE_LoadURL(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles MapBlock network message.
        //! \param data Network event data.
        bool HandleOSNE_MapBlock(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles ScriptTeleport network message.
        //! \param data Network event data.
        bool HandleOSNE_ScriptTeleport(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles ChatFromSimulator network message.
        //! \param data Network event data.
        bool HandleOSNE_ChatFromSimulator(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles ImprovedInstantMessage network message.
        //! \param data Network event data.
        bool HandleOSNE_ImprovedInstantMessage(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles OnlineNotification network message.
        //! \param data Network event data.
        bool HandleOSNE_OnlineNotification(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles OfflineNotification network message.
        //! \param data Network event data.
        bool HandleOSNE_OfflineNotification(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles TerminateFriendship network message.
        //! \param data Network event data.
        bool HandleOSNE_TerminateFriendship(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles DeclineFriendship network message.
        //! \param data Network event data.
        bool HandleOSNE_DeclineFriendship(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles KickUser network message.
        //! \param data Network event data.
        bool HandleOSNE_KickUser(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles EstateOwnerMessage network message.
        //! \param data Network event data.
        bool HandleOSNE_EstateOwnerMessage(ProtocolUtilities::NetworkEventInboundData *data);

        //! Handles GenericMessage network message.
        //! \param data Network event data.
        bool HandleOSNE_GenericMessage(ProtocolUtilities::NetworkEventInboundData *data);

        //! Owner module.
        RexLogicModule *owner_;

        ScriptDialogHandlerPtr script_dialog_handler_; /// @todo: Move to RexLogic module
        bool ongoing_script_teleport_;
    };
}

#endif
