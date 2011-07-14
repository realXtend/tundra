// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Provider.h"
#include "Session.h"
#include "MumbleVoipModule.h"
#include "MicrophoneAdjustmentWidget.h"

#include "EventManager.h"
#include "NetworkEvents.h"
#include "UiProxyWidget.h"
#include "SceneManager.h"
#include "TundraLogicModule.h"
#include "Client.h"

#include "UiAPI.h"
#include "UiMainWindow.h"

#include <QSignalMapper>

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    Provider::Provider(Foundation::Framework* framework, Settings* settings) :
        framework_(framework),
        description_("Mumble in-world voice"),
        settings_(settings),
        microphone_adjustment_widget_(0)
    {
        networkstate_event_category_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");

        if (framework_ &&  framework_->GetServiceManager())
        {
            boost::shared_ptr<Communications::ServiceInterface> communication_service = framework_->GetServiceManager()->GetService<Communications::ServiceInterface>(Service::ST_Communications).lock();
            if (communication_service)
                communication_service->Register(*this);
        }

        framework_->RegisterDynamicObject("mumbleprovider", this);

        /// \note Automatically create session when provider is created. This could also be changed
        CreateSession();
    }

    Provider::~Provider()
    {
        if(session_)
            session_.reset();
    }

    void Provider::Update(f64 frametime)
    {
        if (session_)
            session_->Update(frametime);
    }
    
    bool Provider::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
    {
        if (category_id == networkstate_event_category_)
        {
            switch (event_id)
            {
            case ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED:
            case ProtocolUtilities::Events::EVENT_CONNECTION_FAILED:
                if(session_)
                {
                    CloseSession();
                    session_.reset();
                }
                break;
            }
        }
        return false;
    }

    Communications::InWorldVoice::SessionInterface* Provider::Session()
    {
        return session_.get();
    }

    QString& Provider::Description()
    {
        return description_;
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
            session_.reset();

        if (!session_)
        {
            session_ = MumbleVoip::SessionPtr(new MumbleVoip::Session(framework_, settings_));
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
        if (framework_->IsHeadless())
            return;

        if (microphone_adjustment_widget_)
        {
            if (!microphone_adjustment_widget_->isVisible())
                microphone_adjustment_widget_->show();
            return;
        }

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
        microphone_adjustment_widget_->setParent(framework_->Ui()->MainWindow());
        microphone_adjustment_widget_->setWindowFlags(Qt::Tool);
        microphone_adjustment_widget_->setWindowTitle("Voice Local Test Mode");
        microphone_adjustment_widget_->setAttribute(Qt::WA_DeleteOnClose, true);
        microphone_adjustment_widget_->show();

        connect(microphone_adjustment_widget_, SIGNAL(destroyed()), this, SLOT(OnMicrophoneAdjustmentWidgetDestroyed()));
        if (audio_sending_was_enabled)
            connect(microphone_adjustment_widget_, SIGNAL(destroyed()), session_.get(), SLOT(EnableAudioSending()));
        if (audio_receiving_was_enabled)
            connect(microphone_adjustment_widget_, SIGNAL(destroyed()), session_.get(), SLOT(EnableAudioReceiving()));
    }

    void Provider::OnMicrophoneAdjustmentWidgetDestroyed()
    {
        microphone_adjustment_widget_ = 0;
    }
    
    void Provider::PostInitialize()
    {
        tundra_logic_ = framework_->GetModuleManager()->GetModule<TundraLogic::TundraLogicModule>().lock();
        if (!tundra_logic_)
            RootLogError("MumbleVoip::Proviver: Could not get TundraLogicModule");
    }
} // MumbleVoip
