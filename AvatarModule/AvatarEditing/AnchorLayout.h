// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Avatar_AnchorLayout_h
#define incl_Avatar_AnchorLayout_h

#include <QObject>
#include <QGraphicsAnchorLayout>

class QGraphicsScene;
class QGraphicsWidget;
class QGraphicsLayoutItem;
class QRectF;

namespace Avatar
{
    class AnchorLayout : public QObject, public QGraphicsAnchorLayout
    {
        
    Q_OBJECT

    public:
        AnchorLayout(QObject *parent, QGraphicsScene *scene);
        virtual ~AnchorLayout();

    public slots:
        QGraphicsScene *GetScene() const;

        void AddCornerAnchor(QGraphicsLayoutItem *layout_item, Qt::Corner layout_anchor_corner, Qt::Corner widget_anchor_corner);
        void AddCornerAnchor(QGraphicsLayoutItem *first, Qt::Corner first_anchor_corner, QGraphicsLayoutItem *second, Qt::Corner second_anchor_corner);
        void AddSideAnchor(QGraphicsLayoutItem *layout_item, Qt::Orientation orientation);
        void AnchorWidgetsHorizontally(QGraphicsLayoutItem *first_item, QGraphicsLayoutItem *second_item);
        void AnchorItemToLayout(QGraphicsLayoutItem *item, Qt::AnchorPoint item_point, Qt::AnchorPoint layout_point);
        void AddItemToScene(QGraphicsWidget *graphics_widget);
        void RemoveItemFromScene(QGraphicsWidget *graphics_widget);

    private slots:
        void InitLayout();
        void AdjustLayoutContainer(const QRectF &new_rect);
        void CheckVisibility(QGraphicsLayoutItem *layout_item);

    private:
        QGraphicsScene *scene_;
        QGraphicsWidget *layout_container_;
        QList<QGraphicsWidget *> full_screen_widgets_;

    };
}

#endif
