// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "LoggingFunctions.h"

#include "Provider.h"
#include "MumbleVoipModule.h"
#include "MicrophoneAdjustmentWidget.h"

#include "Framework.h"
#include "TundraLogicModule.h"
#include "Client.h"
#include "Server.h"
#include "UiAPI.h"
#include "UiMainWindow.h"

#include <QSignalMapper>

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    Provider::Provider(Framework* framework, Settings* settings) :
        framework_(framework),
        settings_(settings),
        description_("Mumble VOIP Provider"),
        microphone_adjustment_widget_(0)
    {
        if (framework_->IsHeadless())
            return;

        session_.reset();

        TundraLogic::TundraLogicModule *tundraLogic = framework_->GetModule<TundraLogic::TundraLogicModule>();
        if (tundraLogic)
        {
            TundraLogic::Server *server = tundraLogic->GetServer().get();
            if (server && !server->IsAboutToStart())
            {
                TundraLogic::Client *client = tundraLogic->GetClient().get();
                if (client)
                {
                    /// \todo Change this to ResetSession once if need be, see the todo there. 
                    /// Also connected() should be connected to CreateSession if we gonna reset the ptr!
                    connect(client, SIGNAL(Disconnected()), SLOT(CloseSession()));
                }
            }
        }
        else
            LogError("MumbleVoip::Provider: Could not get TundraLogicModule, cannot initialize!");

        framework_->RegisterDynamicObject("mumblevoip", this);
        CreateSession();
    }

    Provider::~Provider()
    {
        if (session_)
            session_.reset();
    }

    /// IProvider overrides

    MumbleVoip::ISession* Provider::Session()
    {
        return session_.get();
    }

    bool Provider::HasSession()
    {
        return (session_.get() ? true : false);
    }

    QString& Provider::Description()
    {
        return description_;
    }

    // Provider

    void Provider::Update(f64 frametime)
    {
        if (session_)
            session_->Update(frametime);
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

    void Provider::ResetSession()
    {
        if (session_)
        {
            CloseSession();
            /// \todo I find this reset questionable (done on disconnected/login failed) as nothing is creating the session again on connected!
            //session_.reset();
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
}
