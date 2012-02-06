// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IModule.h"
#include "MumbleFwd.h"
#include "MumbleDefines.h"
#include "MumbleNetwork.h"
#include "AudioWizard.h"

#include <QObject>
#include <QSslSocket>
#include <QTimer>
#include <QScriptEngine>
#include <QPointer>
#include <QUuid> // todo remove when DebugConnect is removed

/** I watched Naali and Tundra Mumble functionality being developed for a long time.
    At the time the module was MumbleVoipModule and it was "over engineered" partly due to Naalis
    architectural demands. I've since rewritten parts of the module in multiple occasions,
    both in Tundra 1.x transition and Tundra 2.x transition. The complex messaging model between 
    the Murmur server -> libmumbleclient -> Tundra was always an issue. Also libmumbleclient was not developed
    by anyone but the realXtend Tundra devs and it had substantial amount of bugs that crashed our application
    constantly. 
    
    For the MumblePlugin rewrite from scratch I've decided to link directly to Google Protobufs, Celt, Speex 
    and OpenSSL to remove unwanted layers of complexity. The boost asio networking (from libmumbeclient)
    has been replaced with Qt networking. I also decided to not go and invent the wheel 
    again but read the Mumble client code to learn and in some cases reused their code as is. All source
    files from the Mumble repository https://github.com/mumble-voip/mumble have been modified in some way,
    for example adding namespace to avoid collisions in Tundra and remove header file includes that are not present
    or remove whole classes or sections of code. All Mumble source code is located in the mumble subfolder 
    of this project and license notices have been preserved as per requested by the license. 
    Anyone deploying Tundra with this module will also need to distribute the license with the binary, you can find the
    license from mumble/MumbleLicence.txt.
    
    Credit where credit is due: Thanks to pcgod for the original thin mumble client written with boost networking
    https://github.com/pcgod/libmumbleclient and the folks at mumble project https://github.com/mumble-voip/mumble.
    Both projects were thoroughly examined and both codebases heavily influenced how MumblePlugin turned out.

    Main features:
    - SSL TCP for tunneled input and output voice traffic and general protocol messages.
    - Encrypted UDP for input and output voice traffic.
    - Network mode auto detection and on the fly change from TCP to UDP and from UDP to TCP. This switch is done
      if UDP latencies get too high or ping stops responding. On the fly auto switch to TCP network mode should not be audible to the user.
    - Celt codec that is tested to work against >=1.2.3 Murmur and native Mumble clients.
      Essentially meaning native Mumble clients can also join the channels and everyone should hear each other.
    - Voice activity detection, mic noise suppression and mic volume amplification. Provides ready made audio wizard for easy configuration.
      VAD is self written (techniques studied from native mumble client), other audio processing uses speexdsp library.

    @todo list of not implemented features:
    - [trivial] Add per user voice activity detection, essentially signal when state changes Speaking(userid, bool)
    - [trivial] Remove debug prints and debug console command slots that were used for initial development.
    - [medium] Make new javascript example scene and script that uses the new MumblePlugin API to implement a nice client.
      Verify MumbleScriptTypeDefines.h exposes everything correctly to javascript with this example script! Fine tune signals for easy scripting.
*/
class MumblePlugin : public IModule
{

Q_OBJECT

public:
    /// Constructor.
    MumblePlugin();

    /// Deconstructor.
    virtual ~MumblePlugin();

    /// IModule override.
    void Initialize();

    /// IModule override.
    void Uninitialize();

    /// @see Signal UserPositionalChange
    void EmitPositionalChanged(MumbleUser *user);

protected:
    // QObject override.
    void timerEvent(QTimerEvent *event);

public slots:
    /// Connect to a Murmur server with provided information.
    /** @param address Server host.
        @param port Server port.
        @param username Client user name.
        @param password Client password.
        @param fullChannelName This channel name will be connected after login is completed.
        This must be full name of the channel eg. Root or Root/Tundra or Root/VoIP/Room1 etc.
        Default is "Root" which is the always present main channel on a Murmur server with id 0.
        @param outputAudioMuted Should we mute sending audio after login is completed. Default is false (sending audio).
        @param inputAudioMuted Should we mute receiving audio after login is completed. Default is false (receiving audio).
        @see JoinChannel, SetOutputAudioMuted and SetInputAudioMuted. */
    void Connect(QString address, int port, QString username, QString password, QString fullChannelName = "Root", bool outputAudioMuted = false, bool inputAudioMuted = false);

