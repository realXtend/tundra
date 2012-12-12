// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "MumblePlugin.h"
#include "MumbleNetwork.h"
#include "MumbleNetworkHandler.h"
#include "MumbleData.h"
#include "AudioProcessor.h"

#include "Mumble.pb.h"
#include "mumble/SSL.h"

#include "Framework.h"
#include "Profiler.h"
#include "Application.h"
#include "AudioAPI.h"
#include "ConsoleAPI.h"
#include "ConfigAPI.h"
#include "LoggingFunctions.h"

#include "IRenderer.h"
#include "Scene/Scene.h"
#include "Entity.h"
#include "EC_Placeable.h"
#include "EC_SoundListener.h"

#include "JavascriptModule.h"
#include "MumbleScriptTypeDefines.h"

using namespace MumbleNetwork;
using namespace MumbleAudio;

MumblePlugin::MumblePlugin() :
    IModule("MumblePlugin"),
    LC("[MumblePlugin]: "),
    network_(0),
    audio_(0),
    qobjTimerId_(-1)
{
    state.Reset();
}

MumblePlugin::~MumblePlugin()
{
    if (!framework_->IsHeadless())
        QSslSocket::setDefaultCaCertificates(QList<QSslCertificate>()); // Hides a qt memory leak
}

void MumblePlugin::Initialize()
{
    if (framework_->IsHeadless())
        return;

    /** @note QSslSocket::supportsSsl() is eventually called, either explicitly by us
        or by the QSslSocket when its connecting. The reason this is done in MumblePlugin::Initialize
        is it blocks quite heavily (tested on windows) and it seems this cannot be avoided. For my test setups
        on windows it takes about 3 seconds done here and about 7 seconds if the server connection and mainloop
        are already running. Doing this here won't interrupt ui/mainloop in the first connection that is made
        using MumbplePlugin, instead the startup time is hindered a bit, which again seems is unavoidable but in my opinion
        makes the user experience a lot better. Also note this blocking was also present in previous MumbleVoipModule 
        with libmumbeclient library, it just happened before the first connection (back then was a mystery why this happened 
        as it was buried inside boost asio networking). Maybe this will gets fixed in newer version of OpenSSL or Qt, one can only hope. */
    if (QSslSocket::supportsSsl())
    {
        QStringList addedCerts = Mumble::MumbleSSL::addSystemCA();
        foreach(QString certMsg, addedCerts)
            LogInfo(LC + certMsg);
    }
    else
        LogWarning(LC + "SSL not supported, you cannot connect to Murmur servers without it. Either OpenSSL libraries are missing or your Qt libraries were build without OpenSSL support!");

    RegisterMumblePluginMetaTypes();

    // Register custom QObject and enum meta types to created script engines.
    JavascriptModule *javascriptModule = framework_->GetModule<JavascriptModule>();
    if (javascriptModule)
        connect(javascriptModule, SIGNAL(ScriptEngineCreated(QScriptEngine*)), SLOT(OnScriptEngineCreated(QScriptEngine*)));
    else
        LogWarning(LC + "JavascriptModule not present, MumblePlugin usage from scripts will be limited!");

    framework_->RegisterDynamicObject("mumble", this);
    
    framework_->Console()->RegisterCommand("mumblepackets", "Set MumbleVoip amount of frames per network packet, eg. mumblepackets(6).", 
        this, SLOT(OnFramesPerPacketChanged(const QStringList&)));

    // Audio processing with ~60 fps. This is not tied to FrameAPI::Updated or IModule::Update
    // because we want to process audio at a steady and fast rate even if mainloop max fps is capped to eg. 30 fps.
    qobjTimerId_ = startTimer(15);
}

void MumblePlugin::Uninitialize()
{
    if (!framework_->IsHeadless())
    {
        killTimer(qobjTimerId_);
        Disconnect("Client exiting.");
    }
}

