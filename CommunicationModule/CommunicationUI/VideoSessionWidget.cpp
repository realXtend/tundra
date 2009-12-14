// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "VideoSessionWidget.h"
#include "UiDefines.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

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
          local_video_(new QWidget),
          remote_video_(new QWidget),
          controls_local_widget_(new QWidget(this)),
          controls_remote_widget_(new QWidget(this))
    {
        // Init all ui elements
        video_session_ui_.setupUi(this);

        controls_local_ui_.setupUi(controls_local_widget_);
        controls_local_ui_.horizontalLayout->insertSpacerItem(2, new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed));
        controls_local_widget_->hide();  

        controls_remote_ui_.setupUi(controls_remote_widget_);
        controls_remote_ui_.horizontalLayout->insertSpacerItem(0, new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed));
        controls_remote_ui_.audioCheckBox->setEnabled(false);
        controls_remote_ui_.videoCheckBox->setEnabled(false);
        controls_remote_widget_->hide();
        
        // These do nothing yet
        local_video_->hide();
        remote_video_->hide();

        // Update widget states
        SessionStateChanged(video_session_->GetState());
        UpdateLocalVideoControls(video_session_->GetVideoOutEnabled());
        UpdateLocalAudioControls(video_session_->GetAudioOutEnabled());
        UpdateRemoteVideoControls(video_session_->GetVideoInEnabled());
        UpdateRemoteAudioControls(video_session_->GetAudioInEnabled());

        // Connect signals
        connect(video_session_ui_.closePushButton, SIGNAL( clicked() ),
                this, SLOT( CloseSession() ));
        connect(video_session_, SIGNAL( StateChanged(Communication::VoiceSessionInterface::State) ),
                this, SLOT( SessionStateChanged(Communication::VoiceSessionInterface::State) ));
        connect(video_session_, SIGNAL( VideoOutEnabledStateChanged(bool) ),
                this, SLOT( UpdateLocalVideoControls(bool) ));
        connect(video_session_, SIGNAL( AudioOutEnabledStateChanged(bool) ),
                this, SLOT( UpdateLocalAudioControls(bool) ));
        connect(video_session_, SIGNAL( VideoInEnabledStateChanged(bool) ),
                this, SLOT( UpdateRemoteVideoControls(bool) ));
        connect(video_session_, SIGNAL( AudioInEnabledStateChanged(bool) ),
                this, SLOT( UpdateRemoteAudioControls(bool) ));
    }

    VideoSessionWidget::~VideoSessionWidget()
    {
        SAFE_DELETE(internal_widget_);
    }

    void VideoSessionWidget::SessionStateChanged(Communication::VoiceSessionInterface::State new_state)
    {    
        SAFE_DELETE(internal_widget_);

        switch (new_state)
        {
            case Communication::VoiceSessionInterface::STATE_OPEN:
                ShowVideoWidgets();
                video_session_ui_.statusLabel->setText("Open");
                break;
            case Communication::VoiceSessionInterface::STATE_CLOSED:
                video_session_ui_.statusLabel->setText("Closed");
                break;
            case Communication::VoiceSessionInterface::STATE_ERROR:
                video_session_ui_.mainVerticalLayout->insertSpacerItem(0, new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding));
                video_session_ui_.statusLabel->setText("Failed");
                break;
            case Communication::VoiceSessionInterface::STATE_INITIALIZING:
                video_session_ui_.statusLabel->setText("Initializing...");
                break;
            case Communication::VoiceSessionInterface::STATE_RINGING_LOCAL:
                ShowConfirmationWidget();
                video_session_ui_.statusLabel->setText("Waiting for your confirmation...");
                break;
            case Communication::VoiceSessionInterface::STATE_RINGING_REMOTE:
                video_session_ui_.statusLabel->setText(QString("Waiting confirmation from %1").arg(his_name_));
                break;
        }
    }

    void VideoSessionWidget::LocalVideoStateChange(int state)
    {
        bool enabled = false;
        if (state == Qt::Checked)
            enabled = true;
        else if (state == Qt::Unchecked)
            enabled = false;   
        video_session_->SetVideoOutEnabled(enabled);
        UpdateLocalVideoControls(enabled);
    }

    void VideoSessionWidget::UpdateLocalVideoControls(bool state)
    {
        controls_local_ui_.videoCheckBox->setChecked(state);
        if (state)
            controls_local_ui_.videoCheckBox->setStyleSheet(QString("color: green;"));
        else
            controls_local_ui_.videoCheckBox->setStyleSheet(QString("color: red;"));
    }

    void VideoSessionWidget::LocalAudioStateChange(int state)
    {
        bool enabled = false;
        if (state == Qt::Checked)
            enabled = true;
        else if (state == Qt::Unchecked)
            enabled = false;
        video_session_->SetAudioOutEnabled(enabled);
        UpdateLocalAudioControls(enabled);
    }

    void VideoSessionWidget::UpdateLocalAudioControls(bool state)
    {
        controls_local_ui_.audioCheckBox->setChecked(state);
        if (state)
            controls_local_ui_.audioCheckBox->setStyleSheet(QString("color: green;"));
        else
            controls_local_ui_.audioCheckBox->setStyleSheet(QString("color: red;"));
    }

    void VideoSessionWidget::UpdateRemoteVideoControls(bool state)
    {
        controls_remote_ui_.videoCheckBox->setChecked(state);
        if (state)
            controls_remote_ui_.videoCheckBox->setStyleSheet(QString("color: green;"));
        else
            controls_remote_ui_.videoCheckBox->setStyleSheet(QString("color: red;"));
    }

    void VideoSessionWidget::UpdateRemoteAudioControls(bool state)
    {
        controls_remote_ui_.audioCheckBox->setChecked(state);
        if (state)
            controls_remote_ui_.audioCheckBox->setStyleSheet(QString("color: green;"));
        else
            controls_remote_ui_.audioCheckBox->setStyleSheet(QString("color: red;"));
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
        internal_widget_->setStyleSheet(QString("QWidget#confirmationWidget { background-color: rgb(255,255,255); } QLabel { color: rgb(0,0,0); }"));

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
        QVBoxLayout *internal_v_layout_local_ = new QVBoxLayout();
        QVBoxLayout *internal_v_layout_remote_ = new QVBoxLayout();

        // Local video and controls
        controls_local_widget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        QLabel *sending_label = new QLabel("Sending", this);
        sending_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sending_label->setAlignment(Qt::AlignCenter);
        sending_label->setStyleSheet(QString("font: 12pt 'Estrangelo Edessa'; color: rgb(69, 159, 255);"));
        internal_v_layout_local_->addWidget(sending_label);
        internal_v_layout_local_->addWidget((QWidget *)(video_session_->GetOwnVideo()));
        internal_v_layout_local_->addWidget(controls_local_widget_);

        // Remote video and contols
        controls_remote_widget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        QLabel *receiving_label = new QLabel("Receiving", this);
        receiving_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        receiving_label->setAlignment(Qt::AlignCenter);
        receiving_label->setStyleSheet(QString("font: 12pt 'Estrangelo Edessa'; color: rgb(69, 159, 255);"));
        internal_v_layout_remote_->addWidget(receiving_label);
        internal_v_layout_remote_->addWidget((QWidget *)(video_session_->GetRemoteVideo()));
        internal_v_layout_remote_->addWidget(controls_remote_widget_);

        // But our video containers to the main horizontal layout
        internal_h_layout_->setSpacing(6);
        internal_h_layout_->addLayout(internal_v_layout_local_);
        internal_h_layout_->addLayout(internal_v_layout_remote_);

        // Add to main widgets layout
        internal_widget_->setLayout(internal_h_layout_);
        video_session_ui_.mainVerticalLayout->insertWidget(0, internal_widget_);

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
        video_session_->Close();
        emit Closed(his_name_);
    }
}