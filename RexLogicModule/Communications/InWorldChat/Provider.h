// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicModule_Communications_Provider_h
#define incl_RexLogicModule_Communications_Provider_h

#include "CommunicationsService.h"
#include <QString>

namespace Foundation
{
    class Framework;
    class EventDataInterface;
}

namespace RexLogic
{
    namespace InWorldChat
    {
        class Session;

        /// @todo rename to Manager ?
        class Provider : public Communications::InWorldChat::ProviderInterface
        {
            Q_OBJECT
        public:
            /// Constructor
            explicit Provider(Foundation::Framework* framework);

            /// destructor
            virtual ~Provider();

            /// @return session object, return 0 if the session doesn't exist
            virtual Communications::InWorldChat::SessionInterface* Session() const;

            /// Provider description
            virtual QString Description()const;

            /// Notify provider about chat message received from OpenSim server
            virtual void HandleIncomingChatMessage(const QString& from_uuid, const QString& from_name, const QString& text);

            /// Creates and deletes sessions
            virtual bool HandleNetworkStateEvent(event_id_t event_id, Foundation::EventDataInterface* data);

            virtual bool HandleSceneEvent(event_id_t event_id, Foundation::EventDataInterface* data);

        private slots:
            virtual void SendChatMessgeToServer(const QString& text);

        private:
            virtual void RegisterToCommunicationsService();
            virtual QString OwnAvatarId();

            Foundation::Framework* framework_;
            QString description_;
            InWorldChat::Session* session_;
            QList<InWorldChat::Session*> closed_sessions_;
        };
    }
}

#endif // incl_RexLogic_Communications_Provider_h
