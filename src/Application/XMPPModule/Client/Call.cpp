// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Call.h"
#include "XMPPModule.h"

#include "Framework.h"
#include "LoggingFunctions.h"

#include "qxmpp/QXmppRtpChannel.h"
#include "qxmpp/QXmppJingleIq.h"
#include "qxmpp/QXmppUtils.h"

#include "MemoryLeakCheck.h"

namespace XMPP
{
Call::Call(Framework *framework, QXmppCall *call) :
    framework_(framework),
    call_(call)
{
    if(call->direction() == QXmppCall::IncomingDirection)
        state_ = Call::RingingState;
    else if(call->direction() == QXmppCall::OutgoingDirection)
        state_ = Call::ConnectingState;

    peer_jid_ = jidToBareJid(call->jid());

    bool check;
    check = connect(call_, SIGNAL(stateChanged(QXmppCall::State)), this, SLOT(HandleCallStateChanged(QXmppCall::State)));
    Q_ASSERT(check);

    check = connect(call_, SIGNAL(connected()), this, SLOT(HandleCallConnected()));
    Q_ASSERT(check);

    check = connect(call_, SIGNAL(finished()), this, SLOT(HandleCallTerminated()));
    Q_ASSERT(check);
}

Call::~Call()
{
    if(audio_channel_)
    {
        framework_->Audio()->Stop(audio_channel_);
        audio_channel_.reset();
    }
}

bool Call::Accept()
{
    if(state_ != Call::RingingState)
        return false;

    call_->accept();
    return true;
}

void Call::Hangup()
{
    call_->hangup();
    SetState(Call::FinishedState);
}

bool Call::Suspend()
{
    if(state_ == Call::FinishedState || state_ == Call::SuspendedState)
        return false;

    // QXmpp has no method of actually putting calls on hold like XEP-0167 defines
    // We'll just change the internal state of our call and discard received data.
    // We need to craft the messages ourselves unless this changes.

    SetState(Call::SuspendedState);
    return true;
}

bool Call::Resume()
{
    if(state_ != Call::SuspendedState)
        return false;

    SetState(Call::ActiveState);
    return true;
}

void Call::Update(f64 frametime)
{
    UNREFERENCED_PARAM(frametime);
    HandleOutboundVoice();
}

void Call::HandleCallTerminated()
{
    SetState(Call::FinishedState);
}

void Call::HandleCallConnected()
{
    if(!call_)
        return;

    Q_ASSERT(framework_);

    if(!framework_->Audio())
    {
        LogError("XMPPModule: Tundra sound API not initialized, cannot initialize voice call.");
        HandleCallTerminated();
    }

    QXmppRtpAudioChannel *channel = call_->audioChannel();

    bool stereo; /// \todo change this to global property of the call
    if(channel->payloadType().channels() == 2)
        stereo = true;
    else
        stereo = false;

    /// \todo change to something proper and define as a class member
    int buffer_size = 16/8*channel->payloadType().clockrate()*200/1000;

    framework_->Audio()->StartRecording("", channel->payloadType().clockrate(), true, stereo, buffer_size);

    bool ok = QObject::connect(channel, SIGNAL(readyRead()), this, SLOT(HandleInboundVoice()));
    Q_ASSERT(ok);
}

void Call::HandleInboundVoice()
{
    if(state_ == Call::DisconnectingState || state_ == Call::FinishedState)
        return;

    SoundBuffer buffer;
    QXmppRtpAudioChannel *channel = call_->audioChannel();
    QByteArray data = channel->read(channel->bytesAvailable());

    // For now, just discard the data when on hold (wastes downlink, proper implementation pending)
    if(state_ == Call::SuspendedState)
        return;

    buffer.data.resize(data.size());
    memcpy(&buffer.data[0], data.data(), data.size());

    buffer.frequency = channel->payloadType().clockrate();
    buffer.is16Bit = true;
    if(channel->payloadType().channels() == 2)
        buffer.stereo = true;
    else
        buffer.stereo = false;

    if(!audio_channel_)
        audio_channel_ = framework_->Audio()->PlaySoundBuffer(buffer, SoundChannel::Voice);
    else
        framework_->Audio()->PlaySoundBuffer(buffer, SoundChannel::Voice, audio_channel_);
}

void Call::HandleOutboundVoice()
{
    if(state_ != Call::ActiveState)
        return;

    QXmppRtpAudioChannel *channel = call_->audioChannel();
    QByteArray buffer;

    int buffer_size = (channel->payloadType().clockrate() * channel->payloadType().channels() * (16 / 8) * 160) / 1000;

    while (framework_->Audio()->GetRecordedSoundSize() > buffer_size)
    {
        buffer.resize(buffer_size);
        char *data = buffer.data();
        int bytes = framework_->Audio()->GetRecordedSoundData(data, buffer_size);
        buffer.resize(bytes);

        call_->audioChannel()->write(buffer);
    }
}


void Call::HandleCallStateChanged(QXmppCall::State state)
{
    switch(state)
    {
    case QXmppCall::ConnectingState:
        SetState(Call::ConnectingState);
        break;
    case QXmppCall::ActiveState:
        SetState(Call::ActiveState);
        break;
    case QXmppCall::DisconnectingState:
        SetState(Call::DisconnectingState);
        break;
    case QXmppCall::FinishedState:
        SetState(Call::FinishedState);
        break;
    default:
        break;
    }
}

void Call::SetState(Call::State state)
{
    state_ = state;
    QString state_string;

    switch(state_)
    {
    case Call::RingingState:
        state_string = "ringing";
        break;
    case Call::ConnectingState:
        state_string = "connecting";
        break;
    case Call::ActiveState:
        state_string = "active";
        break;
    case Call::SuspendedState:
        state_string = "suspended";
        break;
    case Call::DisconnectingState:
        state_string = "disconnecting";
        break;
    case Call::FinishedState:
        state_string = "finished";
        break;
    default:
        state_string = "unknown";
        break;
    }

    LogInfo("XMPPModule: Call with \"" + peer_jid_.toStdString() + "\" " + state_string.toStdString());
    emit StateChanged(state_);
}

} // end of namespace: XMPP
