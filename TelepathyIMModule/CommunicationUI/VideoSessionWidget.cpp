// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "VideoSessionWidget.h"
#include "UiDefines.h"
#include "VideoPlaybackWidgetInterface.h"
#include "ConnectionInterface.h"
#include "ChatSessionParticipantInterface.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "MemoryLeakCheck.h"

namespace CommunicationUI
{
    VideoSessionWidget::VideoSessionWidget(QWidget *parent, Communication::VoiceSessionInterface *video_session, QString &my_name, QString &his_name)
        : QWidget(parent),
          video_session_(video_session),
          my_name_(my_name),
          his_name_(his_name),
          internal_widget_(0),
          internal_v_layout_(0),
          internal_h_layout_(0),
          internal_v_layout_local_(0),
          internal_v_layout_remote_(0),
          local_video_(0),
          remote_video_(0),
          controls_local_widget_(new QWidget(this)),
          controls_remote_widget_(new QWidget(this)),
          main_view_visible_(false)

    {
        // Init all ui elements
        video_session_ui_.setupUi(this);

        controls_local_ui_.setupUi(controls_local_widget_);
        controls_local_ui_.videoCheckBox->setStyleSheet(QString("color:red"));
        controls_local_ui_.horizontalLayout->insertSpacerItem(2, new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed));
        controls_local_widget_->hide();  

