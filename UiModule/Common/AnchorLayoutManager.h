// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_AnchorLayoutManager_h
#define incl_UiModule_AnchorLayoutManager_h

#include "UiModuleFwd.h"

#include <QObject>

class QGraphicsScene;
class QGraphicsWidget;
class QGraphicsAnchorLayout;
class QGraphicsLayoutItem;
class QRectF;

namespace CoreUi
{
    class AnchorLayoutManager : public QObject
    {
        
    Q_OBJECT

    public:
        AnchorLayoutManager(QObject *parent, QGraphicsScene *scene);
        virtual ~AnchorLayoutManager();

    public slots:
        void AddCornerAnchor(QGraphicsLayoutItem *layout_item, Qt::Corner layout_anchor_corner, Qt::Corner widget_anchor_corner);
        void AddSideAnchor(QGraphicsLayoutItem *layout_item, Qt::Orientation orientation);

    private slots:
        void InitLayout();
        void AdjustLayoutContainer(const QRectF &new_rect);
        void CheckVisibility(QGraphicsLayoutItem *layout_item);

    private:
        QGraphicsScene *scene_;
        QGraphicsWidget *layout_container_;
        QGraphicsAnchorLayout *anchor_layout_;

    };
}
#endif