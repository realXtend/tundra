/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InWorldChatModule.h
 *  @brief  Simple OpenSim world chat module. Listens for ChatFromSimulator packets and shows the chat on the UI.
 *          Outgoing chat sent using ChatFromViewer packets. Manages EC_ChatBubbles, EC_Billboards, chat logging etc.
 */

#ifndef incl_InWorldChatModule_InWorldChatModule_h
#define incl_InWorldChatModule_InWorldChatModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"

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

QT_BEGIN_NAMESPACE
class QColor;
class QFile;
QT_END_NAMESPACE

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
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

        MODULE_LOGGING_FUNCTIONS

        /// Returns name of this module. Needed for logging.
        static const std::string &NameStatic();

        /// Name of this module.
        static const std::string moduleName;

    public slots:
        /// Sends chat message to server.
        /// @param msg Chat message to be sent.
        void SendChatFromViewer(const QString &msg);

        /// Shows VOIP activity indicator above user avatar.
        /// @param id AgentID of the user.
        /// @param visibility Do we want icon to to be shown or hidden.
        void ShowUserVoipActivityIcon(const RexUUID &id, const bool visibility);

    private:
        Q_DISABLE_COPY(InWorldChatModule);

        /// Console command for testing billboards.
        /// @param params Parameters.
        Console::CommandResult TestAddBillboard(const StringVector &params);

        /// Console command for sending chat message to server.
        /// @param params Parameters.
        Console::CommandResult ConsoleChat(const StringVector &params);

        /// Applies EC_ChatBubble to a scene entity with default parameters.
        /// @param entity Entity.
        /// @message Message to be shown at the chat bubble.
        void ApplyDefaultChatBubble(Scene::Entity &entity, const QString &message);

        /// Applies EC_ChatBubble to a scene entity.
        /// @param entity Entity.
        /// @param texture
        /// @param timeToShow 
        void ApplyBillboard(Scene::Entity &entity, const std::string &texture, float timeToShow);

        /// Returns primitive or avatar entity with the wanted ID.
        /// @param id ID of the entity.
        /// @return Entity pointer matching the id or 0 if not found.
        Scene::Entity *GetEntityWithId(const RexUUID &id);

        /// Handles RexEmotionIcon generic message.
        /// @param params Parameters.
        void HandleRexEmotionIconMessage(StringVector &params);

        /// Handles ChatFromSimulator message.
        /// @param msg Network message.
        void HandleChatFromSimulatorMessage(ProtocolUtilities::NetInMessage &msg);

        /// Creates file for logging.
        /// @return True if the creation was succesful, false otherwise.
        bool CreateLogFile();

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

        /// Do we want to show the in-world chat bubbles
        bool showChatBubbles_;

        /// Do we want to log the chat messages.
        bool logging_;

        /// Log file.
        QFile *logFile_;

    //private slots:
        //void RepositionChatWidget(const QRectF &rect)
    };
}

#endif