        controls_remote_ui_.setupUi(controls_remote_widget_);
        controls_remote_ui_.videoCheckBox->setStyleSheet(QString("color:red"));
        controls_remote_ui_.horizontalLayout->insertSpacerItem(0, new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed));
        controls_remote_ui_.audioCheckBox->setEnabled(false);
        controls_remote_ui_.videoCheckBox->setEnabled(false);
        controls_remote_widget_->hide();
        
        // Update widget states
        SessionStateChanged(video_session_->GetState());
        AudioStreamStateChanged(video_session_->GetAudioStreamState());
        VideoStreamStateChanged(video_session_->GetVideoStreamState());
        UpdateLocalVideoControls(video_session_->IsSendingVideoData());
        UpdateLocalAudioControls(video_session_->IsSendingAudioData());
        UpdateRemoteVideoControls(video_session_->IsReceivingVideoData());
        UpdateRemoteAudioControls(video_session_->IsReceivingAudioData());

        // CLOSE TAB
        connect(video_session_ui_.closePushButton, SIGNAL( clicked() ),
                this, SLOT( CloseSession() ));
        // CONNECTION AND STREAM STATES
        connect(video_session_, SIGNAL( StateChanged(Communication::VoiceSessionInterface::State) ),
                this, SLOT( SessionStateChanged(Communication::VoiceSessionInterface::State) ));
        connect(video_session_, SIGNAL( AudioStreamStateChanged(Communication::VoiceSessionInterface::StreamState) ),
                this, SLOT( AudioStreamStateChanged(Communication::VoiceSessionInterface::StreamState) ));
        connect(video_session_, SIGNAL( VideoStreamStateChanged(Communication::VoiceSessionInterface::StreamState) ),
                this, SLOT( VideoStreamStateChanged(Communication::VoiceSessionInterface::StreamState) ));
        // AUDIO / VIDEO STATES
        connect(video_session_, SIGNAL( SendingVideoData(bool) ),
                this, SLOT( UpdateLocalVideoControls(bool) ));
        connect(video_session_, SIGNAL( SendingAudioData(bool) ),
                this, SLOT( UpdateLocalAudioControls(bool) ));
        connect(video_session_, SIGNAL( ReceivingVideoData(bool) ),
                this, SLOT( UpdateRemoteVideoControls(bool) ));
        connect(video_session_, SIGNAL( ReceivingAudioData(bool) ),
                this, SLOT( UpdateRemoteAudioControls(bool) ));
    }

    VideoSessionWidget::~VideoSessionWidget()
    {
        ClearContent();
    }

    void VideoSessionWidget::ClearContent()
    {
        if (internal_v_layout_local_ && local_video_)
        {
            local_video_->close();
            local_video_ = 0;
        }
        if (internal_v_layout_remote_ && remote_video_)
        {
            remote_video_->close();
            remote_video_ = 0;
        }
        SAFE_DELETE(internal_widget_);
    }

    void VideoSessionWidget::SessionStateChanged(Communication::VoiceSessionInterface::State new_state)
    {
        ClearContent();
        main_view_visible_ = false;

        switch (new_state)
        {
            case Communication::VoiceSessionInterface::STATE_OPEN:
            {
                video_session_ui_.mainVerticalLayout->setAlignment(video_session_ui_.horizontalLayout, Qt::AlignTop);
                ShowVideoWidgets();
                main_view_visible_ = true;
                video_session_ui_.connectionStatus->setText("Open");
                break;
            }
            case Communication::VoiceSessionInterface::STATE_CLOSED:
            {
                video_session_ui_.mainVerticalLayout->insertSpacerItem(0, new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding));
                video_session_ui_.connectionStatus->setText("This coversation has been closed");
                break;
            }
            case Communication::VoiceSessionInterface::STATE_ERROR:
            {
                QLabel *error_label = new QLabel(QString("Connection failed with reason %1").arg(video_session_->GetReason()),this);
                error_label->setAlignment(Qt::AlignCenter);
                video_session_ui_.infoBarFrame->hide();
                video_session_ui_.mainVerticalLayout->insertSpacerItem(0, new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding));
                video_session_ui_.mainVerticalLayout->insertWidget(1, error_label);
                video_session_ui_.mainVerticalLayout->insertSpacerItem(2, new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding));
                break;
            }
            case Communication::VoiceSessionInterface::STATE_INITIALIZING:
            {
                video_session_ui_.mainVerticalLayout->setAlignment(video_session_ui_.horizontalLayout, Qt::AlignBottom);
                video_session_ui_.connectionStatus->setText("Initializing...");
                break;
            }
            case Communication::VoiceSessionInterface::STATE_RINGING_LOCAL:
            {
                ShowConfirmationWidget();
                video_session_ui_.connectionStatus->setText("Waiting for your confirmation...");
                break;
            }
            case Communication::VoiceSessionInterface::STATE_RINGING_REMOTE:
            {
                video_session_ui_.connectionStatus->setText(QString("Waiting confirmation from %1").arg(his_name_));
                break;
            }
        }
    }

    void VideoSessionWidget::AudioStreamStateChanged(Communication::VoiceSessionInterface::StreamState new_state)
    {
        if (main_view_visible_)
        {
            switch (new_state)
            {
                case Communication::VoiceSessionInterface::SS_CONNECTING:
                    video_session_ui_.audioStatus->setText("Connecting");
                    controls_local_ui_.audioCheckBox->setText("Requesting Audio");
                    break;
                case Communication::VoiceSessionInterface::SS_CONNECTED:
                    video_session_ui_.audioStatus->setText("Connected");
                    controls_local_ui_.audioCheckBox->setText("Audio");
                    break;
                case Communication::VoiceSessionInterface::SS_DISCONNECTED:
                    video_session_ui_.audioStatus->setText("Disconnected");
                    controls_local_ui_.audioCheckBox->setText("Audio");
                    break;
            }
        }
    }

    void VideoSessionWidget::VideoStreamStateChanged(Communication::VoiceSessionInterface::StreamState new_state)
    {
        if (main_view_visible_)
        {
            switch (new_state)
            {
                case Communication::VoiceSessionInterface::SS_CONNECTING:
                    video_session_ui_.videoStatus->setText("Connecting");
                    controls_local_ui_.videoCheckBox->setText("Requesting Video");
                    break;
                case Communication::VoiceSessionInterface::SS_CONNECTED:
                    video_session_ui_.videoStatus->setText("Connected");
                    controls_local_ui_.videoCheckBox->setText("Video");
                    break;
                case Communication::VoiceSessionInterface::SS_DISCONNECTED:
                    video_session_ui_.videoStatus->setText("Disconnected");
                    controls_local_ui_.videoCheckBox->setText("Video");
                    break;
            }
        }
    }

    void VideoSessionWidget::LocalVideoStateChange(int state)
    {
        if (main_view_visible_)
        {
            bool enabled = false;
            if (state == Qt::Checked)
                enabled = true;
            else if (state == Qt::Unchecked)
                enabled = false;
            video_session_->SendVideoData(enabled);
            UpdateLocalVideoControls(enabled);
        }
    }

    void VideoSessionWidget::UpdateLocalVideoControls(bool state)
    {
        if (main_view_visible_)
        {
            controls_local_ui_.videoCheckBox->setChecked(state);
            if (state)
            {
                if (local_video_ && local_status_label_)
                {
                    if (video_session_->GetVideoStreamState() == Communication::VoiceSessionInterface::SS_CONNECTED)
                        local_status_label_->setText("Sending video");
                    else if (video_session_->GetVideoStreamState() == Communication::VoiceSessionInterface::SS_CONNECTING)
                        local_status_label_->setText("Requesting video...");
                    else
                        local_status_label_->setText("Could not open video stream");
                }
                controls_local_ui_.videoCheckBox->setStyleSheet(QString("color: green;"));
            }
            else
            {
                if (local_video_ && local_status_label_)
                    local_status_label_->setText("Preview of captured video");
                controls_local_ui_.videoCheckBox->setStyleSheet(QString("color: red;"));
            }
        }
    }

    void VideoSessionWidget::LocalAudioStateChange(int state)
    {
        if (main_view_visible_)
        {
            bool enabled = false;
            if (state == Qt::Checked)
                enabled = true;
            else if (state == Qt::Unchecked)
                enabled = false;
            video_session_->SendAudioData(enabled);
        }
    }

    void VideoSessionWidget::UpdateLocalAudioControls(bool state)
    {
        if (main_view_visible_)
        {
            controls_local_ui_.audioCheckBox->setChecked(state);
            if (state)
                controls_local_ui_.audioCheckBox->setStyleSheet(QString("color: green;"));
            else
                controls_local_ui_.audioCheckBox->setStyleSheet(QString("color: red;"));
        }
    }

    void VideoSessionWidget::UpdateRemoteVideoControls(bool state)
    {

        // Remote video and contols
        remote_video_ = video_session_->GetReceivedVideo();
        if (remote_video_)
        {
            remote_video_->setWindowTitle(QString("Receiving video %1)").arg(his_name_));
            remote_video_->setGeometry(338, 30, 322, 240);
            remote_video_->show();
        }

        if (main_view_visible_)
        {
            controls_remote_ui_.videoCheckBox->setChecked(state);
            if (state)
            {
                if (remote_video_ && remote_status_label_)
                    remote_status_label_->setText("Receiving video");
                controls_remote_ui_.videoCheckBox->setStyleSheet(QString("color: green;"));
            }
            else
            {
                if (remote_video_ && remote_status_label_)
                    remote_status_label_->setText("Friend is currently not sending video");
                controls_remote_ui_.videoCheckBox->setStyleSheet(QString("color: red;"));
            }
        }
    }

    void VideoSessionWidget::UpdateRemoteAudioControls(bool state)
    {
        if (main_view_visible_)
        {
            controls_remote_ui_.audioCheckBox->setChecked(state);
            if (state)
                controls_remote_ui_.audioCheckBox->setStyleSheet(QString("color: green;"));
            else
                controls_remote_ui_.audioCheckBox->setStyleSheet(QString("color: red;"));
        }
    }

    void VideoSessionWidget::ShowConfirmationWidget()
    {
        // Init widgets
        internal_widget_ = new QWidget();
        internal_v_layout_ = new QVBoxLayout(internal_widget_);
        internal_h_layout_ = new QHBoxLayout();
        QLabel *question_label = new QLabel(QString("%1 wants to start a video conversation with you").arg(his_name_));
        QPushButton *accept_button = new QPushButton("Accept", internal_widget_);
        QPushButton *decline_button = new QPushButton("Decline", internal_widget_);

        // Stylesheets for background and text color
        internal_widget_->setObjectName("confirmationWidget");
        internal_widget_->setStyleSheet("");
        internal_widget_->setStyleSheet(QString("QWidget#confirmationWidget { background-color: rgba(255,255,255,0); } QLabel { color: rgb(0,0,0); }"));

        // Add widgets to layouts
        internal_h_layout_->setSpacing(6);
        internal_h_layout_->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Preferred));
        internal_h_layout_->addWidget(question_label);
        internal_h_layout_->addWidget(accept_button);
        internal_h_layout_->addWidget(decline_button);
        internal_h_layout_->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Preferred));

        internal_v_layout_->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Preferred, QSizePolicy::Expanding));
        internal_v_layout_->addLayout(internal_h_layout_);
        internal_v_layout_->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Preferred, QSizePolicy::Expanding));

        // Add bottom layout to widget, insert widget at top of parent widgets layout
        internal_widget_->setLayout(internal_v_layout_);
        video_session_ui_.mainVerticalLayout->insertWidget(0, internal_widget_);
    
        // Connect signals
        connect(accept_button, SIGNAL( clicked() ), video_session_, SLOT( Accept() ));
        connect(decline_button, SIGNAL( clicked() ), video_session_, SLOT( Reject() ));
    }

    void VideoSessionWidget::ShowVideoWidgets()
    {
        // Init widgets
        internal_widget_ = new QWidget();
        internal_h_layout_ = new QHBoxLayout(internal_widget_);
        internal_v_layout_local_ = new QVBoxLayout();
        internal_v_layout_remote_ = new QVBoxLayout();

        // Local video and controls
        local_video_ = video_session_->GetLocallyCapturedVideo();
        if (local_video_)
        {
            local_video_->setWindowTitle(QString("Sending video %1").arg(my_name_));
            local_video_->setGeometry(7, 30, 322, 240);
            local_video_->show();
        }
        controls_local_widget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        local_status_label_ = new QLabel("Preview of captured video", this);
        local_status_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        local_status_label_->setAlignment(Qt::AlignLeft);
        local_status_label_->setStyleSheet(QString("font: bold; font-size: 10px; color: rgba(0,0,0,160);"));
        local_status_label_->setMaximumHeight(20);
        internal_v_layout_local_->setSpacing(3);
        internal_v_layout_local_->addWidget(local_status_label_);
        internal_v_layout_local_->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
        internal_v_layout_local_->addWidget(controls_local_widget_);

        // Remote video and contols
        remote_video_ = video_session_->GetReceivedVideo();
        if (remote_video_)
        {
            remote_video_->setWindowTitle(QString("Receiving video %1)").arg(his_name_));
            remote_video_->setGeometry(338, 30, 322, 240);
            remote_video_->show();
        }
        controls_remote_widget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        remote_status_label_ = new QLabel("Friend is currently not sending video", this);
        remote_status_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        remote_status_label_->setAlignment(Qt::AlignRight);
        remote_status_label_->setStyleSheet(QString("font: bold; font-size: 10px; color: rgba(0,0,0,160);"));
        remote_status_label_->setMaximumHeight(20);
        internal_v_layout_remote_->setSpacing(3);
        internal_v_layout_remote_->addWidget(remote_status_label_);
        internal_v_layout_remote_->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
        internal_v_layout_remote_->addWidget(controls_remote_widget_);

        // But our video containers to the main horizontal layout
        internal_h_layout_->setSpacing(6);
        internal_h_layout_->addLayout(internal_v_layout_local_);
        internal_h_layout_->addLayout(internal_v_layout_remote_);

        // Add to main widgets layout
        internal_widget_->setLayout(internal_h_layout_);
        video_session_ui_.mainVerticalLayout->insertWidget(0, internal_widget_);
        video_session_ui_.mainVerticalLayout->setStretch(0, 1);

        // Some stylesheets for consistent color theme
        // otherwise this will inherit transparent background from parent widget
        internal_widget_->setObjectName("videoSessionWidget");
        internal_widget_->setStyleSheet("");
        internal_widget_->setStyleSheet(QString("QWidget#videoSessionWidget { background-color: rgb(255,255,255); } QLabel { color: rgb(0,0,0); }"));
        controls_local_widget_->show();        
        controls_remote_widget_->show();

        // Connect checkboxes to control video and audio sending
        connect(controls_local_ui_.videoCheckBox, SIGNAL( stateChanged(int) ),
                this, SLOT( LocalVideoStateChange(int) ));
        connect(controls_local_ui_.audioCheckBox, SIGNAL( stateChanged(int) ),
                this, SLOT( LocalAudioStateChange(int) ));
    }

    void VideoSessionWidget::CloseSession()
    {
        ClearContent();
        video_session_->Close();
        emit Closed(his_name_);
    }
}