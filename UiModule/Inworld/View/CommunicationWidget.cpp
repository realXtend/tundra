// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "CommunicationWidget.h"
#include "UiProxyWidget.h"
#include <CommunicationsService.h>
#include <QWidget>
#include <QStackedLayout>
#include <QPlainTextEdit>
#include <QTimer>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QGraphicsScene>
#include <CommunicationsService.h>

namespace CoreUi
{

    VoiceStateWidget::VoiceStateWidget(QWidget * parent, Qt::WindowFlags f)
        : QPushButton(parent),
          state_(STATE_OFFLINE)
    {
        setMinimumSize(42,32);
        setObjectName("stateIndicatorWidget"); // There can be obly one instance of this class
        UpdateStyleSheet();
        update_timer_.start(VOICE_ACTIVITY_UPDATE_INTERVAL_MS_);
        update_timer_.setSingleShot(false);
        connect(&update_timer_, SIGNAL(timeout()), SLOT(UpdateVoiceActivity()) );
    }

    void VoiceStateWidget::setState(State state)
    {
        State old_state = state_;
        state_ = state;
        if (old_state == STATE_OFFLINE && state == STATE_ONLINE)
            voice_activity_ = 1.0; // for notificate user about voice tranmission set on
        UpdateStyleSheet();
        emit StateChanged();
    }

    VoiceStateWidget::State VoiceStateWidget::state() const
    {
        return state_;
    }

    void VoiceStateWidget::UpdateVoiceActivity()
    {
        voice_activity_ -= static_cast<double>(VOICE_ACTIVITY_UPDATE_INTERVAL_MS_)/VOICE_ACTIVITY_FADEOUT_MAX_MS_;
        if (voice_activity_ < 0)
            voice_activity_ = 0;
        UpdateStyleSheet();
    }

    void VoiceStateWidget::SetVoiceActivity(double activity)
    {
        if (activity > 1)
            activity = 1;
        if (activity < 0)
            activity = 0;
        if (activity > voice_activity_)
            voice_activity_ = activity;

        UpdateStyleSheet();
    }

    void VoiceStateWidget::UpdateStyleSheet()
    {
        if (state_ == STATE_OFFLINE)
        {
            setStyleSheet("QPushButton#stateIndicatorWidget { border: 0px; background-color: rgba(34,34,34,191); background-image: url('./data/ui/images/comm/status_offline.png'); background-position: top left; background-repeat: no-repeat; }");
            voice_activity_ = 0;
            return;
        }

        if (voice_activity_ > 0.60)
        {
            setStyleSheet("QPushButton#stateIndicatorWidget { border: 0px; background-color: rgba(34,34,34,191); background-image: url('./data/ui/images/comm/voice_5.png'); background-position: top left; background-repeat: no-repeat; }");
            return;
        }

        if (voice_activity_ > 0.30)
        {
            setStyleSheet("QPushButton#stateIndicatorWidget { border: 0px; background-color: rgba(34,34,34,191); background-image: url('./data/ui/images/comm/voice_3.png'); background-position: top left; background-repeat: no-repeat; }");
            return;
        }

        if (voice_activity_ > 0.5)
        {
            setStyleSheet("QPushButton#stateIndicatorWidget { border: 0px; background-color: rgba(34,34,34,191); background-image: url('./data/ui/images/comm/voice_1.png'); background-position: top left; background-repeat: no-repeat; }");
            return;
        }
        setStyleSheet("QPushButton#stateIndicatorWidget { border: 0px; background-color: rgba(34,34,34,191); background-image: url('./data/ui/images/comm/status_online.png'); background-position: top left; background-repeat: no-repeat; }");
    }

    VoiceUsersInfoWidget::VoiceUsersInfoWidget(QWidget* parent)
        : QPushButton(parent),
          count_label_(this),
          user_count_(0)
    {
        count_label_.setObjectName("voiceUserCount");
        setMinimumSize(64,32);
        setObjectName("voiceUsersInfoWidget"); // There can be obly one instance of this class
        UpdateStyleSheet();
    }

    void VoiceUsersInfoWidget::SetUsersCount(int count)
    {
        user_count_ = count;
        UpdateStyleSheet();
    }

    int VoiceUsersInfoWidget::UsersCount() const
    {
        return user_count_;
    }

    void VoiceUsersInfoWidget::SetVoiceActivity(double activity)
    {
        //! \todo: IMPELEMENT
    }

