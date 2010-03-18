// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_NotificationBrowserWidget_h
#define incl_UiModule_NotificationBrowserWidget_h

#include <QGraphicsProxyWidget>
#include "ui_NotificationBrowserWidget.h"

namespace CoreUi
{
    class NotificationBaseWidget;

    class NotificationBrowserWidget : public QGraphicsProxyWidget, private Ui::NotificationBrowserWidget
    {
        
    Q_OBJECT

    public:
        NotificationBrowserWidget();

    public slots:
        void ShowNotifications(QList<NotificationBaseWidget *> all_notifications);
        void ClearAllContent();

    private slots:
        void MoveActiveToLog(QWidget *active_widget);

    private:
        QWidget *internal_widget_;
    };
}

#endif