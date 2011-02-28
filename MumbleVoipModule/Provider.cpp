// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Provider.h"
#include "Session.h"
#include "MumbleVoipModule.h"
#include "ServerInfoProvider.h"
#include "MicrophoneAdjustmentWidget.h"
#include "EC_VoiceChannel.h"

#include "EventManager.h"
#include "NetworkEvents.h"
#include "UiServiceInterface.h"
#include "UiProxyWidget.h"
#include "SceneManager.h"
#include "TundraLogicModule.h"
#include "Client.h"
#include "Entity.h"

#include <QSignalMapper>

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    Provider::Provider(Foundation::Framework* framework, Settings* settings) :
        framework_(framework),
        description_("Mumble in-world voice"),
        session_(0),
        server_info_provider_(0),
        settings_(settings),
        microphone_adjustment_widget_(0),
        signal_mapper_(new QSignalMapper(this))
    {
        connect(signal_mapper_, SIGNAL(mapped(const QString &)),this, SLOT(ECVoiceChannelChanged(const QString &)));

        server_info_provider_ = new ServerInfoProvider(framework);
        connect(server_info_provider_, SIGNAL(MumbleServerInfoReceived(ServerInfo)), this, SLOT(OnMumbleServerInfoReceived(ServerInfo)) );

        networkstate_event_category_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");
        framework_event_category_ = framework_->GetEventManager()->QueryEventCategory("Framework");

        if (framework_ &&  framework_->GetServiceManager())
        {
            boost::shared_ptr<Communications::ServiceInterface> communication_service = framework_->GetServiceManager()->GetService<Communications::ServiceInterface>(Service::ST_Communications).lock();
            if (communication_service)
                communication_service->Register(*this);
        }

        connect(framework_, SIGNAL(SceneAdded(const QString &)), this, SLOT(OnSceneAdded(const QString &)));
    }

    Provider::~Provider()
    {
        SAFE_DELETE(session_);
        SAFE_DELETE(server_info_provider_);
    }

    void Provider::Update(f64 frametime)
    {
        if (session_)
            session_->Update(frametime);
        CheckChannelQueue();
    }
    
    bool Provider::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
    {
        if (server_info_provider_)
            server_info_provider_->HandleEvent(category_id, event_id, data);

        if (category_id == networkstate_event_category_)
        {
            switch (event_id)
            {
            case ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED:
            case ProtocolUtilities::Events::EVENT_CONNECTION_FAILED:
                CloseSession();
                SAFE_DELETE(session_);
                break;
            }
        }
        if (category_id == framework_event_category_)
        {
            switch (event_id)
            {
            case Foundation::WORLD_STREAM_READY:
                ProtocolUtilities::WorldStreamReadyEvent *event_data = dynamic_cast<ProtocolUtilities::WorldStreamReadyEvent *>(data);
                if (event_data)
                    world_stream_ = event_data->WorldStream;
                break;
            }
        }

        return false;
    }

    void Provider::CheckChannelQueue()
    {
        if (channel_queue_.isEmpty())
            return;
        if (GetUsername().length() == 0)
            return;

        EC_VoiceChannel* channel = channel_queue_.takeFirst();
        if (channel)
            AddECVoiceChannel(channel);
    }

    Communications::InWorldVoice::SessionInterface* Provider::Session()
    {
        return session_;
    }

    QString& Provider::Description()
    {
        return description_;
    }

    void Provider::OnMumbleServerInfoReceived(ServerInfo info)
    {
        ComponentPtr component = framework_->GetComponentManager()->CreateComponent("EC_VoiceChannel", "Public");
        EC_VoiceChannel* channel = dynamic_cast<EC_VoiceChannel*>(component.get());
        if (!channel)
            return;

        channel->setprotocol("mumble");
        channel->setserveraddress(info.server);
        channel->setversion(info.version);
        channel->setusername(info.user_name);
        channel->setserverpassword(info.password);
        channel->setchannelid(info.channel_id);
        channel->setchannelname("Public");
        channel->setenabled(true);

        Scene::EntityPtr e = framework_->GetDefaultWorldScene()->CreateEntity(framework_->GetDefaultWorldScene()->GetNextFreeId());
        e->AddComponent(component,  AttributeChange::LocalOnly);
    }

    void Provider::CloseSession()
    {
        if (session_)
            session_->Close();
        emit SessionUnavailable();
    }

    void Provider::CreateSession()
    {
        if (session_ && session_->GetState() == Session::STATE_CLOSED)
            SAFE_DELETE(session_) //! \todo USE SHARED PTR, SOMEONE MIGHT HAVE POINTER TO SESSION OBJECT !!!!

        if (!session_)
        {
            session_ = new MumbleVoip::Session(framework_, settings_);
            emit SessionAvailable();
        }
    }

    QList<QString> Provider::Statistics()
    {
        if (!session_)
        {
            QList<QString> lines;
            return lines;
        }
        else
            return session_->Statistics();
    }

    void Provider::ShowMicrophoneAdjustmentDialog()
    {
        UiServiceInterface *ui_service = framework_->GetService<UiServiceInterface>();

        if (!ui_service)
            return;

        if (microphone_adjustment_widget_)
            return;

        bool audio_sending_was_enabled = false;
        bool audio_receiving_was_enabled = false;
        if (session_)
        {
            audio_sending_was_enabled = session_->IsAudioSendingEnabled();
            audio_receiving_was_enabled = session_->IsAudioReceivingEnabled();

            session_->DisableAudioSending();
            session_->DisableAudioReceiving();
        }
        
        microphone_adjustment_widget_ = new MicrophoneAdjustmentWidget(framework_, settings_);
        microphone_adjustment_widget_->setWindowTitle("Local Test Mode");
        microphone_adjustment_widget_->setAttribute(Qt::WA_DeleteOnClose, true);
        microphone_adjustment_widget_->show();
        connect(microphone_adjustment_widget_, SIGNAL(destroyed()), this, SLOT(OnMicrophoneAdjustmentWidgetDestroyed()));

        if (audio_sending_was_enabled)
            connect(microphone_adjustment_widget_, SIGNAL(destroyed()), session_, SLOT(EnableAudioSending()));
        if (audio_receiving_was_enabled)
            connect(microphone_adjustment_widget_, SIGNAL(destroyed()), session_, SLOT(EnableAudioReceiving()));
    }

    void Provider::OnMicrophoneAdjustmentWidgetDestroyed()
    {
        microphone_adjustment_widget_ = 0;
    }

    void Provider::OnECAdded(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change)
    {
        if (comp->TypeName() != "EC_VoiceChannel")
            return;

        EC_VoiceChannel* channel = dynamic_cast<EC_VoiceChannel*>(comp);
        if (!channel)
            return;

        if (ec_voice_channels_.contains(channel))
            return;

        QString user_name = GetUsername();
        if (user_name.length() == 0)
        {
            channel_queue_.append(channel);
            return;
        }
        else
            AddECVoiceChannel(channel);
    }

    void Provider::AddECVoiceChannel(EC_VoiceChannel* channel)
    {
        ec_voice_channels_.append(channel);
        channel_names_[channel] = channel->getchannelname();

        if (!session_ || session_->GetState() != Communications::InWorldVoice::SessionInterface::STATE_OPEN)
            CreateSession();
       
        connect(channel, SIGNAL(destroyed(QObject*)), this, SLOT(OnECVoiceChannelDestroyed(QObject*)),Qt::UniqueConnection);
        connect(channel, SIGNAL(OnAttributeChanged(IAttribute *, AttributeChange::Type)), signal_mapper_, SLOT(map()));
        signal_mapper_->setMapping(channel,QString::number(reinterpret_cast<unsigned int>(channel)));

        if (session_->GetChannels().contains(channel->getchannelname()))
            channel->setenabled(false); // We do not want to create multiple channels with a same name

        ServerInfo server_info;
        server_info.server = channel->getserveraddress();
        server_info.version = channel->getversion();
        server_info.password = channel->getserverpassword();
        server_info.channel_id = channel->getchannelid();
        server_info.channel_name = channel->getchannelname();
        server_info.user_name = GetUsername();
        server_info.avatar_id = GetAvatarUuid();

        if (channel->getenabled())
            session_->AddChannel(channel->getchannelname(), server_info);

        if (session_->GetActiveChannel() == "")
            session_->SetActiveChannel(channel->Name());
    }

    void Provider::OnECVoiceChannelDestroyed(QObject* obj)
    {
        foreach(EC_VoiceChannel* channel, ec_voice_channels_)
        {
            if (channel == obj)
            {
                QString channel_name = channel_names_[channel];
                if (session_)
                    session_->RemoveChannel(channel_name);
                ec_voice_channels_.removeAll(channel);
                channel_names_.remove(channel);
                return;
            }
        }
    }

    void Provider::OnSceneAdded(const QString &name)
    {
        Scene::SceneManager* scene = framework_->GetScene(name).get();
        if (!scene)
            return;

        connect(scene, SIGNAL(ComponentAdded(Scene::Entity*, IComponent*, AttributeChange::Type)),
            SLOT(OnECAdded(Scene::Entity*, IComponent*, AttributeChange::Type)));
    }

    void Provider::ECVoiceChannelChanged(const QString &pointer)
    {
        if (!session_)
            return;

        /// @todo If user have edited the active channel -> close, reopen

        foreach(EC_VoiceChannel* channel, ec_voice_channels_)
        {
            if (QString::number(reinterpret_cast<unsigned int>(channel)) != pointer)
                continue;

            if (channel->getenabled() && !session_->GetChannels().contains(channel->getchannelname()))
            {
                ServerInfo server_info;
                server_info.server = channel->getserveraddress();
                server_info.version = channel->getversion();
                server_info.password = channel->getserverpassword();
                server_info.channel_id = channel->getchannelid();
                server_info.channel_name = channel->getchannelname();
                server_info.user_name = GetUsername();
                server_info.avatar_id = GetAvatarUuid();

                channel_names_[channel] = channel->getchannelname();
                session_->AddChannel(channel->getchannelname(), server_info);
            }
            if (!channel->getenabled())
            {
                session_->RemoveChannel(channel->getchannelname());
            }
        }
    }

    QString Provider::GetUsername()
    {
        if (tundra_logic_ && !tundra_logic_->IsServer())
            return tundra_logic_->GetClient()->GetLoginProperty("username");;
        return "";
    }

    QString Provider::GetAvatarUuid()
    {
        /// @todo: Get user's avatar entity uuid
        return "";
    }
    
    void Provider::PostInitialize()
    {
        tundra_logic_ = framework_->GetModuleManager()->GetModule<TundraLogic::TundraLogicModule>().lock();
        if (!tundra_logic_)
            RootLogError("MumbleVoip::Proviver: Could not get TundraLogicModule");
    }
} // MumbleVoip
