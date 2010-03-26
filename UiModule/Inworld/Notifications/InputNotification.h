// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_InputNotification_h
#define incl_UiModule_InputNotification_h

#include "NotificationBaseWidget.h"
#include "UiModuleApi.h"

class QPlainTextEdit;
class QLineEdit;
class QPushButton;

namespace UiServices
{
    class UI_MODULE_API InputNotification : public CoreUi::NotificationBaseWidget
    {

    Q_OBJECT

    public:
        InputNotification(QString message, QString button_title = "Answer", int hide_in_msec = 5000);

    signals:
        void InputRecieved(QString user_input);

    private slots:
        void ParseAndEmitInput();
    
    private:
        QPlainTextEdit *message_box_;
        QLineEdit *answer_line_edit_;
        QPushButton *answer_button_;

    };
}

#endif