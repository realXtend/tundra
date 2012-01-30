// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "MumblePlugin.h"
#include "MumbleNetwork.h"
#include "MumbleNetworkHandler.h"
#include "MumbleData.h"
#include "AudioProcessor.h"

#include "Mumble.pb.h"

#include "Framework.h"
#include "Profiler.h"
#include "Application.h"
#include "AudioAPI.h"
#include "ConsoleAPI.h"
#include "ConfigAPI.h"
#include "LoggingFunctions.h"

#include "IRenderer.h"
#include "Scene.h"
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
    audio_(0)
{
    state.Reset();
}

MumblePlugin::~MumblePlugin()
{
    QSslSocket::setDefaultCaCertificates(QList<QSslCertificate>()); // Hides a qt memory leak
}

void MumblePlugin::Initialize()
{
    RegisterMumblePluginMetaTypes();

    // Register custom QObject and enum meta types to created script engines.
    JavascriptModule *javascriptModule = framework_->GetModule<JavascriptModule>();
    if (javascriptModule)
        connect(javascriptModule, SIGNAL(ScriptEngineCreated(QScriptEngine*)), SLOT(OnScriptEngineCreated(QScriptEngine*)));
    else
        LogWarning(LC + "JavascriptModule not present, MumblePlugin usage from scripts will be limited!");

    framework_->RegisterDynamicObject("mumble", this);

    qobjTimerId_ = startTimer(15);

    /// @todo Remove everything below, was used for early development
    framework_->Console()->RegisterCommand("mumbleconnect", "", this, SLOT(DebugConnect()));
    framework_->Console()->RegisterCommand("mumbledisconnect", "", this, SLOT(Disconnect()));
    framework_->Console()->RegisterCommand("mumblejoin", "", this, SLOT(JoinChannel(QString)));
    framework_->Console()->RegisterCommand("mumblemute", "", this, SLOT(DebugMute()));
    framework_->Console()->RegisterCommand("mumbleunmute", "", this, SLOT(DebugUnMute()));
    framework_->Console()->RegisterCommand("mumbledeaf", "", this, SLOT(DebugDeaf()));
    framework_->Console()->RegisterCommand("mumbleundeaf", "", this, SLOT(DebugUnDeaf()));
    framework_->Console()->RegisterCommand("mumblewizard", "", this, SLOT(RunAudioWizard()));
    DebugConnect(); 
}

void MumblePlugin::Uninitialize()
{
    killTimer(qobjTimerId_);
    Disconnect("Client exiting.");
}