void MumblePlugin::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != qobjTimerId_)
        return;

    // Input/output audio processing
    PROFILE(MumblePlugin_Update)
    if (audio_ && state.serverSynced && state.connectionState == MumbleConnected)
    {
        if (!network_)
            return;
        MumbleUser *me = Me();
        if (!me)
            return;

        // Output audio
        if (!state.outputAudioMuted)
        {
            float levelPeakMic = 0.0f;
            bool speaking = false;

            PROFILE(MumblePlugin_Update_ProcessOutputAudio)
            VoicePacketInfo packetInfo(audio_->ProcessOutputAudio());
            audio_->GetLevels(levelPeakMic, speaking);
            ELIFORP(MumblePlugin_Update_ProcessOutputAudio)

            if (packetInfo.encodedFrames.size() > 0)
            {
                // Remember that in Mumble protocol when you are 'deaf' (state.inputAudioMuted == true) 
                // no one will hear you even if you send the voice packets to the server. Skip sending 
                // anything in this case and mark as not speaking so the end user wont get alarmed 
                // that his voice is going out in 'deaf' mode.
                PROFILE(MumblePlugin_Update_ProcessOutputNetwork)
                if (!state.inputAudioMuted)
                {
                    packetInfo.isLoopBack = state.outputAudioLoopBack;
                    if (!packetInfo.isLoopBack && audioWizard)
                        packetInfo.isLoopBack = true;
                    if (state.outputPositional)
                    {
                        // Sets packetInfo.isPositional false if no active EC_SoundListener is found.
                        // If found updates packetInfo and our MumbleUser position.
                        UpdatePositionalInfo(packetInfo);
                        me->SetAndEmitPositional(packetInfo.isPositional);
                    }
                    else
                        me->SetAndEmitPositional(false);
                    me->SetAndEmitSpeaking(true);

                    network_->SendVoicePacket(packetInfo);
                }
                else
                    me->SetAndEmitSpeaking(false);
                ELIFORP(MumblePlugin_Update_ProcessOutputNetwork)
            }
            else if (!speaking)
                me->SetAndEmitSpeaking(false);

            if (audioWizard)
                audioWizard->SetLevels(levelPeakMic, speaking);
        }
        else
        {
            me->SetAndEmitSpeaking(false);
            audio_->ClearOutputAudio();
        }

        PROFILE(MumblePlugin_Update_ProcessInputAudio)
        // Input audio
        if (!state.inputAudioMuted)
            audio_->PlayInputAudio(this);
        else
            audio_->ClearInputAudio();
        ELIFORP(MumblePlugin_Update_ProcessInputAudio)
    }
    else if (audio_)
    {
        audio_->ClearInputAudio();
        audio_->ClearOutputAudio();
    }
    ELIFORP(MumblePlugin_Update)
}

void MumblePlugin::Connect(QString address, int port, QString username, QString password, QString fullChannelName, bool outputAudioMuted, bool inputAudioMuted)
{
    if (framework_->IsHeadless())
    {
        LogError(LC + "Cannot connect in headless mode!");
        return;
    }

    Disconnect();

    if (fullChannelName.trimmed().isEmpty())
        fullChannelName = "Root";

    state.address = address;
    state.port = (ushort)port;
    state.username = username;
    state.fullChannelName = fullChannelName;
    state.outputAudioMuted = outputAudioMuted;
    state.inputAudioMuted = inputAudioMuted;

    LogInfo(LC + "Connecting to " + state.FullHost() + " as \"" + state.username + "\"");
    state.connectionState = MumbleConnecting;
    emit StateChange(state.connectionState);

    MumbleAudio::AudioSettings currentSettings = LoadSettings();
    state.outputPositional = currentSettings.allowSendingPositional;

    audio_ = new MumbleAudio::AudioProcessor(framework_, currentSettings);
    audio_->moveToThread(audio_);

    network_ = new MumbleNetworkHandler(state.address, state.port, state.username, password);
    network_->codecBitStreamVersion = audio_->CodecBitStreamVersion();
    network_->moveToThread(network_);

    // Handle signals from network thread to the main thread.
    connect(network_, SIGNAL(Connected(QString, int, QString)), SLOT(OnConnected(QString, int, QString)), Qt::QueuedConnection);
    connect(network_, SIGNAL(Disconnected(QString)), SLOT(OnDisconnected(QString)), Qt::QueuedConnection);
    connect(network_, SIGNAL(ServerSynced(uint)), SLOT(OnServerSynced(uint)), Qt::QueuedConnection);
    connect(network_, SIGNAL(NetworkModeChange(MumbleNetwork::NetworkMode, QString)), SLOT(OnNetworkModeChange(MumbleNetwork::NetworkMode, QString)), Qt::QueuedConnection);
    
    connect(network_, SIGNAL(ConnectionRejected(MumbleNetwork::RejectReason, QString)), SLOT(OnConnectionRejected(MumbleNetwork::RejectReason, QString)), Qt::QueuedConnection);
    connect(network_, SIGNAL(PermissionDenied(MumbleNetwork::PermissionDeniedType, MumbleNetwork::ACLPermission, uint, uint, QString)), 
        SLOT(OnPermissionDenied(MumbleNetwork::PermissionDeniedType, MumbleNetwork::ACLPermission, uint, uint, QString)), Qt::QueuedConnection);

    connect(network_, SIGNAL(TextMessageReceived(bool, QList<uint>, uint, QString)), SLOT(OnTextMessageReceived(bool, QList<uint>, uint, QString)), Qt::QueuedConnection);
    connect(network_, SIGNAL(ChannelUpdate(uint, uint, QString, QString)), SLOT(OnChannelUpdate(uint, uint, QString, QString)), Qt::QueuedConnection);
    connect(network_, SIGNAL(ChannelRemoved(uint)), SLOT(OnChannelRemoved(uint)), Qt::QueuedConnection);
    connect(network_, SIGNAL(UserLeft(uint, uint, bool, bool, QString)), SLOT(OnUserLeft(uint, uint, bool, bool, QString)), Qt::QueuedConnection);
    connect(network_, SIGNAL(UserUpdate(MumbleNetwork::MumbleUserState)), SLOT(OnUserUpdate(MumbleNetwork::MumbleUserState)), Qt::QueuedConnection);

    // Handle audio signals from network thread to audio thread.
    connect(network_, SIGNAL(AudioReceived(uint, uint, ByteArrayVector, bool, float3)), audio_, SLOT(OnAudioReceived(uint, uint, ByteArrayVector, bool, float3)), Qt::QueuedConnection);
    
    audio_->start(QThread::HighPriority);
    network_->start(QThread::HighPriority);
}

