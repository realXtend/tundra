// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_NotificationLogWidget_h
#define incl_UiModule_NotificationLogWidget_h

#include "ui_NotificationLogWidget.h"


namespace CoreUi
{
    class NotificationLogWidget : public QWidget, private Ui::NotificationLogWidget
    {
    
    Q_OBJECT

    public:
        NotificationLogWidget(bool active, QWidget *content_widget, QDateTime timestamp, QString title_result = "", QString result = "", QIcon icon = QIcon());

    public slots:
        void Deactivate(QString title, QString result);
        QWidget *GetContentWidget() { return content_widget_; }

    private:
        QWidget *content_widget_;
    };
}

#endif