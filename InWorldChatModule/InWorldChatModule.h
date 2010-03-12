/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InWorldChatModule.h
 *  @brief  Simple OpenSim world chat module. Listens for ChatFromSimulator packets and shows the chat on the UI.
 *          Outgoing chat sent using ChatFromViewer packets. Manages EC_ChatBubbles
 *  @note   Depends on RexLogicModule so don't create dependency to this module.
 */

#ifndef incl_InWorldChatModule_InWorldChatModule_h
#define incl_InWorldChatModule_InWorldChatModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include "ConsoleCommandServiceInterface.h"

#include <QObject>

class RexUUID;

namespace ProtocolUtilities
{
    class NetInMessage;
}

namespace Foundation
{
    class EventDataInterface;
}

namespace ProtocolUtilities
{
    class ProtocolModuleInterface;
    class WorldStream;
    typedef boost::weak_ptr<ProtocolModuleInterface> ProtocolWeakPtr;
    typedef boost::shared_ptr<WorldStream> WorldStreamPtr;
}

namespace UiServices
{
    class UiModule;
}

namespace Naali
{
    class InWorldChatModule :  public QObject, public Foundation::ModuleInterfaceImpl
    {
        Q_OBJECT

    public:
        /// Default constructor.
        InWorldChatModule();

        /// Destructor 
        ~InWorldChatModule();

        /// ModuleInterfaceImpl overrides.
        void Load();
        void PostInitialize();
        void Update(f64 frametime);
        bool HandleEvent(
            event_category_id_t category_id,
            event_id_t event_id,
            Foundation::EventDataInterface* data);

        void SubscribeToNetworkEvents(ProtocolUtilities::ProtocolWeakPtr currentProtocolModule);

        MODULE_LOGGING_FUNCTIONS

        /// Returns name of this module. Needed for logging.
        static const std::string &NameStatic();

        /// Name of this module.
        static const std::string moduleName;

    public slots:
        /// Sends chat message to server.
        /// @param msg Chat message to be sent.
        void SendChatFromViewer(const QString &msg);

    private:
        Q_DISABLE_COPY(InWorldChatModule);

        /// Console command for testing billboards.
        /// @param params Parameters.
        Console::CommandResult TestAddBillboard(const StringVector &params);

        /// Console command for sending chat message to server.
        /// @param params Parameters.
        Console::CommandResult ConsoleChat(const StringVector &params);

        /// Applies EC_ChatBubble to a scene entity.
        /// @param entity Entity.
        /// @message Message to be shown at the chat bubble.
        void ApplyChatBubble(Scene::Entity &entity, const QString &message);

        /// Applies EC_ChatBubble to a scene entity.
        /// @param entity Entity.
        /// @param texture
        /// @param timeToShow 
        void ApplyBillboard(Scene::Entity &entity, const std::string &texture, float timeToShow);

        /// Returns scene entity pointer with the wanter ID:
        /// @param id ID of the entity.
        /// @return Entity pointer matching the id or 0 if not found.
        Scene::Entity *GetEntityWithID(const RexUUID &id);

        /// Handles RexEmotionIcon generic message.
        /// @param params Parameters.
        void HandleRexEmotionIconMessage(StringVector &params);

        /// Handles ChatFromSimulator message.
        /// @param msg Network message.
        void HandleChatFromSimulatorMesage(ProtocolUtilities::NetInMessage &msg);

        /// NetworkState event category.
        event_category_id_t networkStateEventCategory_;

        /// NetworkIn event category.
        event_category_id_t networkInEventCategory_;

        /// Framework event category
        event_category_id_t frameworkEventCategory_;

        /// WorldStream pointer
        ProtocolUtilities::WorldStreamPtr currentWorldStream_ ;

        /// UiModule pointer.
        boost::weak_ptr<UiServices::UiModule> uiModule_;
    };
}

#endif