    /// Disconnect from Murmur server with optional reason.
    void Disconnect(QString reason = "");

    /// Sends a text message to all users in the currently joined channel.
    /** @param message Message to send.
        @return True if successful, false if own user or channel is not 
        there eg. when not connected to a server. 
        @see Signals ChannelTextMessageReceived and PrivateTextMessageReceived */
    bool SendTextMessage(const QString &message);

    /// Sends a text message to a user with userId. 
    /** Can be sent to any user, not just user in your current channel. 
        @param userId target users id.
        @param message Message to send.
        @return True if successful, false if target user is not 
        there eg. when not connected to a server.
        @see Signals ChannelTextMessageReceived and PrivateTextMessageReceived */
    bool SendTextMessage(uint userId, const QString &message);

    /// Join channel with full name eg. Root or Root/Tundra or Root/VoIP/Room1 etc.
    /** @note Case sensitive.
        @see MumbleChannel::fullName. */
    bool JoinChannel(QString fullName);

    /// Join channel with id.
    /// @see MumbleChannel::id.
    bool JoinChannel(uint id);

    /// Get channel by id.
    /** @see MumbleChannel::id.
        @return Null ptr if could not be found, otherwise valid channel ptr. */
    MumbleChannel* Channel(uint id);
    
    /// Get channel by full name eg. Root or Root/Tundra or Root/VoIP/Room1 etc.
    /** @note Case sensitive.
        @see MumbleChannel::fullName.
        @return Null ptr if could not be found, otherwise valid channel ptr. */
    MumbleChannel* Channel(QString fullName);
    
    /// Get channel where user with userId is currently.
    /** @see MumbleUser::id
        @return Null ptr if could not be found, otherwise valid channel ptr. */
    MumbleChannel* ChannelForUser(uint userId);

    /// Get user by id.
    /** @note Getting users by name is not supported as names are not unique.
        @return Null ptr if could not be found, otherwise valid user ptr. */
    MumbleUser* User(uint userId);

    /// Get own MumbleUser ptr.
    /** @note This will return null until client session id has been resolved,
        meaning even after Connected is fired this can and will return null for a short while
        until all channels and users have been synced to us.
        @see Signal MeCreated(). */
    MumbleUser* Me();

    /// Locally mute/unmute a user with id.
    /** @see Mute, UnMute */
    void SetMuted(uint userId, bool muted);

    /// Locally mute a user with id.
    /** @see SetMuted */
    void Mute(uint userId);

    /// Locally unmute a user with id.
    /** @see SetMuted */
    void UnMute(uint userId);

    /// Current connection state.
    /** @see Signals Connected, Disconnected, StateChange. */
    MumbleNetwork::ConnectionState State();

    /// Current network mode, TCP or UDP. 
    /** Not needed by the client code but may be useful information to the end user/user interface.
        @see Signal NetworkModeChange. */
    MumbleNetwork::NetworkMode NetworkMode();

    /// Set if output audio is sent to the server.
    /** False sends voice from microphone to the server, true mutes sending audio. 
        @see Signals SetOutputAudioLoopBack and SetOutputAudioPositional */
    void SetOutputAudioMuted(bool outputAudioMuted);

    /// Set if server should loop back your audio back to us. Not local playback, will take some bandwidth.
    /** True sends your voice back to from the server and other connected clients cannot hear you,
        false sends audio normally and other clients will hear you.
        Useful for voice modes where you want to verify your microphone levels and or
        that networking to server works as expected.
        @note Default after connected is false. */
    void SetOutputAudioLoopBack(bool loopBack);

    /// Set if our voice is positional. 
    /** True sends active EC_SoundListener Entitys EC_Placeable position
        to the server, this information gets relayed to other clients for positional audio playback.
        False does not send position to the server hence making audio non positional.
        @note When setting true make sure correct EC_SoundListener is active to have expected audio
        playback experience in other clients. If no active EC_SoundListener is be found from the scene, 
        sending positional audio is disabled automatically.
        @note Set value is remembered between multiple connections but forgotten on module unload it
        is not stored into any config on disk, your ui layer should do that if needed. */
    void SetOutputAudioPositional(bool positional);