void MumblePlugin::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != qobjTimerId_)
        return;

    PROFILE(MumblePlugin_Update)

    // Audio processing
    if (audio_ && state.serverSynced && state.connectionState == MumbleConnected)
    {
        // Output audio
        if (!state.outputAudioMuted)
        {
            PROFILE(MumblePlugin_Update_ProcessOutputAudio)
            VoicePacketInfo packetInfo(audio_->ProcessOutputAudio());
            ELIFORP(MumblePlugin_Update_ProcessOutputAudio)
            if (!packetInfo.encodedFrames.isEmpty())
            {
                PROFILE(MumblePlugin_Update_ProcessOutputNetwork)
                if (network_)
                {
                    packetInfo.isLoopBack = state.outputAudioLoopBack;
                    if (!packetInfo.isLoopBack && audioWizard)
                        packetInfo.isLoopBack = true;
                    if (state.outputPositional)
                    {
                        // Sets packetInfo.isPositional false if no active EC_SoundListener is found.
                        // If found updates packetInfo and our MumbleUser position.
                        UpdatePositionalInfo(packetInfo); 
                    }
                    network_->SendVoicePacket(packetInfo);
                }
                else
                    LogError(LC + "Network ptr is null while sending out voice data!");
                ELIFORP(MumblePlugin_Update_ProcessOutputNetwork)
            }

            if (audioWizard)
                audioWizard->SetLevels(audio_->levelPeakMic, audio_->isSpeech);
        }
        else
            audio_->ClearOutputAudio();

        PROFILE(MumblePlugin_Update_ProcessInputAudio)
        // Input audio
        if (!state.inputAudioMuted)
        {
            MumbleChannel *c = ChannelForUser(state.sessionId);
            audio_->PlayInputAudio(c != 0 ? c->MutedUserIds() : QList<uint>());
        }
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
    Disconnect();

    state.address = address;
    state.port = (ushort)port;
    state.username = username;
    state.fullChannelName = fullChannelName;
    state.outputAudioMuted = outputAudioMuted;
    state.inputAudioMuted = inputAudioMuted;

    LogInfo(LC + "Connecting to " + state.FullHost() + " as \"" + state.username + "\"");

    audio_ = new MumbleAudio::AudioProcessor(framework_, LoadSettings());
    audio_->moveToThread(audio_);

    network_ = new MumbleNetworkHandler(state.address, state.port, state.username, password);
    network_->codecBitStreamVersion = audio_->CodecBitStreamVersion();
    network_->moveToThread(network_);

    // Handle signals from network thread to the main thread.
    connect(network_, SIGNAL(Connected(QString, int, QString)), SLOT(OnConnected(QString, int, QString)), Qt::QueuedConnection);
    connect(network_, SIGNAL(Disconnected(QString)), SLOT(OnDisconnected(QString)), Qt::QueuedConnection);
    connect(network_, SIGNAL(StateChange(MumbleNetwork::ConnectionState)), SLOT(OnStateChange(MumbleNetwork::ConnectionState)), Qt::QueuedConnection);
    connect(network_, SIGNAL(ServerSynced(uint)), SLOT(OnServerSynced(uint)), Qt::QueuedConnection);
    connect(network_, SIGNAL(NetworkModeChange(MumbleNetwork::NetworkMode, QString)), SLOT(OnNetworkModeChange(MumbleNetwork::NetworkMode, QString)), Qt::QueuedConnection);
    connect(network_, SIGNAL(ConnectionRejected(MumbleNetwork::RejectReason, QString)), SLOT(OnConnectionRejected(MumbleNetwork::RejectReason, QString)), Qt::QueuedConnection);

    connect(network_, SIGNAL(ChannelUpdate(uint, uint, QString, QString)), 
        SLOT(OnChannelUpdate(uint, uint, QString, QString)), Qt::QueuedConnection);
    connect(network_, SIGNAL(ChannelRemoved(uint)), SLOT(OnChannelRemoved(uint)), Qt::QueuedConnection);
    connect(network_, SIGNAL(UserUpdate(uint, uint, QString, QString, QString, bool, bool, bool)), 
        SLOT(OnUserUpdate(uint, uint, QString, QString, QString, bool, bool, bool)), Qt::QueuedConnection);
    connect(network_, SIGNAL(UserLeft(uint, uint, bool, bool, QString)), 
        SLOT(OnUserLeft(uint, uint, bool, bool, QString)), Qt::QueuedConnection);

    // Handle audio signals from network thread to audio thread.
    connect(network_, SIGNAL(AudioReceived(uint, QList<QByteArray>)), audio_, SLOT(OnAudioReceived(uint, QList<QByteArray>)), Qt::QueuedConnection);
    
    audio_->start();
    network_->start();
}

void MumblePlugin::Disconnect(QString reason)
{
    foreach(MumbleUser *pu, pendingUsers_)
        SAFE_DELETE(pu);
    pendingUsers_.clear();
    foreach(MumbleChannel *c, channels_)
        SAFE_DELETE(c); // Note that this frees all channel users as well.
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
        
        if (!reason.isEmpty())
            LogInfo(LC + "Disconnected from " + state.FullHost() + ": " + reason);
        else
            LogInfo(LC + "Disconnected from " + state.FullHost());
        emit Disconnected(reason);

        QCoreApplication::instance()->processEvents();
    }
    SAFE_DELETE(network_);

    state.Reset();
}

bool MumblePlugin::JoinChannel(QString fullName)
{
    if (state.connectionState != MumbleConnected || !network_)
    {
        LogError(LC + "Could not find channel with full name \"" + fullName + "\" not connected to a server.");
        return false;
    }

    MumbleChannel *channel = Channel(fullName);
    if (channel)
        return JoinChannel(channel->id);
    else
    {
        LogError(LC + "Could not find channel with full name \"" + fullName + "\" for join operation.");
        return false;
    }
}

bool MumblePlugin::JoinChannel(uint id)
{
    if (state.connectionState != MumbleConnected || !network_)
    {
        LogError(LC + "Could not find channel with id\"" + QString::number(id) + "\" for join operation.");
        return false;
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
        LogError(LC + "Could not find channel with id\"" + QString::number(id) + "\" for join operation.");
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
        MumbleUser *me = User(state.sessionId);
        if (!me)
        {
            LogError(LC + "Could not find own user ptr to set audio output muted state!");
            return;
        }

        if (audio_)
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
    }

    state.outputAudioMuted = outputAudioMuted;
}