void MumblePlugin::Disconnect(QString reason)
{
    pendingUsersStates_.clear();

    // Note that this frees all channel users as well.
    foreach(MumbleChannel *c, channels_)
        SAFE_DELETE(c);
    channels_.clear();

    if (audioWizard)
        delete audioWizard;

    if (audio_ && audio_->isRunning())
    {
        audio_->exit();
        audio_->wait();
    }
    SAFE_DELETE(audio_);

    state.serverSynced = false;
    if (network_ && network_->isRunning())
    {
        network_->exit();
        network_->wait();
    }
    SAFE_DELETE(network_);

    if (state.connectionState != MumbleNetwork::MumbleDisconnected)
    {
        if (!reason.isEmpty())
            LogInfo(LC + "Disconnected from " + state.FullHost() + ": " + reason);
        else
            LogInfo(LC + "Disconnected from " + state.FullHost());
        state.connectionState = MumbleNetwork::MumbleDisconnected;
        emit StateChange(state.connectionState);
        emit Disconnected(reason);
    }

    state.Reset();
}

bool MumblePlugin::SendTextMessage(const QString &message)
{
    if (!state.serverSynced || state.connectionState != MumbleConnected || !network_)
    {
        LogError(LC + "Cannot send text message, not connected to a server");
        return false;
    }

    MumbleUser *me = Me();
    if (me)
    {
        MumbleChannel *channel = me->Channel();
        if (channel)
        {
            MumbleProto::TextMessage messageText;
            messageText.add_channel_id(channel->id);
            messageText.set_message(utf8(message));
            network_->SendTCP(TextMessage, messageText);
            return true;
        }
        else
            LogError(LC + "Could not find our current channel from the current state to send the text message.");
    }
    else
        LogError(LC + "Could not find our user from the current state to send the text message.");

    return false;
}

bool MumblePlugin::SendTextMessage(uint userId, const QString &message)
{
    if (!state.serverSynced || state.connectionState != MumbleConnected || !network_)
    {
        LogError(LC + "Cannot send text message to user with id " + QString::number(userId) + ", not connected to a server");
        return false;
    }

    MumbleUser *target = User(userId);
    if (target)
    {
        if (target->id != state.sessionId)
        {
            MumbleProto::TextMessage messageText;
            messageText.add_session(target->id);
            messageText.set_message(utf8(message));
            network_->SendTCP(TextMessage, messageText);
            return true;
        }
        else
            LogError(LC + "Cannot sent text message to own user id " + QString::number(userId));
    }
    else
        LogError(LC + "Could not find user with id " + QString::number(userId) + "from the server to send the text message.");
        
    return false;
}

bool MumblePlugin::JoinChannel(QString fullName)
{
    if (!state.serverSynced || state.connectionState != MumbleConnected || !network_)
    {
        LogError(LC + "Could not find channel with full name \"" + fullName + "\" not connected to a server.");
        return false;
    }

    MumbleChannel *channel = Channel(fullName);
    if (channel)
        return JoinChannel(channel->id);
    else
    {
        QString reason = "Channel with full name \"" + fullName + "\" does not exist, cannot join.";
        emit JoinChannelFailed(reason);
        LogError(LC + reason);
        return false;
    }
}

bool MumblePlugin::JoinChannel(uint id)
{
    if (!state.serverSynced || state.connectionState != MumbleConnected || !network_)
    {
        LogError(LC + "Could not find channel with id\"" + QString::number(id) + "\" for join operation.");
        return false;
    }

    MumbleUser *me = Me();
    if (!me)
    {
        LogError(LC + "Could not join channel, own user ptr null!");
        return false;
    }
    if (me->channelId == id)
    {
        LogInfo(LC + "Already joined to the requested channel.");
        return true;
    }

    MumbleChannel *channel = Channel(id);
    if (channel)
    {
        MumbleProto::UserState message;
        message.set_session(state.sessionId);
        message.set_channel_id(id);

        network_->SendTCP(UserState, message);
        return true;
    }
    else
    {
        QString reason = "Channel with id \"" + QString::number(id) + "\" does not exist, cannot join.";
        emit JoinChannelFailed(reason);
        LogError(LC + reason);
        return false;
    }
}

MumbleChannel* MumblePlugin::Channel(uint id)
{
    MumbleChannel *channel = 0;
    foreach(MumbleChannel *iter, channels_)
    {
        if (iter->id == id)
        {
            channel = iter;
            break;
        }
    }
    return channel;
}

MumbleChannel* MumblePlugin::Channel(QString fullName)
{
    MumbleChannel *channel = 0;
    foreach(MumbleChannel *iter, channels_)
    {
        if (QString::compare(iter->fullName, fullName, Qt::CaseSensitive) == 0)
        {
            channel = iter;
            break;
        }
    }
    return channel;
}

