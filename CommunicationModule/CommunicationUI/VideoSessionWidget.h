// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_VideoSessionWidget_h
#define incl_Communication_VideoSessionWidget_h

#include "ui_VideoSessionWidget.h"
#include "ui_VideoSessionControlsWidget.h"
#include "VoiceSessionInterface.h"
//#include "interface.h"

#include "CommunicationModuleFwd.h"

namespace CommunicationUI
{
    class VideoSessionWidget : public QWidget
    {

    Q_OBJECT
    
    public:
        VideoSessionWidget(QWidget *parent, Communication::VoiceSessionInterface *video_session, QString &my_name, QString &his_name);
        virtual ~VideoSessionWidget();

        Communication::VideoPlaybackWidgetInterface *local_video_;
        Communication::VideoPlaybackWidgetInterface *remote_video_;

    private:
        Ui::VideoSessionWidget video_session_ui_;
        Ui::VideoSessionControlsWidget controls_local_ui_;
        Ui::VideoSessionControlsWidget controls_remote_ui_;
        Communication::VoiceSessionInterface *video_session_;
        
        QWidget *internal_widget_;
        QWidget *controls_local_widget_;
        QWidget *controls_remote_widget_;
        QVBoxLayout *internal_v_layout_;
        QVBoxLayout *internal_v_layout_local_;
        QVBoxLayout *internal_v_layout_remote_;
        QHBoxLayout *internal_h_layout_;
        QString my_name_; 
        QString his_name_;
        QLabel *local_status_label_;
        QLabel *remote_status_label_;

        bool main_view_visible_;

    public slots:
        void SessionStateChanged(Communication::VoiceSessionInterface::State new_state);
        void AudioStreamStateChanged(Communication::VoiceSessionInterface::StreamState new_state);
        void VideoStreamStateChanged(Communication::VoiceSessionInterface::StreamState new_state);

        void LocalVideoStateChange(int state);
        void UpdateLocalVideoControls(bool state);
        void LocalAudioStateChange(int state);
        void UpdateLocalAudioControls(bool state);
        void UpdateRemoteVideoControls(bool state);
        void UpdateRemoteAudioControls(bool state);

    private slots:
        void ClearContent();
        void ShowConfirmationWidget();
        void ShowVideoWidgets();
        void CloseSession();

    signals:
        void Closed(const QString &);
    };
}

#endif // incl_Communication_VideoSessionWidget_h