void MumblePlugin::SetOutputAudioLoopBack(bool loopBack)
{
    state.outputAudioLoopBack = loopBack;
}

void MumblePlugin::SetOutputAudioPositional(bool positional)
{
    state.outputPositional = positional;
}

void MumblePlugin::SetInputAudioMuted(bool inputAudioMuted)
{
    if (state.serverSynced)
    {
        MumbleUser *me = User(state.sessionId);
        if (!me)
        {
            LogError(LC + "Could not find own user ptr to set audio input muted state!");
            return;
        }

        if (audio_)
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
    }

    state.inputAudioMuted = inputAudioMuted;
}

void MumblePlugin::RunAudioWizard()
{
    if (audioWizard && audioWizard->isVisible())
        return;

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

    audioWizard = new AudioWizard(audio_->GetSettings());
    connect(audioWizard, SIGNAL(SettingsChanged(MumbleAudio::AudioSettings, bool)), SLOT(OnAudioSettingChanged(MumbleAudio::AudioSettings, bool)));
}

void MumblePlugin::OnConnected(QString address, int port, QString username)
{
    emit Connected(address, port, username);
}

void MumblePlugin::OnDisconnected(QString reason)
{
    Disconnect(reason);
}

void MumblePlugin::OnStateChange(MumbleNetwork::ConnectionState newState)
{
    // Don't signal same state multiple times
    if (state.connectionState == newState)
        return;
    
    if (newState == MumbleConnecting)
        LogInfo(LC + "State changed to \"MumbleConnecting\"");
    else if (newState == MumbleConnected)
        LogInfo(LC + "State changed to \"MumbleConnected\"");
    else if (newState == MumbleDisconnected)
        LogInfo(LC + "State changed to \"MumbleDisconnected\"");

    MumbleNetwork::ConnectionState oldState = state.connectionState;
    state.connectionState = newState;
    emit StateChange(state.connectionState, oldState);
}

void MumblePlugin::OnNetworkModeChange(MumbleNetwork::NetworkMode mode, QString reason)
{
    LogInfo(LC + "Network mode change: " + reason);
    state.networkMode = mode;
    emit NetworkModeChange(state.networkMode, reason);
}

void MumblePlugin::OnConnectionRejected(MumbleNetwork::RejectReason reasonType, QString reasonMessage)
{
    emit ConnectionRejected(reasonType, reasonMessage);
    Disconnect(reasonMessage);
}