    void VoiceUsersInfoWidget::UpdateStyleSheet()
    {
       setStyleSheet("QPushButton#voiceUsersInfoWidget { border: 0px; background-color: rgba(34,34,34,191); background-image: url('./data/ui/images/comm/user.png'); background-position: top left; background-repeat: no-repeat; }");
       count_label_.setStyleSheet("QLabel#voiceUserCount { border: 0px; background-color:  rgba(34,34,34,191); background-position: top left; background-repeat: no-repeat; color: rgb(255,255,255); }");

       if (user_count_ == 0)
           count_label_.setText("0");
       else
           count_label_.setText(QString::number(user_count_));
    }

    CommunicationWidget::CommunicationWidget(Foundation::Framework* framework) :
        framework_(framework),
        QGraphicsProxyWidget(),
        internal_widget_(new QWidget()),
        current_controller_(0),
        im_proxy_(0),
        viewmode_(Normal),
        resizing_horizontal_(false),
        resizing_vertical_(false),
        in_world_speak_mode_on_(false),
        voice_state_widget_(0),
        voice_users_info_widget_(0),
        voice_users_widget_(0)
    {
        Initialise();
        ChangeView(viewmode_);
    }

    // Private

    void CommunicationWidget::Initialise()
    {
        setupUi(internal_widget_);
        setWidget(internal_widget_);
        imButton->setEnabled(false);

        // Stacked layout
        stacked_layout_ = new QStackedLayout();
        stacked_layout_->setMargin(0);
        contentContainerLayout->addLayout(stacked_layout_);

        // History view mode
        history_view_text_edit_ = new QPlainTextEdit(chatContentWidget);
        history_view_text_edit_->setReadOnly(true);
        history_view_text_edit_->setObjectName("historyViewTextEdit");
        history_view_text_edit_->setStyleSheet("QPlainTextEdit#historyViewTextEdit { background-color: rgba(34,34,34,191); border-radius: 7px; border: 1px solid rgba(255,255,255,50); }");
        history_view_text_edit_->setFont(QFont("Calibri", 11));
        stacked_layout_->addWidget(history_view_text_edit_);

        // Slim view mode
        normal_view_widget_ = new NormalChatViewWidget(chatContentWidget);
        stacked_layout_->addWidget(normal_view_widget_);

        stacked_layout_->setCurrentWidget(normal_view_widget_);

        connect(viewModeButton, SIGNAL( clicked() ), SLOT( ChangeViewPressed() ));
        connect(imButton, SIGNAL( clicked() ), SLOT( ToggleImWidget() ));
        connect(chatLineEdit, SIGNAL( returnPressed() ), SLOT( SendMessageRequested() ));
//        connect(voiceToggle, SIGNAL( clicked() ), SLOT(ToggleVoice() ) );

        HideVoiceControls();
    }

    void CommunicationWidget::ShowVoiceControls()
    {
        this->voiceLabel->show();
        this->voiceLeftBorder->show();
        if (voice_state_widget_)
            voice_state_widget_->show();
        if (voice_users_info_widget_)
            voice_users_info_widget_->show();
    }

    void CommunicationWidget::HideVoiceControls()
    {
        this->voiceLabel->hide();
        this->voiceLeftBorder->hide();
        if (voice_state_widget_)
            voice_state_widget_->hide();
        if (voice_users_info_widget_)
            voice_users_info_widget_->hide();
    }

    void CommunicationWidget::ChangeViewPressed()
    {
        switch (viewmode_)
        {
            case Normal:
                ChangeView(History);
                break;
            case History:
                ChangeView(Normal);
                break;
        }
    }

    void CommunicationWidget::ChangeView(ViewMode new_mode)
    {
        viewmode_ = new_mode; 
        switch (viewmode_)
        {
            case Normal:
                chatContentWidget->setStyleSheet("QWidget#chatContentWidget { background-color: transparent; border-top-left-radius: 0px; border-top-right-radius: 0px; }");
                viewModeButton->setStyleSheet("QPushButton#viewModeButton { background-image: url('./data/ui/images/chat/uibutton_HISTORY_normal.png'); }");
                stacked_layout_->setCurrentWidget(normal_view_widget_);
                break;
            case History:
                chatContentWidget->setStyleSheet("QWidget#chatContentWidget { background-color: rgba(34,34,34,191); border-top-left-radius: 7px; border-top-right-radius: 7px; }");
                viewModeButton->setStyleSheet("QPushButton#viewModeButton { background-image: url('./data/ui/images/chat/uibutton_HISTORY_click.png'); }");
                stacked_layout_->setCurrentWidget(history_view_text_edit_);
                break;
        }
    }