MumbleChannel *MumblePlugin::ChannelForUser(uint userId)
{
    MumbleChannel *channel = 0;
    foreach(MumbleChannel *iter, channels_)
    {
        if (iter->User(userId))
        {
            channel = iter;
            break;
        }
    }
    return channel;
}

MumbleUser* MumblePlugin::User(uint userId)
{
    MumbleUser *user = 0;
    foreach(MumbleChannel *iter, channels_)
    {
        user = iter->User(userId);
        if (user)
            break;
    }
    return user;
}

MumbleUser* MumblePlugin::Me()
{
    if (!state.serverSynced)
        return 0;
    return User(state.sessionId);
}

void MumblePlugin::SetMuted(uint userId, bool muted)
{
    if (!state.serverSynced || !network_)
        return;
    if (userId == state.sessionId)
        return;

    MumbleUser *user = User(userId);
    if (user)
    {
        if (user->isMuted == muted)
            return;
        
        user->isMuted = muted;
        user->EmitMuted();
        emit UserMuted(user, user->isMuted);
    }
    else
        LogError(LC + QString("Cannot mute user with id %1, no such user!").arg(userId));
}

void MumblePlugin::Mute(uint userId)
{
    SetMuted(userId, true);
}

void MumblePlugin::UnMute(uint userId)
{
    SetMuted(userId, false);
}

MumbleNetwork::ConnectionState MumblePlugin::State()
{
    return state.connectionState;
}

MumbleNetwork::NetworkMode MumblePlugin::NetworkMode()
{
    if (state.connectionState == MumbleConnected)
        return state.networkMode;
    else 
        return MumbleNetwork::NetworkModeNotSet;
}

void MumblePlugin::SetOutputAudioMuted(bool outputAudioMuted)
{
    if (state.serverSynced)
    {
        MumbleUser *me = Me();
        if (!me)
        {
            LogError(LC + "Could not find own user ptr to set audio output muted state!");
            return;
        }

        if (audio_ && state.outputAudioMuted != outputAudioMuted)
            audio_->SetOutputAudioMuted(outputAudioMuted);

        if (network_)
        {
            // Avoid unwanted network traffic and check value actually changed.
            if (me->isSelfMuted != outputAudioMuted)
            {
                MumbleProto::UserState message;
                message.set_session(state.sessionId);
                message.set_self_deaf(state.inputAudioMuted);
                message.set_self_mute(outputAudioMuted);
                network_->SendTCP(UserState, message);
            }
        }
        
        // Force emitting false speak state for own user.
        // If output is not muted this will be changed to true
        // on the first voice packets we send to the network.
        if (outputAudioMuted)
        {
            me->isSpeaking = true;
            me->SetAndEmitSpeaking(false);
        }
    }

    state.outputAudioMuted = outputAudioMuted;
}

void MumblePlugin::SetOutputAudioLoopBack(bool loopBack)
{
    state.outputAudioLoopBack = loopBack;
}

void MumblePlugin::SetInputAudioMuted(bool inputAudioMuted)
{
    if (state.serverSynced)
    {
        MumbleUser *me = Me();
        if (!me)
        {
            LogError(LC + "Could not find own user ptr to set audio input muted state!");
            return;
        }

        if (audio_ && state.inputAudioMuted != inputAudioMuted)
            audio_->SetInputAudioMuted(inputAudioMuted);
        
        if (network_)
        {
            // Avoid unwanted network traffic and check value actually changed.
            if (me->isSelfDeaf != inputAudioMuted)
            {
                MumbleProto::UserState message;
                message.set_session(state.sessionId);
                message.set_self_deaf(inputAudioMuted);
                message.set_self_mute(state.outputAudioMuted);
                network_->SendTCP(UserState, message);
            }
        }
        
        // Force everyone in our channel to not speaking state.
        // If they are sending audio and we input is not muted, the speaking
        // signal will be emitted next time we get audio packets from them.
        if (inputAudioMuted && me->Channel())
        {
            foreach(MumbleUser *peerUser, me->Channel()->users)
            {
                peerUser->isSpeaking = true;
                peerUser->SetAndEmitSpeaking(false);
            }
        }
    }

    state.inputAudioMuted = inputAudioMuted;
}

bool MumblePlugin::IsOutputAudioMuted()
{
    return state.outputAudioMuted;
}

bool MumblePlugin::IsInputAudioMuted()
{
    return state.inputAudioMuted;
}

void MumblePlugin::RunAudioWizard()
{
    if (framework_->IsHeadless())
        return;

    if (audioWizard && audioWizard->isVisible())
    {
        audioWizard->activateWindow();
        audioWizard->raise();
        return;
    }

    if (audioWizard)
        delete audioWizard;

    if (!state.serverSynced)
    {
        LogError(LC + "Audio wizard can only be shown while connected to a server!");
        return;
    }
    if (!audio_)
    {
        LogError(LC + "Audio wizard can't be shown, audio thread null!");
        return;
    }

    audioWizard = new AudioWizard(framework_, audio_->GetSettings());
    connect(audioWizard, SIGNAL(SettingsChanged(MumbleAudio::AudioSettings, bool)), SLOT(OnAudioSettingChanged(MumbleAudio::AudioSettings, bool)));
    connect(audioWizard, SIGNAL(destroyed(QObject *)), SLOT(AudioWizardDestroyed()));
}

