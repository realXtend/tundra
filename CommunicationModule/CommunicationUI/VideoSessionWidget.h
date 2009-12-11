// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_VideoSessionWidget_h
#define incl_Communication_VideoSessionWidget_h

#include "ui_VideoSessionWidget.h"
#include "interface.h"

namespace CommunicationUI
{
    class VideoSessionWidget : public QWidget
    {

    Q_OBJECT
    
    public:
        VideoSessionWidget(QWidget *parent, Communication::VoiceSessionInterface *video_session, QString &my_name, QString &his_name);
        virtual ~VideoSessionWidget();

    private:
        Ui::VideoSessionWidget video_session_ui_;
        Communication::VoiceSessionInterface *video_session_;
        
        QWidget *confirmation_widget_;
        QVBoxLayout *internal_v_layout_;
        QHBoxLayout *internal_h_layout_;
        QString my_name_; 
        QString his_name_;

    public slots:
        void SessionStateChanged(Communication::VoiceSessionInterface::State new_state);

    private slots:
        void ShowConfirmationWidget();
        void ShowVideoWidgets();
    };
}

#endif // incl_Communication_VideoSessionWidget_h