    void CommunicationWidget::ToggleImWidget()
    {
        if (im_proxy_)
        {
            if (!im_proxy_->isVisible())
                im_proxy_->show();
            else
                im_proxy_->AnimatedHide();
        }
    }

    void CommunicationWidget::ToggleVoice()
    {
        if (!in_world_voice_session_)
            return;

        in_world_speak_mode_on_ = !in_world_speak_mode_on_;
        if (in_world_speak_mode_on_)
            in_world_voice_session_->EnableAudioSending();
        else
            in_world_voice_session_->DisableAudioSending();
    }

    void CommunicationWidget::ToggleVoiceUsers()
    {
        if (!voice_users_widget_)
            return;

        if (voice_users_widget_)
            voice_users_widget_->show();
    }

    void CommunicationWidget::ShowIncomingMessage(bool self_sent_message, QString sender, QString timestamp, QString message)
    {
        // History view
        timestamp = timestamp.midRef(timestamp.indexOf(" ")+1).toString(); // Cut the fat from timestamp for now
        QString htmlcontent("<span style='color:grey'>[");
        htmlcontent.append(timestamp);
        if (!self_sent_message)
            htmlcontent.append("]</span> <span style='color:#0099FF;'>");
        else
            htmlcontent.append("]</span> <span style='color:#FF3330;'>");
        htmlcontent.append(sender);
        htmlcontent.append(": </span><span style='color:#EFEFEF;'>");
        htmlcontent.append(message);
        htmlcontent.append("</span>");
        history_view_text_edit_->appendHtml(htmlcontent);

        // Normal view
        if (!self_sent_message)
            normal_view_widget_->ShowChatMessage(self_sent_message, QString("%1: %2").arg(sender, message));
        else
            normal_view_widget_->ShowChatMessage(self_sent_message, QString("Me: %1").arg(message));
    }

    void CommunicationWidget::SendMessageRequested()
    {
        if (chatLineEdit->text().isEmpty())
            return;

        QString message = chatLineEdit->text();
        chatLineEdit->clear();
        emit SendMessageToServer(message);
    }

    // Protected

    void CommunicationWidget::hoverMoveEvent(QGraphicsSceneHoverEvent *mouse_hover_move_event)
    {
        if (stacked_layout_->currentWidget() == history_view_text_edit_)
        {
            qreal widget_press_pos_x = chatContentWidget->width() - mouse_hover_move_event->scenePos().x();
            qreal widget_press_pos_y = scene()->sceneRect().size().height() - mouse_hover_move_event->scenePos().y() - rect().height();
            if (widget_press_pos_x >= 0 && widget_press_pos_x <= 6)
            {
                if (!QApplication::overrideCursor())
                    QApplication::setOverrideCursor(QCursor(Qt::SizeHorCursor));
            }
            else if (widget_press_pos_y <= 0 && widget_press_pos_y >= -6 && widget_press_pos_x >= 0)
            {
                if (!QApplication::overrideCursor())
                    QApplication::setOverrideCursor(QCursor(Qt::SizeVerCursor));
            }
            else if (QApplication::overrideCursor())
                QApplication::restoreOverrideCursor();
        }
        else if (QApplication::overrideCursor())
            QApplication::restoreOverrideCursor();

        QGraphicsProxyWidget::hoverMoveEvent(mouse_hover_move_event);
    }