    /// Set if input audio is received from server. 
    /** @note Calling this function with true will make us not send or receive audio. If you are not receiving, you cannot send.
        This is how the mumble protocol operates, it saves bandwidth and processing on the server and client(s).
        @see SetOutputAudioMuted. */
    void SetInputAudioMuted(bool inputAudioMuted);

    /// Shows a audio wizard widget.
    /** Shows a audio wizard widget that lets user tweak voice settings for bitrate quality, 
        noise suppression, mic amplification, transmit mode and voice activity detection.
        @note Automatically saves/loads settings to local user profile on disk. */
    void RunAudioWizard();

signals:
    /// Murmur server connection has been established, authenticated and both TCP and UDP connections are ready.
    /** After this signal you can expect the channel and user specific signals to be triggered.
        @see Signals ChannelCreated, UserCreated, MeCreated and JoinedChannel */
    void Connected(QString address, int port, QString username);
    
    /** Disconnected from Murmur server with reason string that can be shows in user interfaces.
        @note The reason string can be empty. */
    void Disconnected(QString reason = "");

    /** State of the connection changed from oldState to newState.
        @note This signal will trigged 'duplicates' with Connected ans Disconnected. */
    void StateChange(MumbleNetwork::ConnectionState newState, MumbleNetwork::ConnectionState oldState);

    /// Network mode changed to mode with reason. 
    /** Murmur is always connected first in TCP mode. After the initial ping(s) is replied UDP mode can be enabled 
        if certain demands are met for latency etc. Connection can revert back to TCP if UDP is detected to fail. 
        This all is transparent to the end user or 3rd party client code but it might be nice to know on 
        what mode we are currently for eg. user interface. */
    void NetworkModeChange(MumbleNetwork::NetworkMode mode, QString reason);

    /// Connection attempt rejected was rejected.
    /** @note Disconnected state change signals will fire after this. 
        @see Signals Connected, Disconnected */
    void ConnectionRejected(MumbleNetwork::RejectReason reasonType, QString reasonMessage);

    /// Permission for action was rejected
    /** @note MumblePlugin will do log prints for some of the common cases to inform the user. This
        signals is mostly meant for advanced user interfaces.
        @param denyReason enum for why operation got rejected.
        @param permission enum for what operation got rejected.
        @param channelId MumbleChannel id where the denial occurred, this can be 0 if its not associated to any channel.
        @param targetUserId MumbleUser id on who the denial occurred, this can be 0 if its not associated to any user.
        @param reason String reason that can be used in user interfaces as is. Can be a empty string. */
    void PermissionDenied(MumbleNetwork::PermissionDeniedType denyReason, MumbleNetwork::ACLPermission permission, uint channelId, uint targetUserId, QString reason);

    /// Channel text message received for the current channel we are in.
    /** @param fromUser MumbleUser ptr of the sender. 
        @param message Received text message.
        @see SendTextMessage */
    void ChannelTextMessageReceived(MumbleUser *fromUser, QString message);

    /// Private text message received from another user.
    /** @param fromUser MumbleUser ptr of the sender. 
        @param message Received text message.
        @see SendTextMessage */
    void PrivateTextMessageReceived(MumbleUser *fromUser, QString message);

    /// New channel created.
    void ChannelCreated(MumbleChannel *channel);

    /// Existing channel with id was removed.
    /** @note Channel id is used as parameter because the channel ptr is  
        deleted before this signal. Do not try to get the channel ptr with the id. */
    void ChannelRemoved(uint id);

    /// Existing channels data was updated. 
    /** @note This it not emitted on when users leave/join the channel.
        @see Signal MumbleChannel::UsersChanged */
    void ChannelUpdated(MumbleChannel *channel);

    /// When channel list changed.
    /** Channel/channels was added or removed from list in the params. You can also listen to 
        ChannelCreated and ChannelRemoved to have the exact same information. 
        @see Signals ChannelCreated, ChannelRemoved */
    void ChannelsChanged(QList<MumbleChannel*> channels);

    /// Joining a requested channel failed with reason.
    void JoinChannelFailed(QString reason);

    /// New user connected.
    /** @see Signal MumbleChannel::UsersChanged */
    void UserCreated(MumbleUser *user);