void MumblePlugin::AudioWizardDestroyed()
{
    if (audio_ && state.serverSynced)
        audio_->ClearInputAudio(state.sessionId);
        
    emit AudioWizardClosed();
}

MumbleAudio::AudioSettings MumblePlugin::CurrentSettings()
{
    if (audio_ && state.serverSynced)
        return audio_->GetSettings();
    else
        return LoadSettings();
}

bool MumblePlugin::ApplySettings(MumbleAudio::AudioSettings settings, bool saveToConfig)
{
    bool appliedEither = false;
    
    // Apply to active audio processor
    if (audio_ && state.serverSynced)
    {
        audio_->ApplySettings(settings);
        appliedEither = true;
    }
    // Store to disk.    
    if (saveToConfig)
    {
        SaveSettings(settings);
        appliedEither = true;
    }
    
    return appliedEither;
}

void MumblePlugin::OnConnected(QString address, int port, QString username)
{
    state.connectionState = MumbleConnected;
    emit StateChange(state.connectionState);
    emit Connected(address, port, username);
}

void MumblePlugin::OnDisconnected(QString reason)
{
    Disconnect(reason);
}

void MumblePlugin::OnNetworkModeChange(MumbleNetwork::NetworkMode mode, QString reason)
{
    // Treat going back to TCP from UDP worth of a warning as it will make the connection poor.
    if (mode == MumbleNetwork::MumbleUDPMode)
        LogInfo(LC + "Network mode change: " + reason);
    else
        LogWarning(LC + "Network mode change: " + reason);
        
    state.networkMode = mode;
    emit NetworkModeChange(state.networkMode, reason);
}

void MumblePlugin::OnConnectionRejected(MumbleNetwork::RejectReason reasonType, QString reasonMessage)
{
    emit ConnectionRejected(reasonType, reasonMessage);
}

void MumblePlugin::OnPermissionDenied(MumbleNetwork::PermissionDeniedType denyReason, MumbleNetwork::ACLPermission permission, uint channelId, uint targetUserId, QString reason)
{
    if (denyReason == PermissionDeniedPermission)
    {
        MumbleChannel *channel = Channel(channelId);
        if (channel)
        {
            if (targetUserId == state.sessionId)
            {
                LogWarning(LC + QString("You were denied %1 privileges in %2").arg(MumbleNetwork::PermissionName(permission)).arg(channel->fullName));
                if (reason.isEmpty())
                    reason = QString("You were denied %1 privileges in %2").arg(MumbleNetwork::PermissionName(permission)).arg(channel->fullName);
            }
            else
            {
                MumbleUser *user = User(targetUserId);
                if (user)
                {    
                    LogWarning(LC + QString("%1 was denied %2 privileges in %3").arg(user->name).arg(MumbleNetwork::PermissionName(permission)).arg(channel->fullName));
                    if (reason.isEmpty())
                        reason = QString("You were denied %1 privileges in %2").arg(MumbleNetwork::PermissionName(permission)).arg(channel->fullName);
                }
            }
        }
    }
    else if (denyReason == PermissionDeniedSuperUser)
    {
        LogError(LC + "Permission denied: Cannot modify SuperUser.");
        if (reason.isEmpty())
            reason = "Cannot modify SuperUser.";
    }
    else if (denyReason == PermissionDeniedTextTooLong)
    {
        LogError(LC + "Permission denied: Text message too long.");
        if (reason.isEmpty())
            reason = "Text message too long.";
    }
    else if (denyReason == PermissionDeniedChannelFull)
    {
        LogError(LC + "Channel is full!");
        if (reason.isEmpty())
            reason = "Channel is full!";
    }

    emit PermissionDenied(denyReason, permission, channelId, targetUserId, reason);
}

void MumblePlugin::OnTextMessageReceived(bool isPrivate, QList<uint> channelIds, uint senderId, QString message)
{
    MumbleUser *sender = User(senderId);
    if (!sender)
        return;

    if (isPrivate)
        emit PrivateTextMessageReceived(sender, message);
    else
    {
        MumbleUser *me = Me();
        if (me)
        {
            foreach(uint channelId, channelIds)
            {
                if (me->channelId == channelId)
                {
                    emit ChannelTextMessageReceived(sender, message);
                    break;
                }
            }
        }
    }
}

void MumblePlugin::OnChannelUpdate(uint id, uint parentId, QString name, QString description)
{
    MumbleChannel *channel = Channel(id);
    bool isNew = channel == 0;
    if (!channel)
    {
        channel = new MumbleChannel(this);
        channels_.push_back(channel);
    }
    channel->id = id;
    channel->parentId = parentId;
    channel->name = name;
    channel->description = description;

    // Resolve full channel name. Murmur will always send the
    // channel tree in a order that this can be done.
    channel->fullName = channel->name;
    uint resolveId = channel->parentId;
    while (id != resolveId)
    {
        MumbleChannel *resolveChannel = Channel(resolveId);
        if (!resolveChannel)
            break;
        channel->fullName.prepend(resolveChannel->name + "/");
        if (resolveId == 0) // Root hit
            break;
        resolveId = resolveChannel->parentId;
    }

    if (isNew)
    {
        emit ChannelCreated(channel);
        emit ChannelsChanged(channels_);
    }
    else
    {
        emit ChannelUpdated(channel);
    }
}

