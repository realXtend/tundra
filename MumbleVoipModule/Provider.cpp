// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Provider.h"
#include "Session.h"
#include "MumbleVoipModule.h"
#include "ServerInfoProvider.h"
#include "EventManager.h"
#include "NetworkEvents.h" // For network events
#include "MicrophoneAdjustmentWidget.h"
#include "UiServiceInterface.h"
#include "UiProxyWidget.h"

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    Provider::Provider(Foundation::Framework* framework, Settings* settings) :
        framework_(framework),
        description_("Mumble in-world voice"),
        session_(0),
        server_info_(0),
        server_info_provider_(0),
        settings_(settings),
        microphone_adjustment_widget_(0)
    {
        server_info_provider_ = new ServerInfoProvider(framework);
        connect(server_info_provider_, SIGNAL(MumbleServerInfoReceived(ServerInfo)), this, SLOT(OnMumbleServerInfoReceived(ServerInfo)) );

        networkstate_event_category_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");

        if (framework_ &&  framework_->GetServiceManager())
        {
            boost::shared_ptr<Communications::ServiceInterface> comm = framework_->GetServiceManager()->GetService<Communications::ServiceInterface>(Foundation::Service::ST_Communications).lock();
            if (comm.get())
            {
                comm->Register(*this);
            }
            return;
        }
    }

    Provider::~Provider()
    {
        SAFE_DELETE(session_);
        SAFE_DELETE(server_info_provider_);
        SAFE_DELETE(server_info_);
    }

    void Provider::Update(f64 frametime)
    {
        if (session_)
            session_->Update(frametime);
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
                break;
            }
        }

        return false;
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
        SAFE_DELETE(server_info_);
        server_info_ = new ServerInfo(info);


        if (session_ && session_->GetState() == Session::STATE_CLOSED)
            SAFE_DELETE(session_) //! \todo USE SHARED PTR, SOMEONE MIGHT HAVE POINTER TO SESSION OBJECT !!!!

        if (!session_ && server_info_)
        {
            session_ = new MumbleVoip::Session(framework_, *server_info_, settings_);
            emit SessionAvailable();
        }
    }

    void Provider::CloseSession()
    {
        if (session_)
            session_->Close();
        emit SessionUnavailable();
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


} // MumbleVoip
