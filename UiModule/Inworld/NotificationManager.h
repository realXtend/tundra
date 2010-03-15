// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_NotificationManager_h
#define incl_UiModule_NotificationManager_h

#include "UiModuleApi.h"

#include <QObject>
#include <QGraphicsScene>
#include <QList>
#include <QRectF>

namespace Foundation
{
    class Framework;
}

namespace CoreUi
{
    class NotifyLabel;
}

namespace Ui
{
    class NotificationWidget;
}

namespace UiServices
{
    class InworldSceneController;

    class UI_MODULE_API NotificationManager : public QObject
    {

    Q_OBJECT

    public:
        NotificationManager(Foundation::Framework *framework, QGraphicsScene *scene);
        virtual ~NotificationManager();

    public slots:
        void ShowInformationString(const QString &text, int duration_msec = 5000);
        void DestroyNotifyLabel(CoreUi::NotifyLabel *notification);
        void SceneRectChanged(const QRectF &rect);


    private slots:
        void ResizeAndPositionNotifyArea();
        
    
    private:
        QList<CoreUi::NotifyLabel*> notifications_;
        QList<CoreUi::NotifyLabel*> visible_notifications_;

        QRectF notice_size_;
        QPointF start_point_;


        Foundation::Framework *framework_;
        QGraphicsScene *scene_;

    };
}

#endif // incl_UiModule_NotificationManager_h