void MumblePlugin::OnChannelUpdate(uint id, uint parentId, QString name, QString description)
{
    MumbleChannel *channel = Channel(id);
    bool isNew = channel == 0;
    if (!channel)
    {
        channel = new MumbleChannel();
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
        LogInfo(LC + "Created channel: " + channel->toString());
        emit ChannelCreated(channel);
        emit ChannelsChanged(channels_);
    }
    else
    {
        LogInfo(LC + "Updated channel: " + channel->toString());
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
            MumbleChannel *childChannel = channels_.at(childIndex);
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
    if (index == -1)
        return;

    MumbleChannel *channel = channels_.at(index);   
    SAFE_DELETE(channel);
    channels_.removeAt(index);

    LogInfo(LC + "Removed channel " + QString::number(id) + " and its childen.");
    emit ChannelRemoved(id);
    emit ChannelsChanged(channels_);
}

void MumblePlugin::OnUserUpdate(uint id, uint channelId, QString name, QString comment, QString hash, bool selfMuted, bool selfDeaf, bool isMe)
{
    MumbleChannel *channel = Channel(channelId);
    if (!channel)
    {
        LogError(LC + "Failed to find channel " + QString::number(channelId) + " for user " + QString::number(id));
        return;
    }

    MumbleUser *user = channel->User(id);
    bool isNew = user == 0;
    if (!user)
    {
        ///@todo make sure this sessionId is no longer in other channels.
        user = new MumbleUser();
        user->id = id;
        user->channelId = channelId;
        if (!state.serverSynced)
            pendingUsers_.push_back(user);
        else
            channel->users.push_back(user);
    }
    if (user->id != id)
        LogError(LC + "Found user who's session id seems to have changed!");

    /// @todo inspect all properties and detect what changed, emit the appropriate signals.
    if (isNew || (user->name != name && !name.isEmpty()))
        user->name = name;
    if (isNew || (user->comment != name && !comment.isEmpty()))
        user->comment = comment;
    if (isNew || (user->hash != name && !hash.isEmpty()))
        user->hash = hash; // can be used to detect mute states when muted user changes channels

    user->isSelfMuted = selfMuted;
    user->isSelfDeaf = selfDeaf;
    user->isMe = isMe;

    if (!state.serverSynced)
        return;

    if (user->isMe)
    {
        state.fullChannelName = channel->fullName;
        emit MeJoinedChannel(channel);
    }

    if (isNew)
    {
        LogInfo(LC + "Created user: " + user->toString());
        emit UserUpdated(user);

        if (isNew)
            channel->EmitUsersChanged();
    }
    else
    {
        LogInfo(LC + "Updated user: " + user->toString());
        emit UserUpdated(user);
    }
}

void MumblePlugin::OnUserLeft(uint id, uint actorId, bool banned, bool kicked, QString reason)
{
    if (!state.serverSynced)
    {
        int index = -1;
        for(int i=0; i<pendingUsers_.count(); i++)
        {
            if (pendingUsers_.at(i)->id == id)
            {
                index = i;
                break;
            }
        }
        if (index != -1)
            pendingUsers_.removeAt(index);
        return;
    }

    MumbleChannel *channel = ChannelForUser(id);
    if (channel)
    {
        if (channel->User(id))
            LogInfo(LC + "User left channel \"" + channel->name + "\": " + channel->User(id)->toString());
        channel->RemoveUser(id);
        channel->EmitUsersChanged();
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

    foreach(MumbleUser *pu, pendingUsers_)
    {
        OnUserUpdate(pu->id, pu->channelId, pu->name, pu->comment, pu->hash, pu->isSelfMuted, pu->isSelfDeaf, state.sessionId == pu->id);
        SAFE_DELETE(pu);
    }
    pendingUsers_.clear();

    MumbleUser *me = User(state.sessionId);
    if (!me)
    {
        LogError(LC + "Could not find own user ptr after connected!");
        return;
    }
        
    emit MeCreated(me);

    MumbleChannel *myChannel = Channel(me->channelId);
    if (!myChannel)
    {
        LogError(LC + "Could not find own channel ptr after connected!");
        return;
    }
    if (state.fullChannelName != myChannel->fullName)
        LogWarning(LC + "Current channel mismatch after connected!");
    
    // Join potential pending channel
    if (pendingChannelJoin != myChannel->fullName)
        JoinChannel(pendingChannelJoin);
    
    // Send and setup audio state
    if (audio_)
    {
        audio_->SetInputAudioMuted(state.inputAudioMuted);
        audio_->SetOutputAudioMuted(state.outputAudioMuted);
    }
    else 
        LogError(LC + "Audio thread null after connected!");

    if (network_)
    {
        if (me->isSelfDeaf != state.inputAudioMuted || me->isSelfMuted != state.outputAudioMuted)
        {
            MumbleProto::UserState message;
            message.set_session(state.sessionId);
            message.set_self_deaf(state.inputAudioMuted);
            message.set_self_mute(state.outputAudioMuted);
            network_->SendTCP(UserState, message);
        }
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

    if (!state.serverSynced)
        return;

    MumbleUser *me = User(state.sessionId);
    if (!me)
    {
        LogError(LC + "Cannot update own MumbleUser positional information, ptr is null!");
        return;
    }
    me->isPositional = false;

    if (!framework_ || !framework_->Renderer())
        return;
    Scene *scene = framework_->Renderer()->MainCameraScene();
    if (!scene)
        return;

    Entity *activeListener = 0;
    EntityList listenerEnts = scene->GetEntitiesWithComponent(EC_SoundListener::TypeNameStatic());
    foreach(EntityPtr listenerEnt, listenerEnts)
    {
        EC_SoundListener *listener = listenerEnt->GetComponent<EC_SoundListener>().get();
        if (listener && listener->ParentEntity() && listener->active.Get())
        {
            activeListener = listener->ParentEntity();
            break;
        }
    }

    if (activeListener)
    {
        EC_Placeable *placeable = activeListener->GetComponent<EC_Placeable>().get();
        if (placeable)
        {
            float3 worldPos = placeable->WorldPosition();
            me->pos = worldPos;
            me->isPositional = true;
            packetInfo.pos = worldPos;
            packetInfo.isPositional = true;
        }
    }        
}

void MumblePlugin::OnAudioSettingChanged(MumbleAudio::AudioSettings settings, bool saveConfig)
{
    if (audio_)
    {
        if (saveConfig)
            SaveSettings(settings);
        audio_->ApplySettings(settings);
    }
    else
        LogError(LC + "Audio wizard can't be shown, audio thread null!");
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
