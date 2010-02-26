// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiNotificationManager_h
#define incl_UiModule_UiNotificationManager_h

//#include "Framework.h"
#include "UiModuleApi.h"

#include <QObject>
#include <QGraphicsView>
#include <QTimer>

namespace Foundation
{
    class Framework;
}

namespace CoreUi
{
    class NotifyProxyWidget;
    class NotifyLabel;
}

namespace Ui
{
    class NotificationWidget;
}

namespace UiServices
{
    class UiSceneManager;

    class UI_MODULE_API UiNotificationManager : public QObject
    {

    Q_OBJECT

    public:
        UiNotificationManager(Foundation::Framework *framework, QGraphicsView *ui_view);
        virtual ~UiNotificationManager();

    public slots:
        void ShowInformationString(const QString &text, int duration_msec = 5000);
        void DestroyNotifyLabel(CoreUi::NotifyLabel *notification);

    private slots:
        void ResizeAndPositionNotifyArea();
    
    private:
        QWidget *notification_widget_;
        CoreUi::NotifyProxyWidget *notification_proxy_widget_;
        Ui::NotificationWidget *notification_ui_;

        Foundation::Framework *framework_;
        QGraphicsView *ui_view_;

        int visible_notifications_;
    };
}

#endif // incl_UiModule_UiNotificationManager_h