// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IModule.h"
#include "MumbleFwd.h"
#include "MumbleDefines.h"
#include "MumbleNetwork.h"

#include <QObject>
#include <QSslSocket>
#include <QTimer>
#include <QScriptEngine>
#include <QUuid> // todo remove when DebugConnect is removed

/** I watched Naali and Tundra Mumble functionality being developed for a long time.
    At the time the module was MumbleVoipModule and it was "over engineered" partly due to Naalis
    architectural demands. I've since rewritten parts of the module in multiple occasions,
    both in Tundra 1.x transition and Tundra 2.x transition. The complex messaging model between 
    the Murmur server -> libmumbleclient -> Tundra was always an issue. Also libmumbleclient was not developed
    by anyone but the realXtend Tundra devs and it had substantial amount of bugs that crashed our application
    constantly. 
    
    For the MumblePlugin rewrite from scratch I've decided to link directly to Google Protobufs, Celt 
    and OpenSSL to remove unwanted layers of complexity. The boost asio networking (from libmumbeclient)
    has been replaced with Qt networking. I also decided to not go and invent the wheel 
    again but read the Mumble client code to learn and in some cases reused their code as is. All source
    files from the Mumble repository https://github.com/mumble-voip/mumble have been modified in some way,
    for example adding namespace to avoid collisions in Tundra and remove header file that are not present.
    All Mumble source code is located in the mumble subfolder of this project and license notices have been 
    preserved as per requested by the license. Anyone deploying Tundra with this module will also need to 
    distribute the license with the binary.
    
    Credit where credit is due: Thanks to pcgod for the original thin mumble client written with boost networking
    https://github.com/pcgod/libmumbleclient and the folks at mumble project https://github.com/mumble-voip/mumble.
    Both projects were thoroughly examined and both codebases heavily influenced how MumblePlugin turned out.

    Main features:
    - SSL TCP for tunneled input and output voice traffic and general protocol messages.
    - Encrypted UDP for input and output voice traffic.
    - Network mode auto detection and on the fly change from TCP UDP and from UDP to TCP. This switch is done
      if UDP latencies get too high or ping stops responding. On the fly auto switch to TCP network mode mosty times not even audible to end user.
    - Celt codec that is tested to work against >=1.2.3 Murmur and native Mumble clients.
      Essentially meaning native Mumble clients can also join the channels and everyone should hear each other.

    @todo list of not implemented features:
    - [trivial] Implement per user mute (send state to server so it wont send us the voice traffic for nothing)
    - [trivial] Implement text messages to and from the server. Expose to 3rd party code.
    - [trivial] Implement auto reconnect on disconnects. Optional and should be exposed to 3rd party code.
    - [trivial] Implement own "avatar" positions tracking. Setting float3 constantly from a script will be too slow.
      Use active EC_SoundListener automatically as there can only be one, this will suit most use cases.
      Still leave a option to set position from 3rd party code directly if someone wants to do some custom stuff.
    - [trivial] Add signals to MumbleChannel and MumbleUser classes that are exposed to 3rd party code. 
      eg. MumbleChannel: UserJoined(id/name), UserLeft(id/name), MumbleUser: mute/selfmute and deaf/selfdeaf state changes etc.
    - [trivial] Remove debug prints and debug console command slots that were used for initial development.
    - [medium] Implement audio quality and frames per packet exposing to 3rd party code.
    - [medium] Make new javascript example scene and script that uses the new MumblePlugin API to implement a nice client.
      Verify MumbleScriptTypeDefines.h exposes everything correctly to javascript with this example script!
    - [potentially hard] Research and implement the ones that are sensible: microphone audio voice level detection, 
      echo cancellation and noise deduction. For this study the speex processing parts of the native Mumble client. 
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

    /// IModule override.
    void Update(f64 frametime);

protected:
    // QObject override.
    void timerEvent(QTimerEvent *event);

public slots:
    /// Connect to a Murmur server with provided information.
    /// @param address Server host.
    /// @param port Server port.
    /// @param username Client user name.
    /// @param password Client password.
    /// @param fullChannelName This channel name will be connected after login is completed.
    /// This must be full name of the channel eg. Root or Root/Tundra or Root/VoIP/Room1 etc.
    /// Default is "Root" which is the always present main channel on a Murmur server with id 0.
    /// @param outputAudioMuted Should we mute sending audio after login is completed. Default is false (sending audio).
    /// @param inputAudioMuted Should we mute receiving audio after login is completed. Default is false (receiving audio).
    /// @see JoinChannel, SetOutputAudioMuted and SetInputAudioMuted.
    void Connect(QString address, int port, QString username, QString password, QString fullChannelName = "Root", bool outputAudioMuted = false, bool inputAudioMuted = false);

    /// Disconnect from Murmur server with optional reason.
    void Disconnect(QString reason = "");

    /// Join channel with full name eg. Root or Root/Tundra or Root/VoIP/Room1 etc.
    /// @note Case sensitive.
    /// @see MumbleChannel::fullName.
    bool JoinChannel(QString fullName);

    /// Join channel with id.
    /// @see MumbleChannel::id.
    bool JoinChannel(uint id);

    /// Get channel by id.
    /// @see MumbleChannel::id.
    /// @return Null ptr if could not be found, otherwise valid channel ptr.
    MumbleChannel* Channel(uint id);
    
    /// Get channel by full name eg. Root or Root/Tundra or Root/VoIP/Room1 etc.
    /// @note Case sensitive.
    /// @see MumbleChannel::fullName.
    /// @return Null ptr if could not be found, otherwise valid channel ptr.
    MumbleChannel* Channel(QString fullName);
    
    /// Get channel where user with userId is currently.
    /// @see MumbleUser::id
    /// @return Null ptr if could not be found, otherwise valid channel ptr.
    MumbleChannel* ChannelForUser(uint userId);

    /// Get user by id.
    /// @note Getting users by name is not supported as names are not unique.
    /// @return Null ptr if could not be found, otherwise valid user ptr.
    MumbleUser* User(uint userId);

    /// Current connection state.
    /// @see Signals Connected, Disconnected, StateChange.
    MumbleNetwork::ConnectionState State();

    /// Current network mode, TCP or UDP. Not needed by the client code
    /// but may be useful information to the end user.
    /// @see Signal NetworkModeChange.
    MumbleNetwork::NetworkMode NetworkMode();

    /// Set if output audio is sent to the server.
    void SetOutputAudioMuted(bool outputAudioMuted);

    /// Set if input audio is received from server. Calling this function with true
    /// will make us not send or receive audio. This is how the mumble protocol operates,
    /// it saves bandwidth and processing on the server and client(s).
    /// @note If you are not receiving audio, you cannot send audio!
    /// @see SetOutputAudioMuted.
    void SetInputAudioMuted(bool inputAudioMuted);

signals:
    /// Murmur server connection has been established, authenticated and both TCP and UDP streams are ready.
    /// After this signal you can expect the channel and user specific signals to be triggered.
    void Connected(QString address, int port, QString username);
    
    /// Disconnected from Murmur server with reason string that can be shows in user interfaces.
    /// @note The reason string can be empty.
    void Disconnected(QString reason = "");

    /// State of the connection changed from oldState to newState.
    /// @note This signal will trigged 'duplicates' with Connected ans Disconnected.
    void StateChange(MumbleNetwork::ConnectionState newState, MumbleNetwork::ConnectionState oldState);

    /// Network mode changed to mode with reason. Murmur is always connected first in TCP mode.
    /// After the initial ping is replied UDP mode can be enabled if certain demands are met for latency etc.
    /// Connection can revert back to TCP if UDP is detected to fail. This all is transparent to the end user
    /// or client code but it might be nice to know on what mode we are currently for eg. user interface.
    void NetworkModeChange(MumbleNetwork::NetworkMode mode, QString reason);

    /// Connection attempt rejected was rejected.
    /// @note Disconnected state change signals will fire after this.
    void ConnectionRejected(MumbleNetwork::RejectReason reasonType, QString reasonMessage);

    /// When channel list changes, meaning either a channel(s) was added or removed.
    void ChannelsChanged(QList<MumbleChannel*> channels);

    /// New channel created.
    void ChannelCreated(MumbleChannel *channel);

    /// Existing channel with id was removed.
    void ChannelRemoved(uint id);

    /// Existing channels data was updated. Note that this is NOT emitted on user changes inside the.
    /// @see MumbleChannel::UsersChanged(QList<MumbleUser*> users)
    void ChannelUpdated(MumbleChannel *channel);

    /// New user connected.
    /// @see MumbleChannel::UsersChanged(QList<MumbleUser*> users)
    void UserCreated(MumbleUser *user);

    /// User information updated.
    void UserUpdated(MumbleUser *user);

private slots:
    void OnConnected(QString address, int port, QString username);
    void OnDisconnected(QString reason);
    void OnStateChange(MumbleNetwork::ConnectionState newState);
    void OnNetworkModeChange(MumbleNetwork::NetworkMode mode, QString reason);
    void OnConnectionRejected(MumbleNetwork::RejectReason reasonType, QString reasonMessage);

    void OnChannelUpdate(uint id, uint parentId, QString name, QString description);
    void OnChannelRemoved(uint id);
    void OnUserUpdate(uint id, uint channelId, QString name, QString comment, QString hash, bool selfMuted, bool selfDeaf, bool isMe);
    void OnUserLeft(uint id, uint actorId, bool banned, bool kicked, QString reason);

    // When server send synced message we have all server channels and users.
    void OnServerSynced(uint sessionId);

    // Registers the QObject and other types from this plugin to created script engines.
    void OnScriptEngineCreated(QScriptEngine *engine);

    // Console command debugging
    void DebugConnect()     { Connect("127.0.0.1", 64738, "Debug" + QUuid::createUuid().toString(), "debug", "Root", false, false); }
    void DebugMute()        { SetOutputAudioMuted(true); }
    void DebugUnMute()      { SetOutputAudioMuted(false); }
    void DebugDeaf()        { SetInputAudioMuted(true); }
    void DebugUnDeaf()      { SetInputAudioMuted(false); }
    void DebugFrames(QString frames);

private:
    MumbleNetworkHandler *network_;
    MumbleAudio::AudioProcessor *audio_;

    QList<MumbleChannel*> channels_;
    QList<MumbleUser*> pendingUsers_;
    
    MumblePluginState state;

    int qobjTimerId_;

    QString LC;
};