void MumblePlugin::OnChannelRemoved(uint id)
{
    // Clean out children and dead channels
    Q_FOREVER
    {
        int childIndex = -1;
        for(int i=0; i<channels_.length(); i++)
        {
            if (channels_.at(i) == 0 || channels_.at(i)->parentId == id)
            {
                childIndex = i;
                break;
            }
        }
        if (childIndex != -1)
        {
            MumbleChannel *childChannel = channels_[childIndex];
            if (childChannel)
            {
                uint childId = childChannel->id;
                SAFE_DELETE(childChannel);
                emit ChannelRemoved(childId);
            }
            channels_.removeAt(childIndex);
        }
        else
            break;
    }

    // Remove main channel with id
    int index = -1;
    for(int i=0; i<channels_.length(); i++)
    {
        if (channels_.at(i)->id == id)
        {
            index = i;
            break;
        }
    }
    if (index != -1)
    {
        MumbleChannel *channel = channels_[index];   
        SAFE_DELETE(channel);
        channels_.removeAt(index);
        
        emit ChannelRemoved(id);
        emit ChannelsChanged(channels_);
    }
}

void MumblePlugin::OnUserUpdate(MumbleNetwork::MumbleUserState userState)
{
    if (!userState.hasId)
    {
        LogWarning(LC + "User update received without user id, cannot act on it!");
        return;
    }

    // Not yet synced, push/merge to pending
    if (!state.serverSynced)
    {
        bool pendingMerged = false;
        for(int iPend=0; iPend<pendingUsersStates_.size(); iPend++)
        {
            MumbleNetwork::MumbleUserState &pendingState = pendingUsersStates_[iPend];
            if (pendingState.id == userState.id)
            {
                pendingState.Merge(userState);
                pendingMerged = true;
                break;
            }
        }
        if (!pendingMerged)
            pendingUsersStates_.push_back(userState);
        return;
    }

    bool channelChange = false;
    bool mutedChange = false;
    bool selfMutedChange = false;
    bool selfDeafChange = false;

    MumbleUser *user = User(userState.id);
    bool isNew = (user == 0);
    
    // Created new user
    if (isNew)
    {
        channelChange = true;
        mutedChange = true;
        selfMutedChange = true;
        selfDeafChange = true;
        
        user = new MumbleUser(this);
        user->id = userState.id;
        user->channelId = userState.channelId;
        user->name = userState.name;
        user->comment = userState.comment;
        user->hash = userState.hash;
        user->isSelfMuted = userState.selfMuted;
        user->isSelfDeaf = userState.selfDeaf;
        user->isMe = userState.isMe;
    }
    // Existing user update
    else if (user)
    {
        // Detect changed properties for existing user
        if (userState.hasChannelId && user->channelId != userState.channelId)
        {
            // Handle previous channel user removal and signaling
            MumbleChannel *previousChannel = user->Channel();
            if (previousChannel)
            {
                if (previousChannel->RemoveUser(user->id))
                {
                    previousChannel->EmitUserLeft(user->id);
                    previousChannel->EmitUsersChanged();
                }
            }
            user->channelId = userState.channelId;
            channelChange = true;
        }
        if (userState.hasSelfMute && user->isSelfMuted != userState.selfMuted)
        {
            user->isSelfMuted = userState.selfMuted;
            selfMutedChange = true;
        }
        if (userState.hasSelfDeaf && user->isSelfDeaf != userState.selfDeaf)
        {
            user->isSelfDeaf = userState.selfDeaf;
            selfDeafChange = true;
        }
    }
    else
    {
        LogError(LC + "Error on MumbleUser creation/update for id " + QString::number(userState.id));
        return;
    }
    
    // Make sure we have the channel
    MumbleChannel *channel = Channel(user->channelId);
    if (!channel)
    {
        LogError(LC + "User creation/update detected unknown channel, aborting operation.");
        SAFE_DELETE(user);
        return;
    }

    // Emit user created/updated
    if (isNew)
    {
        if (user->isMe)
            emit MeCreated(user);
        emit UserCreated(user);
    }
    else
        emit UserUpdated(user);

    // Emit user property changes
    if (channelChange)
    {
        if (channel->AddUser(user))
        {
            if (user->isMe)
            {
                state.fullChannelName = channel->fullName;
                emit JoinedChannel(channel);
            }

            user->channelId = channel->id;
            user->EmitChannelChanged(channel);
            
            // For "me" also emit all other users from channel. So the 3rd party code listening does 
            // not have to listen to MumbleChannel::UserChanged or query MumbleChannel::users.
            // When JoinedChannel(MumbleChannel) is fired you can just hook to MumbleChannel::UserJoined
            // and you can be confident that this will give you all users currently in the channel.
            channel->EmitUserJoined(user);
            if (user->isMe)
            {
                foreach(MumbleUser *userIter, channel->users)
                    if (userIter && userIter != user)
                        channel->EmitUserJoined(userIter);
            }
            channel->EmitUsersChanged();
        }
    }
    if (mutedChange)
    {
        user->EmitMuted();
        emit UserMuted(user, user->isMuted);
    }
    if (selfMutedChange)
    {
        user->EmitSelfMuted();
        emit UserSelfMuted(user, user->isSelfMuted);
    }
    if (selfDeafChange)
    {
        user->EmitSelfDeaf();
        emit UserSelfDeaf(user, user->isSelfDeaf);
    }
}

