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
        voice_activity_ = activity;
    }

    void VoiceUsersInfoWidget::UpdateStyleSheet()
    {
        if (voice_activity_ > 0)
        {
           setStyleSheet("QPushButton#voiceUsersInfoWidget { border: 0px; background-color: transparent; background-image: url('./data/ui/images/comm/user_green.png'); background-position: top left; background-repeat: no-repeat; }");
        }
        else
        {
           //setStyleSheet("QPushButton#voiceUsersInfoWidget { border: 0px; background-color: rgba(34,34,34,191); background-image: url('./data/ui/images/comm/user.png'); background-position: top left; background-repeat: no-repeat; }");
           setStyleSheet("QPushButton#voiceUsersInfoWidget { border: 0px; background-color: transparent; background-image: url('./data/ui/images/comm/user.png'); background-position: top left; background-repeat: no-repeat; }");
        }
        count_label_.setStyleSheet("QLabel#voiceUserCount { border: 0px; background-color: transparent; background-position: top left; background-repeat: no-repeat; color: rgb(255,255,255); }");

        if (user_count_ == 0)
            count_label_.setText("0");
        else
            count_label_.setText(QString::number(user_count_));
    }

} // CommUI
