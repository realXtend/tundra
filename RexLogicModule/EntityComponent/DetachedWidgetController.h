/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
*/
#ifndef incl_DetachedWidgetController_h
#define incl_DetachedWidgetController_h

#include "ui_DetachedWidget.h"
#include <QWidget>
#include <CoreTypes.h>
class QPushButton;
class QMouseEvent;
class QEvent;
namespace RexLogic
{
    class DetachedWidgetController: public QWidget, private Ui::DetachedWidget
    {
        Q_OBJECT

    public:
        DetachedWidgetController();
        bool eventFilter(QObject *obj, QEvent *event);

        virtual ~DetachedWidgetController();
        void AddButton(QPushButton *button);
        void SetText(QString text);
        void DisableButtons(bool val);

    protected:
        void mousePressEvent ( QMouseEvent * event ) ;
        void mouseReleaseEvent ( QMouseEvent * event );
        void mouseMoveEvent ( QMouseEvent * event ) ;

    private:
        bool mouse_drag_;
        QPoint prev_mouse_pos;
        qreal button_text_padding_;
    };
}

#endif