void MumblePlugin::OnUserLeft(uint id, uint actorId, bool banned, bool kicked, QString reason)
{
    if (!state.serverSynced)
    {
        int index = -1;
        for(int i=0; i<pendingUsersStates_.count(); i++)
        {
            if (pendingUsersStates_.at(i).id == id)
            {
                index = i;
                break;
            }
        }
        if (index != -1)
            pendingUsersStates_.removeAt(index);
        return;
    }

    MumbleChannel *channel = ChannelForUser(id);
    if (channel)
    {
        MumbleUser *user = channel->User(id);
        if (user)
        {
            if (channel->RemoveUser(id))
            {
                channel->EmitUserLeft(id);
                channel->EmitUsersChanged();
            }
            SAFE_DELETE(user);
        }
    }
    else
        LogError(LC + "Could not find channel for disconnected user " + QString::number(id));

    if (audio_)
        audio_->ClearInputAudio(id);
}

void MumblePlugin::OnServerSynced(uint sessionId)
{
    state.serverSynced = true;
    state.sessionId = sessionId;

    QString pendingChannelJoin = state.fullChannelName;

    // Find and process own user ptr first, this is done because
    // MeCreated and JoinedChannel signals are fired inside OnUserUpdate.
    // We want to have these signals fire first so you can start to listen to
    // the channel signals (user joined/left) of your own channel and get full information with them.
    int meIndex = -1;
    for(int i=0; i<pendingUsersStates_.length(); i++)
    {
        MumbleNetwork::MumbleUserState &meState = pendingUsersStates_[i];
        if (meState.id == state.sessionId)
        {
            meState.isMe = true;
            OnUserUpdate(meState);
            meIndex = i;
            break;
        }
    }
    if (meIndex != -1)
        pendingUsersStates_.removeAt(meIndex);

    // Iterate and create/join other users on the server
    foreach(MumbleNetwork::MumbleUserState pendingState, pendingUsersStates_)
        OnUserUpdate(pendingState);
    pendingUsersStates_.clear();

    MumbleUser *me = Me();
    if (!me)
    {
        LogError(LC + "Could not find own user ptr after connected!");
        return;
    }

    MumbleChannel *myChannel = Channel(me->channelId);
    if (!myChannel)
    {
        LogError(LC + "Could not find own channel ptr after connected!");
        return;
    }
    if (state.fullChannelName != myChannel->fullName)
        LogError(LC + "Current channel mismatch after connected!");
    
    // Join potential pending channel
    if (pendingChannelJoin != myChannel->fullName)
        JoinChannel(pendingChannelJoin);
    
    // Send and setup audio state
    if (audio_)
    {
        me->SetAndEmitSpeaking(false);
        
        audio_->SetInputAudioMuted(state.inputAudioMuted);
        audio_->SetOutputAudioMuted(state.outputAudioMuted);
    }
    else 
        LogError(LC + "Audio thread null after connected!");

    if (network_)
    {
        MumbleProto::UserState message;
        message.set_session(state.sessionId);
        message.set_self_deaf(state.inputAudioMuted);
        message.set_self_mute(state.outputAudioMuted);
        network_->SendTCP(UserState, message);
    }
    else
        LogError(LC + "Network thread null after connected!");
}

void MumblePlugin::OnScriptEngineCreated(QScriptEngine *engine)
{
    RegisterMumblePluginMetaTypes(engine);
}

void MumblePlugin::UpdatePositionalInfo(VoicePacketInfo &packetInfo)
{
    packetInfo.isPositional = false;
    if (!state.outputPositional)
        return;
    if (!state.serverSynced)
        return;

    MumbleUser *me = Me();
    if (!me)
    {
        LogError(LC + "Cannot update own MumbleUser positional information, ptr is null!");
        return;
    }

    if (!framework_ || !framework_->Renderer() || !framework_->Renderer()->MainCameraScene())
    {
        if (me->isPositional == true)
            me->SetAndEmitPositional(false);
        return;
    }
    Scene *scene = framework_->Renderer()->MainCameraScene();

    Entity *activeListener = 0;
    EntityList listenerEnts = scene->GetEntitiesWithComponent(EC_SoundListener::TypeNameStatic());
    for(EntityList::const_iterator iter = listenerEnts.begin(); iter != listenerEnts.end(); ++iter)
    {
        Entity *ent = (*iter).get();
        if (ent)
        {
            EC_SoundListener *listener = ent->GetComponent<EC_SoundListener>().get();
            if (listener && listener->ParentEntity() && listener->active.Get())
            {
                activeListener = listener->ParentEntity();
                break;
            }
        }
    }

    if (activeListener)
    {
        EC_Placeable *placeable = activeListener->GetComponent<EC_Placeable>().get();
        if (placeable)
        {
            float3 worldPos = placeable->WorldPosition();
            
            // Change our positional state and emit signals.
            me->pos = worldPos;
            if (me->isPositional == false)
                me->SetAndEmitPositional(true);
            packetInfo.pos = worldPos;
            packetInfo.isPositional = true;
            return;
        }
    }    

    // If we get here no active EC_SoundListener could be found for our position.
    // If our user is in positional state, change that to false and emit signals
    if (me->isPositional == true)
        me->SetAndEmitPositional(false);
}

