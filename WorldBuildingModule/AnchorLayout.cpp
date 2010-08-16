// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AnchorLayout.h"

#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QGraphicsLayoutItem>
#include <QGraphicsWidget>

#include <QDebug>

namespace WorldBuilding
{
    AnchorLayout::AnchorLayout(QObject *parent, QGraphicsScene *scene) :
        QObject(parent),
        QGraphicsAnchorLayout(0),
        scene_(scene)
    {
        InitLayout();
        AdjustLayoutContainer(scene_->sceneRect());
        connect(scene_, SIGNAL( sceneRectChanged(const QRectF &) ), SLOT( AdjustLayoutContainer(const QRectF &) ));
    }

    AnchorLayout::~AnchorLayout()
    {
    }

    // Private

    void AnchorLayout::InitLayout()
    {
        setSpacing(0);
        setContentsMargins(0,0,0,0);

        layout_container_ = new QGraphicsWidget();
        layout_container_->setLayout(this);

        scene_->addItem(layout_container_);
    }

    void AnchorLayout::AdjustLayoutContainer(const QRectF &new_rect)
    {
        layout_container_->setGeometry(new_rect);
    }

    void AnchorLayout::CheckVisibility(QGraphicsLayoutItem *layout_item)
    {
        QGraphicsWidget *widget = dynamic_cast<QGraphicsWidget *>(layout_item);
        if (!widget)
            return;
        if (!widget->isVisible())
            widget->show();
    }

    // Public

    QGraphicsScene *AnchorLayout::GetScene() const
    {
        return scene_;
    }

    void AnchorLayout::AddCornerAnchor(QGraphicsLayoutItem *layout_item, Qt::Corner layout_anchor_corner, Qt::Corner widget_anchor_corner)
    {
        addCornerAnchors(this, layout_anchor_corner, layout_item, widget_anchor_corner);
        CheckVisibility(layout_item);
    }

    void AnchorLayout::AddSideAnchor(QGraphicsLayoutItem *layout_item, Qt::Orientation orientation)
    {
        addAnchors(this, layout_item, orientation);
        CheckVisibility(layout_item);
    }

    void AnchorLayout::AnchorWidgetsHorizontally(QGraphicsLayoutItem *first_item, QGraphicsLayoutItem *second_item)
    {
        addAnchor(first_item, Qt::AnchorLeft, second_item, Qt::AnchorRight);
    }

    void AnchorLayout::AddItemToScene(QGraphicsWidget *graphics_widget)
    {
        scene_->addItem(graphics_widget);
    }

    void AnchorLayout::RemoveItemFromScene(QGraphicsWidget *graphics_widget)
    {
        scene_->removeItem(graphics_widget);
    }
}
