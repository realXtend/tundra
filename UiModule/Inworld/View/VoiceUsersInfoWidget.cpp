/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   VoiceUsersInfoWidget.cpp
 *  @brief  Widget for voice user list information
 *          
 */
 
#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "VoiceUsersInfoWidget.h"
#include "CommunicationsService.h"

#include "DebugOperatorNew.h"

namespace CommUI
{
    VoiceUsersInfoWidget::VoiceUsersInfoWidget(QWidget* parent)
        : QPushButton(parent),
          count_label_(new QLabel(this)),
          user_count_(0)
    {
        count_label_->setObjectName("voiceUserCount");
        setMinimumSize(42,32);
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        setObjectName("voiceUsersInfoWidget"); // for stylesheets (There can be obly one instance of this class)
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
        voice_activity_ = activity;
    }

    void VoiceUsersInfoWidget::UpdateStyleSheet()
    {
        if (voice_activity_ > 0)
            setStyleSheet("QPushButton#voiceUsersInfoWidget { border: 0px; color: rgb(0,0,0); background-color: transparent; background-image: url('./data/ui/images/comm/user_green.png'); background-position: top left; background-repeat: no-repeat; }");
        else
           setStyleSheet("QPushButton#voiceUsersInfoWidget { border: 0px; color: rgb(0,0,0); background-color: transparent; background-image: url('./data/ui/images/comm/user.png'); background-position: top left; background-repeat: no-repeat; }");
        
        count_label_->setStyleSheet("QLabel#voiceUserCount { alignment: center center; padding-left: 25px; background-color: transparent; color: rgb(255,255,255); }");
        count_label_->setText(QString("%1").arg(user_count_));
    }

} // CommUI