    void CommunicationWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *mouse_hover_leave_event)
    {
        if (QApplication::overrideCursor())
            QApplication::restoreOverrideCursor();
        QGraphicsProxyWidget::hoverLeaveEvent(mouse_hover_leave_event);
    }

    void CommunicationWidget::mousePressEvent(QGraphicsSceneMouseEvent *mouse_press_event)
    {   
        resizing_horizontal_ = false;
        resizing_vertical_ = false;
        if (stacked_layout_->currentWidget() == history_view_text_edit_)
        {
            qreal widget_press_pos_x = chatContentWidget->width() - mouse_press_event->scenePos().x();
            qreal widget_press_pos_y = scene()->sceneRect().size().height() - mouse_press_event->scenePos().y() - rect().height();
            if (widget_press_pos_x >= 0 && widget_press_pos_x <= 6)
            {
                mouse_press_event->accept();
                QApplication::setOverrideCursor(QCursor(Qt::SizeHorCursor));
                resizing_horizontal_ = true;
                return;
            }
            else if (widget_press_pos_y <= 0 && widget_press_pos_y >= -6 && widget_press_pos_x >= 0)
            {
                mouse_press_event->accept();
                QApplication::setOverrideCursor(QCursor(Qt::SizeVerCursor));
                resizing_vertical_ = true;
                return;
            }
        }
        QGraphicsProxyWidget::mousePressEvent(mouse_press_event);
    }

    void CommunicationWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *mouse_move_event)
    {
        if (resizing_horizontal_)
            chatContentWidget->setMinimumWidth(mouse_move_event->scenePos().x());
        else if (resizing_vertical_)
            chatContentWidget->setMinimumHeight(scene()->sceneRect().size().height() - mouse_move_event->scenePos().y() - chatControlsWidget->height());
        QGraphicsProxyWidget::mouseMoveEvent(mouse_move_event);
    }

    void CommunicationWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouse_release_event)
    {
        if (resizing_horizontal_ || resizing_vertical_)
        {
            resizing_horizontal_ = false;
            resizing_vertical_ = false;
            QApplication::restoreOverrideCursor();
        }
        QGraphicsProxyWidget::mouseReleaseEvent(mouse_release_event);
    }

    // Public

    void CommunicationWidget::UpdateController(QObject *controller)
    {
        // Disconnect previous
        if (current_controller_)
        {
            this->disconnect(SIGNAL( SendMessageToServer(const QString&) ));
            this->disconnect(current_controller_);
        }

        // Connect present controller
        current_controller_ = controller;
        connect(current_controller_, SIGNAL( MessageReceived(bool, QString, QString, QString) ),
                this, SLOT( ShowIncomingMessage(bool, QString, QString, QString) ));
        connect(this, SIGNAL( SendMessageToServer(const QString&) ), 
                current_controller_, SLOT( SendChatMessageToServer(const QString&) ));

        // Clear old ui messages from history view
        if (history_view_text_edit_)
            history_view_text_edit_->clear();
    }

    void CommunicationWidget::UpdateImWidget(UiServices::UiProxyWidget *im_proxy)
    {
        im_proxy_ = im_proxy;
        imButton->setEnabled(true);

        // Initialize In-World Voice
        if (framework_ &&  framework_->GetServiceManager())
        {
            boost::shared_ptr<Communications::ServiceInterface> comm = framework_->GetServiceManager()->GetService<Communications::ServiceInterface>(Foundation::Service::ST_Communications).lock();
            if (comm.get())
            {
                connect(comm.get(), SIGNAL(InWorldVoiceAvailable()), SLOT(InitializeInWorldVoice()) );
            }
        }
    }

    void CommunicationWidget::SetFocusToChat()
    {
        chatLineEdit->setFocus(Qt::MouseFocusReason);
    }

    void CommunicationWidget::InitializeInWorldVoice()
    {
        if (framework_ &&  framework_->GetServiceManager())
        {
            boost::shared_ptr<Communications::ServiceInterface> comm = framework_->GetServiceManager()->GetService<Communications::ServiceInterface>(Foundation::Service::ST_Communications).lock();
            if (comm.get())
            {
                in_world_voice_session_ = comm->InWorldVoiceSession();
                connect(in_world_voice_session_, SIGNAL(StartSendingAudio()), SLOT(UpdateInWorldVoiceIndicator()) );
                connect(in_world_voice_session_, SIGNAL(StopSendingAudio()), SLOT(UpdateInWorldVoiceIndicator()) );
                connect(in_world_voice_session_, SIGNAL(StateChanged(Communications::InWorldVoice::SessionInterface::State)), SLOT(UpdateInWorldVoiceIndicator()) );
                connect(in_world_voice_session_, SIGNAL(ParticipantJoined(Communications::InWorldVoice::ParticipantInterface*)), SLOT(UpdateInWorldVoiceIndicator()) );
                connect(in_world_voice_session_, SIGNAL(ParticipantLeft(Communications::InWorldVoice::ParticipantInterface*)), SLOT(UpdateInWorldVoiceIndicator()) );
                connect(in_world_voice_session_, SIGNAL(destroyed()), SLOT(UninitializeInWorldVoice()));
                connect(in_world_voice_session_, SIGNAL(SpeakerVoiceActivityChanged(double)), SLOT(UpdateInWorldVoiceIndicator()));
                connect(in_world_voice_session_, SIGNAL(StateChanged(Communications::InWorldVoice::SessionInterface::State)), SLOT(UpdateInWorldVoiceIndicator()));
            }
        }

        if (voice_state_widget_)
        {
            this->voiceLayoutH->removeWidget(voice_state_widget_);
            SAFE_DELETE(voice_state_widget_);
        }
        voice_state_widget_ = new VoiceStateWidget(0);
        connect(voice_state_widget_, SIGNAL( clicked() ), SLOT(ToggleVoice() ) );
        this->voiceLayoutH->addWidget(voice_state_widget_);
        voice_state_widget_->show();

        if (voice_users_info_widget_)
        {
            this->voiceLayoutH->removeWidget(voice_users_info_widget_);
            SAFE_DELETE(voice_users_info_widget_);
        }
        voice_users_info_widget_ = new VoiceUsersInfoWidget(0);
        this->voiceLayoutH->addWidget(voice_users_info_widget_);
        connect(voice_users_info_widget_, SIGNAL( clicked() ), SLOT(ToggleVoiceUsers() ) );
        voice_users_info_widget_->show();

//        voice_users_widget_ = new VoiceUsersWidget();
        UpdateInWorldVoiceIndicator();
        ShowVoiceControls();
    }

    void CommunicationWidget::UpdateInWorldVoiceIndicator()
    {
        if (!in_world_voice_session_)
            return;

        if (in_world_voice_session_->GetState() != Communications::InWorldVoice::SessionInterface::STATE_OPEN)
        {
            HideVoiceControls();
            return;
        }

        if (in_world_voice_session_->IsAudioSendingEnabled())
        {
            if (voice_state_widget_)
            {
                voice_state_widget_->setState(VoiceStateWidget::STATE_ONLINE);
                voice_state_widget_->SetVoiceActivity(in_world_voice_session_->SpeakerVoiceActivity());
            }
        }
        else
        {
            if (voice_state_widget_)
                voice_state_widget_->setState(VoiceStateWidget::STATE_OFFLINE);
        }

        if (voice_users_info_widget_)
            voice_users_info_widget_->SetUsersCount(in_world_voice_session_->Participants().count());
    }

    void CommunicationWidget::UninitializeInWorldVoice()
    {
        //! \todo: set indicator status -> disabled (hide voice controls)
        in_world_voice_session_ = 0;
    }

    // NormalChatViewWidget : QWidget

    NormalChatViewWidget::NormalChatViewWidget(QWidget *parent) :
        QWidget(parent)
    {
        setObjectName("normalChatViewWidget");
        setStyleSheet("QWidget#normalChatViewWidget { background-color: transparent; }");

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setMargin(0);
        layout->setSpacing(3);
        layout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding));
        setLayout(layout);
    }

    void NormalChatViewWidget::ShowChatMessage(bool own_message, QString message)
    {
        ChatLabel *chat_label = new ChatLabel(own_message, message);
        layout()->addWidget(chat_label);
        connect(chat_label, SIGNAL( DestroyMe(ChatLabel*) ), SLOT( RemoveChatLabel(ChatLabel*) ));
    }

    void NormalChatViewWidget::RemoveChatLabel(ChatLabel *label)
    {
        int index = layout()->indexOf(label);
        if (index != -1)
        {
            layout()->removeItem(layout()->itemAt(index));
            SAFE_DELETE(label);
            if (layout()->count() < 4)
                updateGeometry();
        }
    }

    // ChatLabel

    ChatLabel::ChatLabel(bool own_message, QString message) :
        QLabel(message)
    {
        setFont(QFont("Arial", 12));
        if (own_message)
            setStyleSheet("background-color: rgba(34,34,34,191); color: white; border-radius: 5px; padding: 3px;");
        else
            setStyleSheet("background-color: rgba(34,34,34,150); color: white; border-radius: 5px; padding: 3px;");
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        QTimer::singleShot(10000, this, SLOT(TimeOut()));
    }

    void ChatLabel::TimeOut()
    {
        emit DestroyMe(this);
    }



}
