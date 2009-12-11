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
          confirmation_widget_(0),
          internal_v_layout_(0),
          internal_h_layout_(0)
    {
        video_session_ui_.setupUi(this);

        SessionStateChanged(video_session_->GetState());
        connect(video_session_, SIGNAL( StateChanged(Communication::VoiceSessionInterface::State) ),
                this, SLOT( SessionStateChanged(Communication::VoiceSessionInterface::State) ));
    }

    VideoSessionWidget::~VideoSessionWidget()
    {

    }

    void VideoSessionWidget::SessionStateChanged(Communication::VoiceSessionInterface::State new_state)
    {
        SAFE_DELETE(confirmation_widget_);
        SAFE_DELETE(internal_v_layout_);
        SAFE_DELETE(internal_h_layout_);

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

    void VideoSessionWidget::ShowConfirmationWidget()
    {
        confirmation_widget_ = new QWidget();
        internal_v_layout_ = new QVBoxLayout();
        internal_h_layout_ = new QHBoxLayout();
        QLabel *question_label = new QLabel(QString("%1 wants to start a video conversation with you").arg(his_name_));
        QPushButton *accept_button = new QPushButton("Accept", confirmation_widget_);
        QPushButton *decline_button = new QPushButton("Decline", confirmation_widget_);

        confirmation_widget_->setObjectName("confirmationWidget");
        confirmation_widget_->setStyleSheet("");
        confirmation_widget_->setStyleSheet(QString("QWidget#confirmationWidget { background-color: rgb(255,255,255); } QLabel { color: rgb(0,0,0); }"));

        connect(accept_button, SIGNAL( clicked() ), video_session_, SLOT( Accept() ));
        connect(decline_button, SIGNAL( clicked() ), video_session_, SLOT( Reject() ));

        internal_h_layout_->setSpacing(6);
        internal_h_layout_->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Preferred));
        internal_h_layout_->addWidget(question_label);
        internal_h_layout_->addWidget(accept_button);
        internal_h_layout_->addWidget(decline_button);
        internal_h_layout_->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Preferred));

        internal_v_layout_->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Preferred, QSizePolicy::Expanding));
        internal_v_layout_->addLayout(internal_h_layout_);
        internal_v_layout_->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Preferred, QSizePolicy::Expanding));

        video_session_ui_.mainVerticalLayout->insertLayout(0, internal_v_layout_);
    }

    void VideoSessionWidget::ShowVideoWidgets()
    {
        internal_h_layout_ = new QHBoxLayout();
        internal_h_layout_->setSpacing(6);
        internal_h_layout_->addWidget(video_session_->GetOwnVideo());
        internal_h_layout_->addWidget(video_session_->GetRemoteVideo());

        video_session_ui_.mainVerticalLayout->insertLayout(0, internal_h_layout_);
    }
}