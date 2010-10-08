/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   VoiceUsersInfoWidget.h
 *  @brief  Widget for voice user list information
 *          
 */

#ifndef incl_UiModule_VoiceUsersInfoWidget_h
#define incl_UiModule_VoiceUsersInfoWidget_h

#include <QWidget>
#include <QLabel>
#include <QPushButton>

namespace Foundation
{
    class Framework;
}

namespace Communications
{
    namespace InWorldVoice
    {
        class SessionInterface;
        class ParticipantInterface;
    }
}

namespace CommUI
{
    //! Show user count and voice activity
    //!
    //! @todo: Move to out from UModule
    //! @todo Rename to VoiceUsersButton
    class VoiceUsersInfoWidget : public QPushButton
    {
        Q_OBJECT
    public:
        VoiceUsersInfoWidget(QWidget* parent = 0);
        virtual void SetUsersCount(int count);
        virtual int UsersCount() const;
        virtual void SetVoiceActivity(double activity);
    private:
        void UpdateStyleSheet();

        int user_count_;
        double voice_activity_;
        QLabel count_label_;
    };

} // namespace CommUI

#endif // incl_UiModule_VoiceUsersInfoWidget_h