void MumblePlugin::OnAudioSettingChanged(MumbleAudio::AudioSettings settings, bool saveConfig)
{
    if (audio_)
    {
        if (saveConfig)
            SaveSettings(settings);
        state.outputPositional = settings.allowSendingPositional;
        audio_->ApplySettings(settings);

        MumbleUser *me = Me();
        if (me)
            me->SetAndEmitPositional(state.outputPositional);
    }
    else
        LogError(LC + "Audio wizard can't be shown, audio thread null!");
}

void MumblePlugin::OnFramesPerPacketChanged(const QStringList &params)
{
    if (!params.isEmpty())
    {
        bool ok = false;
        int framesPerPacket = params.first().toInt(&ok);
        if (ok)
        {
            if (audio_)
                audio_->ApplyFramesPerPacket(framesPerPacket);
            else
                LogError(LC + "Cannot set frames per packet, no active VOIP connection.");
        }
        else
            LogError(LC + "Cannot set frames per packet, failed to parse integer from " + params.first());
    }
    else
        LogError(LC + "Cannot set frames per packet, given parameter list is empty, needs single integer.");   
}

MumbleAudio::AudioSettings MumblePlugin::LoadSettings()
{
    ConfigAPI *config = framework_->Config();
    if (!config)
    {
        LogError(LC + "ConfigAPI null in LoadSettings(), returning default config!");
        return MumbleAudio::AudioSettings();
    }

    MumbleAudio::AudioSettings settings;
    
    ConfigData data("mumbleplugin", "output");
    if (config->HasValue(data, "quality"))
        settings.quality = (AudioQuality)config->Get(data, "quality").toInt();
    if (config->HasValue(data, "transmitmode"))
        settings.transmitMode = (TransmitMode)config->Get(data, "transmitmode").toInt();
    if (config->HasValue(data, "supression"))
        settings.suppression = config->Get(data, "supression").toInt();
    if (config->HasValue(data, "amplification"))
        settings.amplification = config->Get(data, "amplification").toInt();
    if (config->HasValue(data, "VADmin"))
        settings.VADmin = config->Get(data, "VADmin").toFloat();
    if (config->HasValue(data, "VADmax"))
        settings.VADmax = config->Get(data, "VADmax").toFloat();
    if (config->HasValue(data, "innerRange"))
        settings.innerRange = config->Get(data, "innerRange").toInt();
    if (config->HasValue(data, "outerRange"))
        settings.outerRange = config->Get(data, "outerRange").toInt();
    if (config->HasValue(data, "allowSendingPositional"))
        settings.allowSendingPositional = config->Get(data, "allowSendingPositional").toBool();
    if (config->HasValue(data, "allowReceivingPositional"))
        settings.allowReceivingPositional = config->Get(data, "allowReceivingPositional").toBool();
    if (config->HasValue(data, "recordingDevice"))
        settings.recordingDevice = config->Get(data, "recordingDevice").toString();
    return settings;
}

void MumblePlugin::SaveSettings(MumbleAudio::AudioSettings settings)
{
    ConfigAPI *config = framework_->Config();
    if (!config)
    {
        LogError(LC + "ConfigAPI null in SaveSettings()!");
        return;
    }

    ConfigData data("mumbleplugin", "output");
    config->Set(data, "quality", (int)settings.quality);
    config->Set(data, "transmitmode", (int)settings.transmitMode);
    config->Set(data, "supression", settings.suppression);
    config->Set(data, "amplification", settings.amplification);
    config->Set(data, "VADmin", settings.VADmin);
    config->Set(data, "VADmax", settings.VADmax);
    config->Set(data, "innerRange", settings.innerRange);
    config->Set(data, "outerRange", settings.outerRange);
    config->Set(data, "allowSendingPositional", settings.allowSendingPositional);
    config->Set(data, "allowReceivingPositional", settings.allowReceivingPositional);
    config->Set(data, "recordingDevice", settings.recordingDevice);
}

void MumblePlugin::EmitUserPositionalChanged(MumbleUser *user)
{
    emit UserPositionalChanged(user, user->isPositional);
}

void MumblePlugin::EmitUserSpeaking(MumbleUser *user)
{
    emit UserSpeaking(user, user->isSpeaking);
}

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *fw)
    {
        Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
        IModule *module = new MumblePlugin();
        fw->RegisterModule(module);
    }
}