    /// User information updated.
    void UserUpdated(MumbleUser *user);

    /** User local muted state changed by us.
        @see Signal MumbleUser::Muted */
    void UserMuted(MumbleUser *user, bool muted);

    /// Users self muted state changed. 
    /** Meaning this user muted/unmuted outgoing voice himself.
        If selfMuted is true no one on the channel can hear him (guaranteed by the server). 
        @see Signal MumbleUser::SelfMuted */
    void UserSelfMuted(MumbleUser *user, bool selfMuted);

    /// This users self deaf state changed. 
    /** Meaning this user deafen/undeafen both incoming and outgoing voice himself.
        If selfDeaf is true no one on the channel can hear him or send audio to him (guaranteed by the server). 
        @see Signal MumbleUser::SelfDeaf */
    void UserSelfDeaf(MumbleUser *user, bool selfDeaf);

    /// This users positional state changed.
    /** If positional is true this users audio is played with position.
        False it is played as mono. 
        @note You can access the position from MumbleUser::pos property. 
        @see Signal MumbleUser::PositionalChanged */
    void UserPositionalChanged(MumbleUser *user, bool positional);

    /** Own MumbleUser was created. Fired when all channels and users have
        been synced to us and we have a valid client session id. */
    void MeCreated(MumbleUser *me);

    /** Own MumbleUser joined a channel. Provided for easier 
        hooking up to the channel signals. */
    void JoinedChannel(MumbleChannel *channel);    

private slots:
    // Various private slots for processing data from the MumbleNetworkHandler thread.
    // Usually verifies that data is good and emits one of the MumblePlugin signals, calls its public slots or modifies MumblePluginState.
    void OnConnected(QString address, int port, QString username);
    void OnDisconnected(QString reason);
    void OnStateChange(MumbleNetwork::ConnectionState newState);
    void OnNetworkModeChange(MumbleNetwork::NetworkMode mode, QString reason);
    void OnConnectionRejected(MumbleNetwork::RejectReason reasonType, QString reasonMessage);
    void OnPermissionDenied(MumbleNetwork::PermissionDeniedType denyReason, MumbleNetwork::ACLPermission permission, uint channelId, uint targetUserId, QString reason);
    void OnTextMessageReceived(bool isPrivate, QList<uint> channelIds, uint senderId, QString message);
    void OnChannelUpdate(uint id, uint parentId, QString name, QString description);
    void OnChannelRemoved(uint id);
    void OnUserUpdate(uint id, uint channelId, QString name, QString comment, QString hash, bool selfMuted, bool selfDeaf, bool isMe);
    void OnUserLeft(uint id, uint actorId, bool banned, bool kicked, QString reason);

    // Stores to local user profile disk config if saveConfig is true. Receives this signal from AudioWizards OK, Cancel and Apply buttons.
    void OnAudioSettingChanged(MumbleAudio::AudioSettings settings, bool saveConfig);

    // When server send synced message we have all server channels and users.
    void OnServerSynced(uint sessionId);

    // Registers the QObject and other types from this plugin to created script engines.
    void OnScriptEngineCreated(QScriptEngine *engine);

    // Updates our active EC_SoundListener position to the voice packet info.
    void UpdatePositionalInfo(MumbleNetwork::VoicePacketInfo &packetInfo);

    MumbleAudio::AudioSettings LoadSettings();
    void SaveSettings(MumbleAudio::AudioSettings settings);

    // Console command debugging, to be removed once initial development is completed.
    void DebugConnect()     { Connect("127.0.0.1", 64738, "Debug" + QUuid::createUuid().toString(), "debug", "Root/test", false, false); }
    void DebugMute()        { SetOutputAudioMuted(true); }
    void DebugUnMute()      { SetOutputAudioMuted(false); }
    void DebugDeaf()        { SetInputAudioMuted(true); }
    void DebugUnDeaf()      { SetInputAudioMuted(false); }
    void DebugMute(QString userIdStr);

private:
    MumbleNetworkHandler *network_;
    MumbleAudio::AudioProcessor *audio_;

    QList<MumbleChannel*> channels_;
    QList<MumbleUser*> pendingUsers_;
    
    MumblePluginState state;

    QPointer<MumbleAudio::AudioWizard> audioWizard;

    int qobjTimerId_;

    QString LC;
};
