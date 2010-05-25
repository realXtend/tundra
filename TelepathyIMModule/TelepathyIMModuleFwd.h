// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TelepathyIMModule_TelepathyIMModuleFwd_h
#define incl_TelepathyIMModule_TelepathyIMModuleFwd_h

#include <vector>

namespace TpQt4Communication
{
    class Credentials;
}

namespace Communication
{
    class CredentialsInterface;
    class ContactInterface;
    class ContactGroupInterface;
    class ChatSessionParticipantInterface;
    class ChatMessageInterface;
    class ChatSessionInterface;
    class VoiceSessionParticipantInterface;
    class VideoPlaybackWidgetInterface;
    class AudioDataProducerInterface;
    class VoiceSessionInterface;
    class VideoSessionInterface;
    class FriendRequestInterface;
    class ConnectionInterface;
    class ConnectionProviderInterface;
    class CommunicationServiceInterface;

    class Credentials;
    class NetworkEventHandlerInterface;
    class CommunicationService;
//    class CommunicationModule;

    typedef std::vector<ContactInterface*> ContactVector;
    typedef std::vector<ContactGroupInterface*> ContactGroupVector;
    typedef std::vector<ChatMessageInterface*> ChatMessageVector;
    typedef std::vector<ChatSessionParticipantInterface*> ChatSessionParticipantVector;
    typedef std::vector<VoiceSessionParticipantInterface*> VoiceSessionParticipantVector;
    typedef std::vector<FriendRequestInterface*> FriendRequestVector;
    typedef std::vector<ConnectionInterface*> ConnectionVector;
    typedef std::vector<ConnectionProviderInterface*> ConnectionProviderVector;
}

namespace TelepathyIM
{
    class VoiceSessionParticipant;
    class VoiceSession;
    class VideoWidget;
    class VideoSession;
    class OutgoingFriendRequest;
    class GMainLoopThread;
    class FriendRequest;
    class FarsightChannel;
    class ContactGroup;
    class Contact;
    class ConnectionProvider;
    class Connection;
    class ChatSessionParticipant;
    class ChatSession;
    class ChatMessage;
    class FriendRequest;
    class ContactGroup;

    typedef std::vector<ChatMessage*> ChatMessageVector;
    typedef std::vector<ChatSessionParticipant*> ChatSessionParticipantVector;
    typedef std::vector<Contact*> ContactVector;
    typedef std::vector<ChatSession*> ChatSessionVector;
    typedef std::vector<VoiceSession*> VoiceSessionVector;
    typedef std::vector<FriendRequest*> FriendRequestVector;
    typedef std::vector<OutgoingFriendRequest*> OutgoingFriendRequestVector;
    typedef std::vector<VoiceSessionParticipant*> VoiceSessionParticipantVector;
}

namespace CommunicationUI
{
    class ChatSessionWidget;
    class EventHandler;
    class FriendListItem;
    class FriendListWidget;
    class OpenSimChatWidget;
    class MasterWidget;
}

namespace OpensimIM
{
    class ConnectionProvider;
    class ChatMessage;
    class Connection;
    class ChatSessionParticipant;
    class ChatSession;
    class VideoSessionWidget;
    class Contact;

    typedef std::vector<ChatMessage*> ChatMessageVector;
}

namespace UiDefines
{
    class UiStates;
    class PresenceStatus;
    class TimeStampGenerator;
}

namespace UiManagers
{
    class SessionManager;
}

namespace UiHelpers
{
    class ConfigHelper;
    class FriendHelper;
    class LoginHelper;
    class SessionHelper;
}

#endif // incl_TelepathyIMModule_TelepathyIMModuleFwd_h
