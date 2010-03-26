// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModule_UIGraphicsView_h
#define incl_QtModule_UIGraphicsView_h

#include <QGraphicsView>
#include "QtModuleApi.h"

namespace QtUI
{
    class QT_MODULE_API UIGraphicsView : public QGraphicsView
    {
        Q_OBJECT
    public:
        UIGraphicsView(QWidget* parent = 0);
        virtual ~UIGraphicsView();

    signals:
        void ViewResized(QResizeEvent* event);

    protected:
        void resizeEvent(QResizeEvent* event);


    };

}


#endif 