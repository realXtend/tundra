// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_QuestionNotification_h
#define incl_UiModule_QuestionNotification_h

#include "NotificationBaseWidget.h"
#include "UiModuleApi.h"

class QPlainTextEdit;
class QLineEdit;
class QPushButton;

namespace UiServices
{
    class UI_MODULE_API QuestionNotification : public CoreUi::NotificationBaseWidget
    {

    Q_OBJECT

    public:
        QuestionNotification(QString question, QString first_button_title = "Yes", QString second_button_title = "No", 
                             QString third_button_title = "", int hide_in_msec = 5000);

    signals:
        void QuestionAnswered(QString clicked_button_title);

    private slots:
        void FirstButtonClicked();
        void SecondButtonClicked();
        void ThirdButtonClicked();
        void EmitAnswer(QString clicked_button_title);

    private:
        QPlainTextEdit *question_box_;
        QPushButton *first_button_;
        QPushButton *second_button_;
        QPushButton *third_button_;

    };
}

#endif