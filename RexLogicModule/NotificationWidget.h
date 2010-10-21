// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_NotificationWidget_h 
#define incl_RexLogic_NotificationWidget_h 

#include "StableHeaders.h"
#include "RexLogicModule.h"
#include "InputEvents.h"
#include "Framework.h"

#include <QGraphicsView>
#include <QPushButton>
#include <QTimeLine>

namespace RexLogic
{

    class NotificationWidget : public QGraphicsView 
    {
        Q_OBJECT
    public:
        NotificationWidget(RexLogicModule *rex_logic, QWidget *parent = 0);

        void SetupScene();
        QString picture_name_;
        QString start_;
        QString end_;

        bool HandleInputEvent(event_id_t event_id, IEventData* data);

    public slots:
        void FocusOnObject();
        void updateStep(int);

    private:
        QGraphicsScene* scene_;
        QFrame *frame_pic_;
        QTimeLine *time_line_;

        RexLogicModule* rex_logic_;
        Foundation::Framework* framework_;
    };
}

#endif 