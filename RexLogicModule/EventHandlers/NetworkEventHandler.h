// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_NetworkEventHandler_h
#define incl_NetworkEventHandler_h

#include "ComponentInterface.h"
#include "NetworkEvents.h"

namespace ProtocolUtilities
{
    class ProtocolModuleInterface;
}

namespace RexLogic
{
    struct DecodedTerrainPatch;
    class RexLogicModule;
    class ScriptDialogHandler;
    typedef boost::shared_ptr<ScriptDialogHandler> ScriptDialogHandlerPtr;
 
    /// Handles incoming SLUDP network events in a reX-specific way. \todo Break down into more logical functions.
    class NetworkEventHandler
    {
    public:
        explicit NetworkEventHandler(RexLogicModule *rexlogicmodule);
        virtual ~NetworkEventHandler();

        // !Handle network events coming from OpenSimProtocolModule
        bool HandleOpenSimNetworkEvent(event_id_t event_id, Foundation::EventDataInterface *data);
    private:
        // !Handler functions for Opensim network events
        bool HandleOSNE_AgentMovementComplete(ProtocolUtilities::NetworkEventInboundData *data);
        bool HandleOSNE_ImprovedTerseObjectUpdate(ProtocolUtilities::NetworkEventInboundData *data);
        bool HandleOSNE_KillObject(ProtocolUtilities::NetworkEventInboundData *data);
        bool HandleOSNE_LogoutReply(ProtocolUtilities::NetworkEventInboundData *data);
        bool HandleOSNE_ObjectUpdate(ProtocolUtilities::NetworkEventInboundData *data);
        bool HandleOSNE_RegionHandshake(ProtocolUtilities::NetworkEventInboundData *data);
        bool HandleOSNE_SoundTrigger(ProtocolUtilities::NetworkEventInboundData *data);
        bool HandleOSNE_PreloadSound(ProtocolUtilities::NetworkEventInboundData *data);
        bool HandleOSNE_ScriptDialog(ProtocolUtilities::NetworkEventInboundData *data);       
        bool HandleOSNE_LoadURL(ProtocolUtilities::NetworkEventInboundData *data);
        bool HandleOSNE_MapBlock(ProtocolUtilities::NetworkEventInboundData *data);
        
        //! Handler functions for GenericMessages
        bool HandleOSNE_GenericMessage(ProtocolUtilities::NetworkEventInboundData *data);

        void DebugCreateTerrainVisData(const DecodedTerrainPatch &heightData, int patchSize);

        boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> protocolModule_;

        RexLogicModule *rexlogicmodule_;

        ScriptDialogHandlerPtr script_dialog_handler_;

    };
}

#endif
